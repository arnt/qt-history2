/****************************************************************************
**
** Implementation of OLE drag and drop for Qt.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP

#include "qapplication_p.h"
#include "qevent.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qdragobject.h"
#include "qimage.h"
#include "qbuffer.h"
#include "qdatastream.h"
#include "qbitmap.h"
#include "qt_windows.h"
#include <shlobj.h>
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

static HCURSOR *cursor = 0;
static QDragObject *global_src = 0;
static bool acceptact = false;

/* OleStdGetDropEffect
** -------------------
**
** Convert a keyboard state into a DROPEFFECT.
**
** returns the DROPEFFECT value derived from the key state.
**    the following is the standard interpretation:
**          no modifier -- Default Drop     (0 is returned)
**          CTRL        -- DROPEFFECT_COPY
**          SHIFT       -- DROPEFFECT_MOVE
**          CTRL-SHIFT  -- DROPEFFECT_LINK
**
**    Default Drop: this depends on the type of the target application.
**    this is re-interpretable by each target application. a typical
**    interpretation is if the drag is local to the same document
**    (which is source of the drag) then a MOVE operation is
**    performed. if the drag is not local, then a COPY operation is
**    performed.
*/
#define OleStdGetDropEffect(grfKeyState)    \
    ((grfKeyState & MK_CONTROL) ?          \
        ((grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY) :  \
        ((grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0))

// Returns a LPFORMATETC enumerating all CF's that ms can be produced.
static
LPFORMATETC allFormats(int& n)
{
    n = 0;
    QWindowsMime* wm;
    QList<QWindowsMime*> mimes = QWindowsMime::all();
    for (int pos=0; pos<mimes.size(); ++pos) {
        wm = mimes[pos];
        n += wm->countCf();
    }

    LPFORMATETC fmtetc = new FORMATETC[n];

    int i = 0;
    for (int pos=0; pos<mimes.size(); ++pos) {
        wm = mimes[pos];
        int t = wm->countCf();
        for (int j=0; j<t; j++) {
            fmtetc[i].cfFormat = wm->cf(j);
            fmtetc[i].dwAspect = DVASPECT_CONTENT;
            fmtetc[i].tymed = TYMED_HGLOBAL;
            fmtetc[i].ptd = NULL;
            fmtetc[i].lindex = -1;
            i++;
        }
    }

    Q_ASSERT(n==i);
    return fmtetc;
}

// Returns a LPFORMATETC enumerating the CF's that ms can be produced
// from type \a mime.
static
LPFORMATETC someFormats(const char* mime, int& n)
{
    n = 0;
    QWindowsMime* wm;
    QList<QWindowsMime*> mimes = QWindowsMime::all();
    for (int pos=0; pos<mimes.size(); ++pos) {
        wm = mimes[pos];
        if (wm->cfFor(mime)) n += wm->countCf();
    }

    LPFORMATETC fmtetc = new FORMATETC[n];

    int i = 0;
    for (int pos=0; pos<mimes.size(); ++pos) {
        wm = mimes[pos];
        if (wm->cfFor(mime)) {
            int t = wm->countCf();
            for (int j=0; j<t; j++) {
                fmtetc[i].cfFormat = wm->cf(j);
                fmtetc[i].dwAspect = DVASPECT_CONTENT;
                fmtetc[i].tymed = TYMED_HGLOBAL;
                fmtetc[i].ptd = NULL;
                fmtetc[i].lindex = -1;
                i++;
            }
        }
    }

    Q_ASSERT(n==i);
    return fmtetc;
}


// Returns a LPFORMATETC enumerating the CF's that ms can produce
// (after being converted).
static
LPFORMATETC someFormats(const QMimeSource* ms, int& n)
{
    n = 0;
    QWindowsMime* wm;
    QList<QWindowsMime*> mimes = QWindowsMime::all();
    for (int pos=0; pos<mimes.size(); ++pos) {
        wm = mimes[pos];
        n += wm->countCf();
    }

    LPFORMATETC fmtetc = new FORMATETC[n]; // Bigger than needed

    int i = 0;
    for (int pos=0; pos<mimes.size(); ++pos) {
        wm = mimes[pos];
        int t = wm->countCf();
        for (int j=0; j<t; j++) {
            if (ms->provides(wm->mimeFor(wm->cf(j)))) {
                fmtetc[i].cfFormat = wm->cf(j);
                fmtetc[i].dwAspect = DVASPECT_CONTENT;
                fmtetc[i].tymed = TYMED_HGLOBAL;
                fmtetc[i].ptd = NULL;
                fmtetc[i].lindex = -1;
                i++;
            }
        }
    }
    n = i;

    return fmtetc;
}



class QOleDropSource : public IDropSource
{
    QWidget* src;
public:
    QOleDropSource(QWidget* w) :
        src(w)
    {
        m_refs = 1;
    }

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    /* IDropSource methods */
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);

private:
    ULONG m_refs;
};


class QOleDataObject : public IDataObject
{
public:
    QOleDataObject(QDragObject*);

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    /* IDataObject methods */
    STDMETHOD(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium);
    STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
    STDMETHOD(QueryGetData)(LPFORMATETC pformatetc);
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
    STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                       BOOL fRelease);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf,
                      LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);

private:
    ULONG m_refs;
    QDragObject* object;
};

class QOleDropTarget : public IDropTarget
{
    QWidget* widget;

public:
    QOleDropTarget(QWidget* w);

    void releaseQt()
    {
        widget = 0;
    }

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IDropTarget methods */
    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

    /* Utility function to read type of drag from key state*/
    STDMETHOD_(BOOL, QueryDrop)(DWORD grfKeyState, LPDWORD pdwEffect);

private:
    ULONG m_refs;
    BOOL acceptfmt;

    POINTL last_pt;
    DWORD last_effect;
    DWORD last_keystate;
};

static
LPDATAOBJECT current_dropobj = 0;

bool QDragManager::eventFilter(QObject *, QEvent *)
{
    return false;
}

void QDragManager::timerEvent(QTimerEvent*)
{
}

void QDragManager::cancel(bool /* deleteSource */)
{
    if (object) {
        beingCancelled = true;
        object = 0;
    }

#ifndef QT_NO_CURSOR
    // insert cancel code here

    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropEnd);
#endif
}


void QDragManager::move(const QPoint &)
{
    // not used in windows implementation
}


void QDragManager::drop()
{
    // not used in windows implementation
}


bool QDropEvent::provides(const char* mimeType) const
{
    if (!current_dropobj) // Sanity
        return false;

    int n;
    FORMATETC *fmtetc = someFormats(mimeType,n);
    bool does = false;
    for (int i=0; i<n && !does; i++) {
        int cf = fmtetc[i].cfFormat;
        QWindowsMime* wm = QWindowsMime::convertor(mimeType,cf);
        if (wm && NOERROR == current_dropobj->QueryGetData(fmtetc+i))
            does = true;
    }
    delete [] fmtetc;
    return does;
}

static
const char* dnd_format(int fn)
{
    if (!current_dropobj) // Sanity
        return 0;

    static QByteArray fmt("");

    int n;
    LPFORMATETC fmtetc = allFormats(n);
    int i;
    for (i=0; i<n && fn >= 0; i++) {
        // Does the drag source provide this format that we accept?
        if (NOERROR == current_dropobj->QueryGetData(fmtetc+i))
            fn--;
    }
    if (fn==-1)
        fmt = QWindowsMime::cfToMime(fmtetc[i-1].cfFormat);
    delete [] fmtetc;

    return fmt.isEmpty() ? 0 : (const char*)fmt;
}

const char* QDropEvent::format(int fn) const
{
    return dnd_format(fn);
}

QByteArray qt_olednd_obtain_data(const char *format)
{
    QByteArray result;

    if (!current_dropobj) // Sanity
        return result;

#ifdef USE_FORMATENUM // doesn't work yet
    LPENUMFORMATETC FAR fmtenum;
    HRESULT hr=current_dropobj->EnumFormatEtc(DATADIR_GET, &fmtenum);

    if (hr == NOERROR) {
        FORMATETC fmtetc;
        ULONG i=0;
        while (NOERROR==fmtenum->Next(i, &fmtetc, &i) && i) {
            int cf = fmtetc.cfFormat;
            QWindowsMime* wm = QWindowsMime::convertor(format, cf);
            STGMEDIUM medium;
            HGLOBAL hText;
            HRESULT hr;

            fmtetc.ptd = NULL;
            fmtetc.dwAspect = DVASPECT_CONTENT;
            fmtetc.lindex = -1;
            fmtetc.tymed = TYMED_HGLOBAL;

            hr = current_dropobj->GetData(&fmtetc, &medium);
            if (!FAILED(hr)) {
                hText = medium.hGlobal;
                char* d = (char*)GlobalLock(hText);
                int len = GlobalSize(medium.hGlobal);
                QByteArray r;
                r.setRawData(d,len);
                QByteArray tr = wm->convertToMime(r,format,cf);
                tr.detach();
                r.resetRawData(d,len);
                GlobalUnlock(hText);
                ReleaseStgMedium(&medium);
                return tr;
            }
        }
    }
#else

    int n;
    FORMATETC *fmtetc = someFormats(format,n);
    for (int i=0; i<n && result.isNull(); i++) {
        int cf = fmtetc[i].cfFormat;
        QWindowsMime* wm = QWindowsMime::convertor(format,cf);
        if (wm) {
            STGMEDIUM medium;
            HRESULT hr = current_dropobj->GetData(fmtetc+i, &medium);
            if (!FAILED(hr)) {
                HGLOBAL hText = medium.hGlobal;
                char* d = (char*)GlobalLock(hText);
                int len = GlobalSize(hText);
                QConstByteArray r(d, len);
                result = wm->convertToMime(r,format,cf);
                GlobalUnlock(hText);
                ReleaseStgMedium(&medium);
            }
        }
    }
    delete [] fmtetc;
#endif
    return result;
}

QByteArray QDropEvent::encodedData(const char* format) const
{
    return qt_olednd_obtain_data(format);
}


static QDragObject::DragMode current_mode = QDragObject::DragDefault;

bool QDragManager::drag(QDragObject * o, QDragObject::DragMode mode)
{
    if (object == o) {
        return false;
    }

    if (object) {
        cancel();
        if (dragSource)
            dragSource->removeEventFilter(this);
        beingCancelled = false;
    }

    object = o;
    dragSource = (QWidget *)(object->parent());
    global_src = o;
    global_src->setTarget(0);

#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropStart);
#endif

    const char* fmt;
    for (int i=0; (fmt=object->format(i)); i++)
        QWindowsMime::registerMimeType(fmt);

    DWORD result_effect;
    QOleDropSource *src = new QOleDropSource(dragSource);
    QOleDataObject *obj = new QOleDataObject(o);
    DWORD allowed_effects = 0;
    current_mode = mode;
    switch (mode) {
      case QDragObject::DragDefault:
        allowed_effects = DROPEFFECT_MOVE|DROPEFFECT_COPY;
        break;
      case QDragObject::DragMove:
        allowed_effects = DROPEFFECT_MOVE;
        break;
      case QDragObject::DragCopy:
        allowed_effects = DROPEFFECT_COPY;
        break;
      case QDragObject::DragCopyOrMove:
        allowed_effects = DROPEFFECT_MOVE|DROPEFFECT_COPY;
        break;
      case QDragObject::DragLink:
        allowed_effects = 0;
        break;
    }
    acceptact = false;
    allowed_effects |= DROPEFFECT_LINK;
    updatePixmap();
#ifdef Q_OS_TEMP
    HRESULT r = 0;
    result_effect = 0;
#else
    HRESULT r = DoDragDrop(obj, src, allowed_effects, &result_effect);
#endif
    if (dragSource) {
        QDragResponseEvent e(r == DRAGDROP_S_DROP);
        QApplication::sendEvent(dragSource, &e);
    }
    obj->Release();        // Will delete obj if refcount becomes 0
    src->Release();        // Will delete src if refcount becomes 0
    if (!global_src->target())
        acceptact=false;

    current_dropobj = 0;
    dragSource = 0;
    delete global_src;
    global_src = 0;
    object = 0;
    current_mode = QDragObject::DragDefault;

    updatePixmap();

    return r == DRAGDROP_S_DROP
        && (result_effect & DROPEFFECT_MOVE);
        //&& !acceptact;
}

void qt_olednd_unregister(QWidget* widget, QOleDropTarget *dst)
{
    dst->releaseQt();
#ifndef Q_OS_TEMP
    CoLockObjectExternal(dst, false, true);
    RevokeDragDrop(widget->winId());
#endif
    delete dst;
}

QOleDropTarget* qt_olednd_register(QWidget* widget)
{
    QOleDropTarget* dst = new QOleDropTarget(widget);
#ifndef Q_OS_TEMP
    RegisterDragDrop(widget->winId(), dst);
    CoLockObjectExternal(dst, true, true);
#endif
    return dst;
}


/***********************************************************
 * Standard implementation of IEnumFormatEtc. This code
 * is from \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
 ***********************************************************/
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize);
STDAPI_(void) OleStdFree(LPVOID pmem);
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);
STDAPI_(DVTARGETDEVICE FAR*) OleStdCopyTargetDevice(DVTARGETDEVICE FAR* ptdSrc);
STDAPI_(LPENUMFORMATETC)
  OleStdEnumFmtEtc_Create(ULONG nCount, LPFORMATETC lpEtc);


//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDropSource::QueryInterface(REFIID iid, void FAR* FAR* ppv)
{
    if(iid == IID_IUnknown || iid == IID_IDropSource)
    {
      *ppv = this;
      ++m_refs;
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
QOleDropSource::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
QOleDropSource::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}

//---------------------------------------------------------------------
//                    IDropSource Methods
//---------------------------------------------------------------------

STDMETHODIMP
QOleDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if (fEscapePressed) {
        return ResultFromScode(DRAGDROP_S_CANCEL);
    } else if (!(grfKeyState & (MK_LBUTTON|MK_MBUTTON|MK_RBUTTON))) {
        return ResultFromScode(DRAGDROP_S_DROP);
    } else {
        qApp->sendPostedEvents();
        return NOERROR;
    }
}

STDMETHODIMP
QOleDropSource::GiveFeedback(DWORD dwEffect)
{
    if (cursor) {
        int c = -1;
        switch (dwEffect) {
          case DROPEFFECT_MOVE:
            c = 0;
            break;
          case DROPEFFECT_COPY:
            c = 1;
            break;
          case DROPEFFECT_LINK:
            c = 2;
            break;
        }
        if (c >= 0) {
            SetCursor(cursor[c]);
            return ResultFromScode(S_OK);
        }
    }
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}


//---------------------------------------------------------------------
//                    QOleDataObject Constructor
//---------------------------------------------------------------------

QOleDataObject::QOleDataObject(QDragObject* o) :
    object(o)
{
    m_refs = 1;
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDataObject::QueryInterface(REFIID iid, void FAR* FAR* ppv)
{
    if(iid == IID_IUnknown || iid == IID_IDataObject)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
QOleDataObject::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
QOleDataObject::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}

//---------------------------------------------------------------------
//                    IDataObject Methods
//
// The following methods are NOT supported for data transfer using the
// clipboard or drag-drop:
//
//      IDataObject::SetData    -- return E_NOTIMPL
//      IDataObject::DAdvise    -- return OLE_E_ADVISENOTSUPPORTED
//                 ::DUnadvise
//                 ::EnumDAdvise
//      IDataObject::GetCanonicalFormatEtc -- return E_NOTIMPL
//                     (NOTE: must set pformatetcOut->ptd = NULL)
//---------------------------------------------------------------------

extern bool qt_CF_HDROP_valid(const char *mime, int cf, QMimeSource * src);

STDMETHODIMP
QOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
{
    // This method is called by the drag-drop target to obtain the data
    // that is being dragged.

    pmedium->tymed = 0;
    pmedium->pUnkForRelease = NULL;
    pmedium->hGlobal = NULL;

    QWindowsMime *wm;

    const char* fmt;
    for (int i=0; (fmt=object->format(i)); i++) {
        if ((wm=QWindowsMime::convertor(fmt,pformatetc->cfFormat))
            && (pformatetc->dwAspect & DVASPECT_CONTENT) &&
               (pformatetc->tymed & TYMED_HGLOBAL) &&
               qt_CF_HDROP_valid(fmt, pformatetc->cfFormat, object))
        {
            QByteArray data =
                wm->convertFromMime(object->encodedData(fmt),
                            fmt, pformatetc->cfFormat);
            if (data.size()) {
                HGLOBAL hData = GlobalAlloc(0, data.size());
                if (!hData) {
                    return ResultFromScode(E_OUTOFMEMORY);
                }
                void* out = GlobalLock(hData);
                memcpy(out,data.data(),data.size());
                GlobalUnlock(hData);
                pmedium->tymed = TYMED_HGLOBAL;
                pmedium->hGlobal = hData;
                return ResultFromScode(S_OK);
            } else {
            }
        }
    }
    return ResultFromScode(DATA_E_FORMATETC);
}

STDMETHODIMP
QOleDataObject::GetDataHere(LPFORMATETC, LPSTGMEDIUM)
{
    return ResultFromScode(DATA_E_FORMATETC);
}

STDMETHODIMP
QOleDataObject::QueryGetData(LPFORMATETC pformatetc)
{
    // This method is called by the drop target to check whether the source
    // provides data in a format that the target accepts.

    const char* fmt;
    for (int i=0; (fmt=object->format(i)); i++) {
        if (QWindowsMime::convertor(fmt,pformatetc->cfFormat) &&
           (pformatetc->dwAspect & DVASPECT_CONTENT) &&
           (pformatetc->tymed & TYMED_HGLOBAL) &&
           qt_CF_HDROP_valid(fmt, pformatetc->cfFormat, object))
        {
            return ResultFromScode(S_OK);
        }
    }
    return ResultFromScode(S_FALSE);
}

STDMETHODIMP
QOleDataObject::GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC pformatetcOut)
{
    pformatetcOut->ptd = NULL;
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP
QOleDataObject::SetData(LPFORMATETC, STGMEDIUM *, BOOL)
{
    // A data transfer object that is used to transfer data
    //    (either via the clipboard or drag/drop does NOT
    //    accept SetData on ANY format.
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP
QOleDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
{
    // A standard implementation is provided by OleStdEnumFmtEtc_Create
    // which can be found in \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
    // This code from ole2ui is copied to the enumfetc.c file in this sample.

    SCODE sc = S_OK;
    int n;
    LPFORMATETC fmtetc = someFormats(object,n);

    if (dwDirection == DATADIR_GET){
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(n, fmtetc);
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;

    } else if (dwDirection == DATADIR_SET){
        // A data transfer object that is used to transfer data
        //    (either via the clipboard or drag/drop does NOT
        //    accept SetData on ANY format.
        sc = E_NOTIMPL;
        goto error;
    } else {
        sc = E_INVALIDARG;
        goto error;
    }

error:

    delete [] fmtetc;
    return ResultFromScode(sc);
}

STDMETHODIMP
QOleDataObject::DAdvise(FORMATETC FAR*, DWORD,
                       LPADVISESINK, DWORD FAR*)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


STDMETHODIMP
QOleDataObject::DUnadvise(DWORD)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP
QOleDataObject::EnumDAdvise(LPENUMSTATDATA FAR*)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}





QOleDropTarget::QOleDropTarget(QWidget* w) :
    widget(w)
{
   m_refs = 1;
   acceptfmt = false;
   last_pt.x = last_pt.y = -100000;
   last_effect = DROPEFFECT_NONE;
   last_keystate = (DWORD)-1;
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDropTarget::QueryInterface(REFIID iid, void FAR* FAR* ppv)
{
    if(iid == IID_IUnknown || iid == IID_IDropTarget)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
QOleDropTarget::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
QOleDropTarget::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}

//---------------------------------------------------------------------
//                    IDropTarget Methods
//---------------------------------------------------------------------

STDMETHODIMP
QOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    current_dropobj = pDataObj;
    QDragEnterEvent de(widget->mapFromGlobal(QPoint(pt.x,pt.y)));

    acceptfmt = true; // set this true to get the correct initial action
    QueryDrop(grfKeyState, pdwEffect);

    if (*pdwEffect & DROPEFFECT_MOVE)
        de.setAction(QDropEvent::Move);
    else if (*pdwEffect & DROPEFFECT_LINK)
        de.setAction(QDropEvent::Link);

    de.acceptAction(false);
    de.accept(false);

    QApplication::sendEvent(widget, &de);
    acceptfmt = de.isAccepted();
    acceptact = de.isActionAccepted();

    if (!acceptfmt)
        *pdwEffect = DROPEFFECT_NONE;
    else if (de.action() == QDropEvent::Move)
        *pdwEffect = DROPEFFECT_MOVE;
    else if (de.action() == QDropEvent::Copy)
        *pdwEffect = DROPEFFECT_COPY;
    else if (de.action() == QDropEvent::Link)
        *pdwEffect = DROPEFFECT_LINK;


    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    bool old_acceptfmt = acceptfmt; // set this to get the correct action.
    acceptfmt = true;
    QueryDrop(grfKeyState, pdwEffect);
    acceptfmt = old_acceptfmt;

    if (pt.x == last_pt.x && pt.y == last_pt.y &&
        *pdwEffect == last_effect && grfKeyState == last_keystate) {
        if (!acceptact&&!acceptfmt)
            *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    last_pt = pt;
    last_effect = *pdwEffect;
    last_keystate = grfKeyState;

    QDragMoveEvent de(widget->mapFromGlobal(QPoint(pt.x,pt.y)));
    if (*pdwEffect & DROPEFFECT_MOVE)
        de.setAction(QDropEvent::Move);
    else if (*pdwEffect & DROPEFFECT_LINK)
        de.setAction(QDropEvent::Link);
    de.acceptAction(acceptact);
    de.accept(acceptfmt);

    QApplication::sendEvent(widget, &de);
    acceptfmt = de.isAccepted();
    acceptact = de.isActionAccepted();

    if (!acceptfmt)
        *pdwEffect = DROPEFFECT_NONE;
    else if (de.action() == QDropEvent::Move)
        *pdwEffect = DROPEFFECT_MOVE;
    else if (de.action() == QDropEvent::Copy)
        *pdwEffect = DROPEFFECT_COPY;
    else if (de.action() == QDropEvent::Link)
        *pdwEffect = DROPEFFECT_LINK;

    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragLeave()
{
    acceptfmt = false;
    current_dropobj = 0;
    QDragLeaveEvent de;
    QApplication::sendEvent(widget, &de);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    if (QueryDrop(grfKeyState, pdwEffect))
    {
        current_dropobj = pDataObj;

        if (global_src)
            global_src->setTarget(widget);
        QDropEvent de(widget->mapFromGlobal(QPoint(pt.x,pt.y)));
        if (*pdwEffect & DROPEFFECT_MOVE)
            de.setAction(QDropEvent::Move);
        else if (*pdwEffect & DROPEFFECT_LINK)
            de.setAction(QDropEvent::Link);

        de.acceptAction(acceptact);
        de.accept(acceptfmt);

        QApplication::sendEvent(widget, &de);

        acceptfmt = de.isAccepted();
        acceptact = de.isActionAccepted();

        if (!acceptfmt)
            *pdwEffect = DROPEFFECT_NONE;
        else if (acceptact)
            *pdwEffect = DROPEFFECT_NONE; /* NONE because the source should do nothing ..
                                          the tagret is responsible to perform the correct action*/
        else if (de.action() == QDropEvent::Move)
            *pdwEffect = DROPEFFECT_MOVE;
        else if (de.action() == QDropEvent::Copy)
            *pdwEffect = DROPEFFECT_COPY;
        else if (de.action() == QDropEvent::Link)
            *pdwEffect = DROPEFFECT_LINK;


        // We won't get any mouserelease-event, so manually adjust qApp state:
        QApplication::winMouseButtonUp();

        acceptfmt = false;
        current_dropobj = 0;
        return NOERROR;
    }

    *pdwEffect = DROPEFFECT_NONE;
    return ResultFromScode(DATA_E_FORMATETC);
}

//---------------------------------------------------------------------
// QOleDropTarget::QueryDrop: Given key state, determines the type of
// acceptable drag and returns the a dwEffect.
//---------------------------------------------------------------------
STDMETHODIMP_(BOOL)
QOleDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
{
    DWORD dwOKEffects = *pdwEffect;

    if (!acceptfmt)
        goto dropeffect_none;

    *pdwEffect = OleStdGetDropEffect(grfKeyState);
    if (*pdwEffect == 0) {
        // No modifier keys used by user while dragging. Try in order: MOVE, COPY, LINK.
        if (DROPEFFECT_MOVE & dwOKEffects) {
            *pdwEffect = DROPEFFECT_MOVE;
        } else if (DROPEFFECT_COPY & dwOKEffects) {
            *pdwEffect = DROPEFFECT_COPY;
        } else if (DROPEFFECT_LINK & dwOKEffects) {
            *pdwEffect = DROPEFFECT_LINK;
        }
        else goto dropeffect_none;
    } else {
        // Check if the drag source application allows the drop effect desired by user.
        // The drag source specifies this in DoDragDrop
        if (!(*pdwEffect & dwOKEffects))
            goto dropeffect_none;
    }
    return true;

dropeffect_none:
    *pdwEffect = DROPEFFECT_NONE;
    return false;
}

extern HBITMAP qt_createIconMask(const QBitmap &bitmap);

void QDragManager::updatePixmap()
{
    if (object) {
        if (cursor) {
#ifndef Q_OS_TEMP
            for (int i=0; i<n_cursor; i++) {
                DestroyCursor(cursor[i]);
            }
#endif
            delete [] cursor;
            cursor = 0;
        }

        QPixmap pm = object->pixmap();
        if (pm.isNull()) {
            // None.
        } else {
            cursor = new HCURSOR[n_cursor];
            QPoint pm_hot = object->pixmapHotSpot();
            for (int cnum=0; cnum<n_cursor; cnum++) {
                QPixmap cpm = pm_cursor[cnum];

                int x1 = qMin(-pm_hot.x(),0);
                int x2 = qMax(pm.width()-pm_hot.x(),cpm.width());
                int y1 = qMin(-pm_hot.y(),0);
                int y2 = qMax(pm.height()-pm_hot.y(),cpm.height());

                int w = x2-x1+1;
                int h = y2-y1+1;

#ifndef Q_OS_TEMP
                if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
                    // Limited cursor size
                    int reqw = GetSystemMetrics(SM_CXCURSOR);
                    int reqh = GetSystemMetrics(SM_CYCURSOR);
                    if (reqw < w) {
                        // Not wide enough - move objectpm right
                        pm_hot.setX(pm_hot.x()-w+reqw);
                    }
                    if (reqh < h) {
                        // Not tall enough - move objectpm down
                        pm_hot.setY(pm_hot.y()-h+reqh);
                    }
                    // Always use system cursor size
                    w = reqw;
                    h = reqh;
                }
#endif

                QPixmap colorbits(w,h,-1,QPixmap::NormalOptim);
                {
                    QPainter p(&colorbits);
                    p.fillRect(0,0,w,h,color1);
                    p.drawPixmap(qMax(0,-pm_hot.x()),qMax(0,-pm_hot.y()),pm);
                    p.drawPixmap(qMax(0,pm_hot.x()),qMax(0,pm_hot.y()),cpm);
                }

                QBitmap maskbits(w,h,true,QPixmap::NormalOptim);
                {
                    QPainter p(&maskbits);
                    if (pm.mask()) {
                        QBitmap m(*pm.mask());
                        m.setMask(m);
                        p.drawPixmap(qMax(0,-pm_hot.x()),qMax(0,-pm_hot.y()),m);
                    } else {
                        p.fillRect(qMax(0,-pm_hot.x()),qMax(0,-pm_hot.y()),
                            pm.width(),pm.height(),color1);
                    }
                    if (cpm.mask()) {
                        QBitmap m(*cpm.mask());
                        m.setMask(m);
                        p.drawPixmap(qMax(0,pm_hot.x()),qMax(0,pm_hot.y()),m);
                    } else {
                        p.fillRect(qMax(0,pm_hot.x()),qMax(0,pm_hot.y()),
                            cpm.width(),cpm.height(),
                            color1);
                    }
                }

                HBITMAP im = qt_createIconMask(maskbits);
                ICONINFO ii;
                ii.fIcon     = false;
                ii.xHotspot  = qMax(0,pm_hot.x());
                ii.yHotspot  = qMax(0,pm_hot.y());
                ii.hbmMask   = im;
                ii.hbmColor  = colorbits.hbm();
                cursor[cnum] = CreateIconIndirect(&ii);
                DeleteObject(im);
            }
        }
    }
}

#endif // QT_NO_DRAGANDDROP
