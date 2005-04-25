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
#include "qbuffer.h"
#include "qdatastream.h"
#include "qcursor.h"
#include "qt_windows.h"
#include <shlobj.h>
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include "qdnd_p.h"

//#define QDND_DEBUG

#ifdef QDND_DEBUG
extern QString dragActionsToString(Qt::DropActions actions);
#endif

Qt::DropActions translateToQDragDropActions(DWORD pdwEffects)
{
    Qt::DropActions actions = Qt::IgnoreAction;
    if (pdwEffects & DROPEFFECT_LINK)
        actions |= Qt::LinkAction;
    if (pdwEffects & DROPEFFECT_COPY)
        actions |= Qt::CopyAction;
    if (pdwEffects & DROPEFFECT_MOVE)
        actions |= Qt::MoveAction;
    return actions;
}

Qt::DropAction translateToQDragDropAction(DWORD pdwEffect)
{
    if (pdwEffect & DROPEFFECT_LINK)
        return Qt::LinkAction;
    if (pdwEffect & DROPEFFECT_COPY)
        return Qt::CopyAction;
    if (pdwEffect & DROPEFFECT_MOVE)
        return Qt::MoveAction;
    return Qt::IgnoreAction;
}

DWORD translateToWinDragEffects(Qt::DropActions action)
{
    DWORD effect = DROPEFFECT_NONE;
    if (action & Qt::LinkAction)
        effect |= DROPEFFECT_LINK;
    if (action & Qt::CopyAction)
        effect |= DROPEFFECT_COPY;
    if (action & Qt::MoveAction)
        effect |= DROPEFFECT_MOVE;
    return effect;
}

Qt::KeyboardModifiers toQtKeyboardModifiers(DWORD keyState)
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    if (keyState & MK_SHIFT)
        modifiers |= Qt::ShiftModifier;
    if (keyState & MK_CONTROL)
        modifiers |= Qt::ControlModifier;
    if (keyState & MK_ALT)
        modifiers |= Qt::AltModifier;

    return modifiers;
}

Qt::MouseButtons toQtMouseButtons(DWORD keyState)
{
    Qt::MouseButtons buttons = Qt::NoButton;

    if (keyState & MK_LBUTTON)
        buttons |= Qt::LeftButton;
    if (keyState & MK_RBUTTON)
        buttons |= Qt::RightButton;
    if (keyState & MK_MBUTTON)
        buttons |= Qt::MidButton;

    return buttons;
}

class QOleDropSource : public IDropSource
{
public:
    QOleDropSource();
    virtual ~QOleDropSource();

    void createCursors();

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // IDropSource methods
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);

private:
    Qt::DropAction currentAction;
    QMap <Qt::DropAction, QCursor> cursors;

    ULONG m_refs;
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
    QRect answerRect;
    QPoint lastPoint;
    DWORD choosenEffect;
    DWORD lastKeyState;
};


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



QOleDropSource::QOleDropSource()
{
    m_refs = 1;
    currentAction = Qt::IgnoreAction;
}

QOleDropSource::~QOleDropSource()
{
}

void QOleDropSource::createCursors()
{
    QDragManager *manager = QDragManager::self();
    if (manager && manager->object
        && (!manager->object->pixmap().isNull()
        || manager->hasCustomDragCursors())) {
        QPixmap pm = manager->object->pixmap();
        QList<Qt::DropAction> actions;
        actions << Qt::MoveAction << Qt::CopyAction << Qt::LinkAction;
        QPoint hotSpot = manager->object->hotSpot();
        for (int cnum = 0; cnum < actions.size(); ++cnum) {
            QPixmap cpm = manager->dragCursor(actions.at(cnum));
            int w = cpm.width();
            int h = cpm.height();

            if (!pm.isNull()) {
                int x1 = qMin(-hotSpot.x(),0);
                int x2 = qMax(pm.width()-hotSpot.x(),cpm.width());
                int y1 = qMin(-hotSpot.y(),0);
                int y2 = qMax(pm.height()-hotSpot.y(),cpm.height());

                w = x2-x1+1;
                h = y2-y1+1;
            }

#ifndef Q_OS_TEMP
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
                // Limited cursor size
                int reqw = GetSystemMetrics(SM_CXCURSOR);
                int reqh = GetSystemMetrics(SM_CYCURSOR);
                if (reqw < w) {
                    // Not wide enough - move objectpm right
                    hotSpot.setX(hotSpot.x()-w+reqw);
                }
                if (reqh < h) {
                    // Not tall enough - move objectpm down
                    hotSpot.setY(hotSpot.y()-h+reqh);
                }
                // Always use system cursor size
                w = reqw;
                h = reqh;
            }
#endif

            QPixmap newCursor(w, h);
            if (!pm.isNull()) {
                newCursor.fill(QColor(0, 0, 0, 0));
                QPainter p(&newCursor);
                p.drawPixmap(qMax(0,-hotSpot.x()),qMax(0,-hotSpot.y()),pm);
                p.drawPixmap(qMax(0,hotSpot.x()),qMax(0,hotSpot.y()),cpm);
            } else {
                newCursor = cpm;
            }

            cursors[actions.at(cnum)] = QCursor(newCursor, pm.isNull() ? 0 : qMax(0,hotSpot.x()),
                                                pm.isNull() ? 0 : qMax(0,hotSpot.y()));
        }
    }
}



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
        qApp->processEvents();
        return NOERROR;
    }
}

STDMETHODIMP
QOleDropSource::GiveFeedback(DWORD dwEffect)
{
    Qt::DropAction action = translateToQDragDropAction(dwEffect);

#ifdef QDND_DEBUG
    qDebug("QOleDropSource::GiveFeedback(DWORD dwEffect)");
    qDebug("dwEffect = %s", dragActionsToString(action).toLatin1().data());
#endif

    if (currentAction != action) {
        currentAction = action;
        QDragManager::self()->emitActionChanged(currentAction);
    }

    if (cursors.contains(currentAction)) {
        SetCursor(cursors[currentAction].handle());
        return ResultFromScode(S_OK);
    }

    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}


//---------------------------------------------------------------------
//                    QOleDataObject Constructor
//---------------------------------------------------------------------

QOleDataObject::QOleDataObject(QMimeData *mimeData)
{
    m_refs = 1;
    data = mimeData;
    CF_PERFORMEDDROPEFFECT = RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
    performedEffect = DROPEFFECT_NONE;
}

void QOleDataObject::releaseQt()
{
    if (data) {
        delete data;
        data = 0;
    }
}

const QMimeData *QOleDataObject::mimeData() const
{
    return data;
}

DWORD QOleDataObject::reportedPerformedEffect() const
{
    return performedEffect;
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP
QOleDataObject::QueryInterface(REFIID iid, void FAR* FAR* ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDataObject) {
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
    if (--m_refs == 0) {
        releaseQt();
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

STDMETHODIMP
QOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
{
#ifdef QDND_DEBUG
    qDebug("QOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)");
    char buf[256] = {0};
    GetClipboardFormatNameA(pformatetc->cfFormat, buf, 255);
    qDebug("CF = %d : %s", pformatetc->cfFormat, buf);
#endif

    if (!data)
        return ResultFromScode(DATA_E_FORMATETC);

    QWindowsMime *converter = QWindowsMime::converterFromMime(*pformatetc, data);

    if (converter && converter->convertFromMime(*pformatetc, data, pmedium))
        return ResultFromScode(S_OK);
    else
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

    if (!data)
        return ResultFromScode(DATA_E_FORMATETC);

    if (QWindowsMime::converterFromMime(*pformatetc, data))
        return ResultFromScode(S_OK);
    return ResultFromScode(S_FALSE);
}

STDMETHODIMP
QOleDataObject::GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC pformatetcOut)
{
    pformatetcOut->ptd = NULL;
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP
QOleDataObject::SetData(LPFORMATETC pFormatetc, STGMEDIUM *pMedium, BOOL fRelease)
{
    if (pFormatetc->cfFormat == CF_PERFORMEDDROPEFFECT && pMedium->tymed == TYMED_HGLOBAL) {
        DWORD * val = (DWORD*)GlobalLock(pMedium->hGlobal);
        performedEffect = *val;
        GlobalUnlock(pMedium->hGlobal);
        if (fRelease)
            ReleaseStgMedium(pMedium);
        return ResultFromScode(S_OK);
    }
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP
QOleDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
{
#ifdef QDND_DEBUG
    qDebug("QOleDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)");
#endif

    if (!data)
        return ResultFromScode(DATA_E_FORMATETC);

    SCODE sc = S_OK;

    if (dwDirection == DATADIR_GET) {

        QVector<FORMATETC> fmtetcs = QWindowsMime::allFormatsForMime(data);
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(fmtetcs.size(), fmtetcs.data());
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;

    } else {

        FORMATETC formatetc;
        formatetc.cfFormat = CF_PERFORMEDDROPEFFECT;
        formatetc.dwAspect = DVASPECT_CONTENT;
        formatetc.lindex = -1;
        formatetc.ptd = NULL;
        formatetc.tymed = TYMED_HGLOBAL;
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(1, &formatetc);
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;
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


//---------------------------------------------------------------------
//                    QOleDropTarget
//---------------------------------------------------------------------

QOleDropTarget::QOleDropTarget(QWidget* w)
:   widget(w)
{
   m_refs = 1;
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
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)");
#endif

    if (!QApplicationPrivate::tryModalHelper(widget)) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    QDragManager *manager = QDragManager::self();
    manager->dropData->currentDataObject = pDataObj;
    manager->dropData->currentDataObject->AddRef();

    if (manager->dragPrivate()) manager->dragPrivate()->target = widget;
    manager->emitTargetChanged(widget);

    lastPoint = widget->mapFromGlobal(QPoint(pt.x,pt.y));
    lastKeyState = grfKeyState;

    QMimeData * md = manager->source() ? manager->dragPrivate()->data : manager->dropData;
    QDragEnterEvent e(lastPoint, translateToQDragDropActions(*pdwEffect), md,
                      toQtMouseButtons(grfKeyState), toQtKeyboardModifiers(grfKeyState));
    QApplication::sendEvent(widget, &e);


    answerRect = e.answerRect();
    if (e.isAccepted())
        choosenEffect = translateToWinDragEffects(e.dropAction());
    else
        choosenEffect = DROPEFFECT_NONE;
    *pdwEffect = choosenEffect;

    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)");
#endif

    if (!QApplicationPrivate::tryModalHelper(widget)) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    QPoint tmpPoint = widget->mapFromGlobal(QPoint(pt.x,pt.y));
    // see if we should compress this event
    if ((tmpPoint == lastPoint || answerRect.contains(tmpPoint)) && lastKeyState == grfKeyState) {
        *pdwEffect = choosenEffect;
        return NOERROR;
    }

    lastPoint = tmpPoint;
    lastKeyState = grfKeyState;

    QDragManager *manager = QDragManager::self();
    QMimeData *md = manager->source() ? manager->dragPrivate()->data : manager->dropData;
    QDragMoveEvent e(lastPoint, translateToQDragDropActions(*pdwEffect), md,
                     toQtMouseButtons(grfKeyState), toQtKeyboardModifiers(grfKeyState));
    if (choosenEffect != DROPEFFECT_NONE) {
        e.setDropAction(translateToQDragDropAction(choosenEffect));
        e.accept();
    }
    QApplication::sendEvent(widget, &e);

    answerRect = e.answerRect();
    if (e.isAccepted())
        choosenEffect = translateToWinDragEffects(e.dropAction());
    else
        choosenEffect = DROPEFFECT_NONE;
    *pdwEffect = choosenEffect;

    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragLeave()
{
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::DragLeave()");
#endif

    if (!QApplicationPrivate::tryModalHelper(widget)) {
        return NOERROR;
    }

    QDragLeaveEvent e;
    QApplication::sendEvent(widget, &e);

    QDragManager *manager = QDragManager::self();
    if (manager->dragPrivate()) manager->dragPrivate()->target = 0;
    manager->emitTargetChanged(widget);


    manager->dropData->currentDataObject->Release();
    manager->dropData->currentDataObject = 0;

    return NOERROR;
}

#define KEY_STATE_BUTTON_MASK (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)

STDMETHODIMP
QOleDropTarget::Drop(LPDATAOBJECT /*pDataObj*/, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
#ifdef QDND_DEBUG
    qDebug("QOleDropTarget::Drop(LPDATAOBJECT /*pDataObj*/, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)");
#endif

    if (!QApplicationPrivate::tryModalHelper(widget)) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    lastPoint = widget->mapFromGlobal(QPoint(pt.x,pt.y));
    // grfKeyState does not all ways contain button state in the drop so if
    // it doesn't then use the last known button state;
    if ((grfKeyState & KEY_STATE_BUTTON_MASK) == 0)
        grfKeyState |= lastKeyState & KEY_STATE_BUTTON_MASK;
    lastKeyState = grfKeyState;

    QDragManager *manager = QDragManager::self();
    QMimeData *md = manager->source() ? manager->dragPrivate()->data : manager->dropData;
    QDropEvent e(lastPoint, translateToQDragDropActions(*pdwEffect), md,
                 toQtMouseButtons(grfKeyState), toQtKeyboardModifiers(grfKeyState));
    QApplication::sendEvent(widget, &e);

    if (e.isAccepted()) {
        if (e.dropAction() == Qt::MoveAction || e.dropAction() == Qt::TargetMoveAction) {
            if (e.dropAction() == Qt::MoveAction)
                choosenEffect = DROPEFFECT_MOVE;
            else
                choosenEffect = DROPEFFECT_COPY;
            HGLOBAL hData = GlobalAlloc(0, sizeof(DWORD));
            if (hData) {
                DWORD *moveEffect = (DWORD *)GlobalLock(hData);;
                *moveEffect = DROPEFFECT_MOVE;
                GlobalUnlock(hData);
                STGMEDIUM medium;
                memset(&medium, 0, sizeof(STGMEDIUM));
                medium.tymed = TYMED_HGLOBAL;
                medium.hGlobal = hData;
                FORMATETC format;
                format.cfFormat = RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
                format.tymed = TYMED_HGLOBAL;
                format.ptd = 0;
                format.dwAspect = 1;
                format.lindex = -1;
                manager->dropData->currentDataObject->SetData(&format, &medium, true);
            }
        } else {
            choosenEffect = translateToWinDragEffects(e.dropAction());
        }
    } else {
        choosenEffect = DROPEFFECT_NONE;
    }
    *pdwEffect = choosenEffect;

    manager->dropData->currentDataObject->Release();
    manager->dropData->currentDataObject = 0;

    return NOERROR;

        // We won't get any mouserelease-event, so manually adjust qApp state:
///### test this        QApplication::winMouseButtonUp();
}

//---------------------------------------------------------------------
//                    QDropData
//---------------------------------------------------------------------

bool QDropData::hasFormat(const QString &mimeType) const
{
    if (!currentDataObject) // Sanity
        return false;

    return QWindowsMime::converterToMime(mimeType, currentDataObject) != 0;
}

QStringList QDropData::formats() const
{
    QStringList fmts;
    if (!currentDataObject) // Sanity
        return fmts;

    fmts = QWindowsMime::allMimesForFormats(currentDataObject);

    return fmts;
}

QVariant QDropData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
    QVariant result;

    if (!currentDataObject) // Sanity
        return result;

    QWindowsMime *converter = QWindowsMime::converterToMime(mimeType, currentDataObject);

    if (converter) {
        result = converter->convertToMime(mimeType, currentDataObject, type);
        // if we just got a bytearray but we wanted more then try to decode
        if (result.type() == QVariant::ByteArray && type != QVariant::ByteArray) {
            QDropData *that = const_cast<QDropData *>(this);
            that->setData(mimeType, result.toByteArray());
            result = QMimeData::retrieveData(mimeType, type);
            that->clear();
        }
    }

    return result;
}

Qt::DropAction QDragManager::drag(QDrag *o)

{
#ifdef QDND_DEBUG
    qDebug("QDragManager::drag(QDrag *drag)");
#endif

    if (object == o || !o || !o->d_func()->source)
        return Qt::IgnoreAction;

    if (object) {
        cancel();
        qApp->removeEventFilter(this);
        beingCancelled = false;
    }

    object = o;

#ifdef QDND_DEBUG
    qDebug("actions = %s", dragActionsToString(dragPrivate()->possible_actions).toLatin1().data());
#endif

    dragPrivate()->target = 0;

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropStart);
#endif

    QStringList fmts = o->mimeData()->formats();
    for(int i = 0; i < fmts.size(); ++i)
        QWindowsMime::registerMimeType(fmts.at(i).toLatin1());

    DWORD resultEffect;
    QOleDropSource *src = new QOleDropSource();
    src->createCursors();
    QOleDataObject *obj = new QOleDataObject(o->mimeData());
    DWORD allowedEffects = translateToWinDragEffects(dragPrivate()->possible_actions);

#ifdef Q_OS_TEMP
    HRESULT r = 0;
    resultEffect = 0;
#else
    HRESULT r = DoDragDrop(obj, src, allowedEffects, &resultEffect);
#endif

    Qt::DropAction ret = Qt::IgnoreAction;
    if (r == DRAGDROP_S_DROP) {
        if (obj->reportedPerformedEffect() == DROPEFFECT_MOVE && resultEffect != DROPEFFECT_MOVE) {
            ret = Qt::TargetMoveAction;
            resultEffect = DROPEFFECT_MOVE;
        } else {
            ret = translateToQDragDropAction(resultEffect);
        }
        // Force it to be a copy if an unsuported operation occured.
        // This indicates a bug in the drop target.
        if (resultEffect != DROPEFFECT_NONE && !(resultEffect & allowedEffects))
            ret = Qt::CopyAction;
    } else {
        dragPrivate()->target = 0;
    }

    // clean up
    obj->releaseQt();
    obj->Release();        // Will delete obj if refcount becomes 0
    src->Release();        // Will delete src if refcount becomes 0
    object = 0;
    o->setMimeData(0);
    o->deleteLater();

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropEnd);
#endif

    return ret;
}

void QDragManager::cancel(bool /* deleteSource */)
{
    if (object) {
        beingCancelled = true;
        object = 0;
    }

#ifndef QT_NO_CURSOR
    // insert cancel code here ######## todo

    if (restoreCursor) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropEnd);
#endif
}


void qt_olednd_unregister(QWidget* widget, QOleDropTarget *dst)
{
    dst->releaseQt();
    dst->Release();
#ifndef Q_OS_TEMP
    CoLockObjectExternal(dst, false, true);
    RevokeDragDrop(widget->winId());
#endif
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

void QDragManager::updatePixmap()
{
    // not used in windows implementation
}

bool QDragManager::eventFilter(QObject *, QEvent *)
{
    // not used in windows implementation
    return false;
}

void QDragManager::timerEvent(QTimerEvent*)
{
    // not used in windows implementation
}

void QDragManager::move(const QPoint &)
{
    // not used in windows implementation
}

void QDragManager::drop()
{
    // not used in windows implementation
}

#endif // QT_NO_DRAGANDDROP



