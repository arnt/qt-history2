/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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
#include "qimage.h"
#include "qbuffer.h"
#include "qdatastream.h"
#include "qbitmap.h"
#include "qt_windows.h"
#include <shlobj.h>
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include "qdnd_p.h"

//#define QDND_DEBUG

#ifdef QDND_DEBUG
extern QString dragActionsToString(QDrag::DropActions actions);
#endif

static HCURSOR *cursor = 0;
static QDragPrivate *global_src = 0;
static bool acceptact = false;


class QWinMimeData : public QMimeData
{
};

/*
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
LPFORMATETC someFormats(const QString &mime, int& n)
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
LPFORMATETC someFormats(const QMimeData* ms, int& n)
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
            if (ms->hasFormat(wm->mimeFor(wm->cf(j)).latin1())) {
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

*/

class QOleDropSource : public IDropSource
{
public:
    QOleDropSource()
    {
        m_refs = 1;
        currentAction = QDrag::IgnoreAction;
    }

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // IDropSource methods
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);

private:
    ULONG m_refs;
    QDrag::DropAction currentAction;
};


class QOleDataObject : public IDataObject
{
public:
    QOleDataObject(QMimeData *mimeData);

    // IUnknown methods 
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // IDataObject methods 
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
    QMimeData *data;
};

class QOleDropTarget : public IDropTarget
{
public:
    QOleDropTarget(QWidget* w);

    void releaseQt()
    {
        widget = 0;
    }

    // IUnknown methods 
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // IDropTarget methods 
    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

private:
    ULONG m_refs;
    QWidget* widget;
    QRect acceptRect;
    DWORD choosenEffect;

        //////////////
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
#ifndef QT_NO_ACCESSIBILITY
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
/*
static
QString dnd_format(int fn)
{
    if (!current_dropobj) // Sanity
        return 0;

    QString fmt;

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

    return fmt;
}
*/
bool QDropData::hasFormat(const QString &mimeType) const
{
  /*  if (!current_dropobj) // Sanity
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
    */
    return false;
}


QStringList QDropData::formats() const
{
    /*
    QStringList fmts;
    if (!current_dropobj) // Sanity
        return fmts;

    int n;
    LPFORMATETC fmtetc = allFormats(n);
    for (int i=0; i<n; i++) {
        // Does the drag source provide this format that we accept?
        if (NOERROR == current_dropobj->QueryGetData(fmtetc+i))
            fmts.append(QWindowsMime::cfToMime(fmtetc[i].cfFormat));
    }
    delete [] fmtetc;

    return fmts;
    */
    return QStringList();
}
/*
QByteArray qt_olednd_obtain_data(const QString &format)
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
                const QByteArray r = QByteArray::fromRawData((char*)GlobalLock(hText), GlobalSize(hText));
                result = wm->convertToMime(r,format,cf);
		result.detach(); // insure that we no longer reference the global data.
                GlobalUnlock(hText);
                ReleaseStgMedium(&medium);
            }
        }
    }
    delete [] fmtetc;
#endif
    return result;
}
*/

QVariant QDropData::retrieveData(const QString &format, QVariant::Type) const
{
    //return qt_olednd_obtain_data(format);
    return QVariant();
}

QDrag::DropAction QDragManager::drag(QDrag *drag)
{
#ifdef QDND_DEBUG
    qDebug("QDragManager::drag(QDrag *drag)");
    qDebug("actions = %s", dragActionsToString(dragPrivate(drag)->request_action).latin1());
#endif
  /*  if (object == o || !o->source)
        return QDrag::NoAction;

    if (object) {
        o->source->removeEventFilter(this);
        cancel();
        beingCancelled = false;
    }

    object = o;
    global_src = o;
    o->target = 0;
*/
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropStart);
#endif

  /*  QStringList fmts = o->data->formats();
    for(int i = 0; i < fmts.size(); ++i)
        QWindowsMime::registerMimeType(fmts.at(i).latin1());
*/
    DWORD result_effect;
    QOleDropSource *src = new QOleDropSource();
    QOleDataObject *obj = new QOleDataObject(drag->mimeData());
    DWORD allowed_effects = 0;
    if (dragPrivate(drag)->request_action & QDrag::LinkAction)
        allowed_effects = DROPEFFECT_LINK;
    if (dragPrivate(drag)->request_action & QDrag::CopyAction)
        allowed_effects = DROPEFFECT_COPY;
    if (dragPrivate(drag)->request_action & QDrag::MoveAction)
        allowed_effects = DROPEFFECT_MOVE;
    
    updatePixmap();

#ifdef Q_OS_TEMP
    HRESULT r = 0;
    result_effect = 0;
#else
    HRESULT r = DoDragDrop(obj, src, allowed_effects, &result_effect);
#endif

    obj->Release();        // Will delete obj if refcount becomes 0
    src->Release();        // Will delete src if refcount becomes 0
    
    if (r == DRAGDROP_S_DROP) {
        if (result_effect & DROPEFFECT_LINK)
            return QDrag::LinkAction;
        if (result_effect & DROPEFFECT_COPY)
            return QDrag::CopyAction;
        if (result_effect & DROPEFFECT_MOVE)
            return QDrag::MoveAction;
    }
    return QDrag::IgnoreAction;
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
        return NOERROR;
    }
}

STDMETHODIMP
QOleDropSource::GiveFeedback(DWORD dwEffect)
{
    QDrag::DropAction action = QDrag::IgnoreAction;

    if (dwEffect & DROPEFFECT_MOVE)
        action = QDrag::MoveAction;
    else if (dwEffect & DROPEFFECT_COPY)
        action = QDrag::CopyAction;
    else if (dwEffect & DROPEFFECT_LINK)
        action = QDrag::LinkAction;
    
    if (action != currentAction) {
        currentAction = action;
       //### emit action changed signal 
    }
    //###set the correct cursor
    //HCURSOR c = dragManager->hCursors[dragManager->cursorIndex(action)];
//	if (c) {
//	    SetCursor(c);
//	    return ResultFromScode(S_OK);
//	}
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}


//---------------------------------------------------------------------
//                    QOleDataObject Constructor
//---------------------------------------------------------------------

QOleDataObject::QOleDataObject(QMimeData *mimeData)
{
    m_refs = 1;
    data = mimeData;
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

extern bool qt_CF_HDROP_valid(const QString &mime, int cf, QMimeData *src);

STDMETHODIMP
QOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
{
#ifdef QDND_DEBUG
    qDebug("QOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)");
#endif
    pmedium->tymed = 0;
    pmedium->pUnkForRelease = NULL;
    pmedium->hGlobal = NULL;
    
    if (pformatetc->dwAspect & DVASPECT_CONTENT && pformatetc->tymed & TYMED_HGLOBAL) {
        if (pformatetc->cfFormat == CF_TEXT) {
            HGLOBAL hData = GlobalAlloc(0, data->text().length() + 1);
            if (!hData) {
                return ResultFromScode(E_OUTOFMEMORY);
            }
            void* out = GlobalLock(hData);
            memcpy(out, data->text().latin1(), data->text().length() + 1);
            GlobalUnlock(hData);
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = hData;
            return ResultFromScode(S_OK);
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
#ifdef QDND_DEBUG
    qDebug("QOleDataObject::QueryGetData(LPFORMATETC pformatetc)");
#endif
    if (pformatetc->dwAspect & DVASPECT_CONTENT &&
        pformatetc->tymed & TYMED_HGLOBAL &&
        pformatetc->cfFormat == CF_TEXT) {
        return ResultFromScode(S_OK);
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
#ifdef QDND_DEBUG
    qDebug("QOleDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)");
#endif
    SCODE sc = S_OK;
    ///### just text for now
    FORMATETC fmtetc[1];
    if (dwDirection == DATADIR_GET) {
        fmtetc[0].cfFormat = CF_TEXT;
        fmtetc[0].dwAspect = DVASPECT_CONTENT;
        fmtetc[0].lindex = -1;
        fmtetc[0].ptd =  NULL;
        fmtetc[0].tymed = TYMED_HGLOBAL;
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(1, fmtetc);
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;
    } else {
        sc = E_INVALIDARG;
    }
    
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

QDrag::DropActions translateToQDragDropActions(DWORD pdwEffects)
{
    QDrag::DropActions actions = QDrag::IgnoreAction;
    if (pdwEffects & DROPEFFECT_LINK)
        actions |= QDrag::LinkAction;
    if (pdwEffects & DROPEFFECT_COPY)
        actions |= QDrag::CopyAction;
    if (pdwEffects & DROPEFFECT_MOVE)
        actions |= QDrag::MoveAction;
    return actions;
}

DWORD translateToWinDragEffect(QDrag::DropAction action)
{
    if (action & QDrag::LinkAction)
        return DROPEFFECT_LINK;
    if (action & QDrag::CopyAction)
        return DROPEFFECT_COPY;
    if (action & QDrag::MoveAction)
        return DROPEFFECT_MOVE;
    return DROPEFFECT_NONE;
}

STDMETHODIMP
QOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)");
#endif

   /* //### ?????
       if (!qt_tryModalHelper(widget)) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }
    */

    QWinMimeData winMimeData;
    QDragEnterEvent dragEnterEvent(widget->mapFromGlobal(QPoint(pt.x,pt.y)), translateToQDragDropActions(*pdwEffect), &winMimeData);
    QApplication::sendEvent(widget, &dragEnterEvent);



   // acceptfmt = true; // set this true to get the correct initial action
   // QueryDrop(grfKeyState, pdwEffect);

    acceptRect = dragEnterEvent.answerRect();
    choosenEffect = translateToWinDragEffect(dragEnterEvent.dropAction());
    *pdwEffect = choosenEffect;

    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)");
#endif
    
     /* //### ?????
       if (!qt_tryModalHelper(widget)) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }
    */

    QWinMimeData winMimeData;
    QDragMoveEvent dragMoveEvent(widget->mapFromGlobal(QPoint(pt.x,pt.y)), translateToQDragDropActions(*pdwEffect), &winMimeData);
    QApplication::sendEvent(widget, &dragMoveEvent);

    acceptRect = dragMoveEvent.answerRect();
    choosenEffect = translateToWinDragEffect(dragMoveEvent.dropAction());
    *pdwEffect = choosenEffect;

    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragLeave()
{
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::DragLeave()");
#endif
 //   if (!qt_tryModalHelper(widget))
  //      return NOERROR;

    QDragLeaveEvent dragLeaveEvent;
    QApplication::sendEvent(widget, &dragLeaveEvent);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)");
#endif
   // if (!qt_tryModalHelper(widget)) {
   //     *pdwEffect = DROPEFFECT_NONE;
   //     return NOERROR;
   // }

    QWinMimeData winMimeData;
    QDragMoveEvent dropEvent(widget->mapFromGlobal(QPoint(pt.x,pt.y)), translateToQDragDropActions(*pdwEffect), &winMimeData);
    QApplication::sendEvent(widget, &dropEvent);

    choosenEffect = translateToWinDragEffect(dropEvent.dropAction());
    *pdwEffect = choosenEffect;

    return NOERROR;

    /*
  
    if (QueryDrop(grfKeyState, pdwEffect))
    {
        current_dropobj = pDataObj;

        if (global_src)
            global_src->target = widget;
        QDropEvent de(widget->mapFromGlobal(QPoint(pt.x,pt.y)), QDrag::CopyAction, QDragManager::self()->dropData);
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
            *pdwEffect = DROPEFFECT_NONE; // NONE because the source should do nothing ..
                                          //the tagret is responsible to perform the correct action
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
    */
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
            QPoint pm_hot = object->hotSpot();
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
                    p.fillRect(0,0,w,h,Qt::color1);
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
                            pm.width(),pm.height(),Qt::color1);
                    }
                    if (cpm.mask()) {
                        QBitmap m(*cpm.mask());
                        m.setMask(m);
                        p.drawPixmap(qMax(0,pm_hot.x()),qMax(0,pm_hot.y()),m);
                    } else {
                        p.fillRect(qMax(0,pm_hot.x()),qMax(0,pm_hot.y()),
                            cpm.width(),cpm.height(),
                            Qt::color1);
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
