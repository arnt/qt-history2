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

#include "qaxwidget.h"
#include <activeqt/qaxaggregated.h>

#include <qabstracteventdispatcher.h>
#include <qapplication.h>
#include <private/qapplication_p.h>
#include <qdockwidget.h>
#include <qevent.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qmetaobject.h>
#include <qpainter.h>
#include <qpointer.h>
#include <qregexp.h>
#include <quuid.h>
#include <qwhatsthis.h>

#include <ocidl.h>
#include <olectl.h>
#include <docobj.h>

// #define QAX_DEBUG

#ifdef QAX_DEBUG
#define AX_DEBUG(x) qDebug(#x);
#else
#define AX_DEBUG(x);
#endif

// missing interface from win32api
#if defined(Q_CC_GNU) 
#   if !defined(IOleInPlaceObjectWindowless)
#       undef INTERFACE
#       define INTERFACE IOleInPlaceObjectWindowless
        DECLARE_INTERFACE_(IOleInPlaceObjectWindowless,IOleInPlaceObject)
        {
	   STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	   STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	   STDMETHOD_(ULONG,Release)(THIS) PURE;
	   STDMETHOD(GetWindow)(THIS_ HWND*) PURE;
	   STDMETHOD(ContextSensitiveHelp)(THIS_ BOOL) PURE;
	   STDMETHOD(InPlaceDeactivate)(THIS) PURE;
	   STDMETHOD(UIDeactivate)(THIS) PURE;
	   STDMETHOD(SetObjectRects)(THIS_ LPCRECT,LPCRECT) PURE;
	   STDMETHOD(ReactivateAndUndo)(THIS) PURE;
           STDMETHOD(OnWindowMessage)(THIS_ UINT, WPARAM, LPARAM, LRESULT*) PURE;
           STDMETHOD(GetDropTarget)(THIS_ IDropTarget**) PURE;
        };
#   endif
#endif

#include "../shared/qaxtypes.h"

static QAbstractEventDispatcher::EventFilter previous_filter = 0;
static int filter_ref = 0;

static ushort mouseTbl[] = {
    WM_MOUSEMOVE,	QEvent::MouseMove,		0,
    WM_LBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::LeftButton,
    WM_LBUTTONUP,	QEvent::MouseButtonRelease,	Qt::LeftButton,
    WM_LBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::LeftButton,
    WM_RBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::RightButton,
    WM_RBUTTONUP,	QEvent::MouseButtonRelease,	Qt::RightButton,
    WM_RBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::RightButton,
    WM_MBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::MidButton,
    WM_MBUTTONUP,	QEvent::MouseButtonRelease,	Qt::MidButton,
    WM_MBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::MidButton,
    0,			0,				0
};

static Qt::MouseButtons translateMouseButtonState(int s, int type, int button)
{
    Qt::MouseButtons bst = 0;
    if (s & MK_LBUTTON)
        bst |= Qt::LeftButton;
    if (s & MK_MBUTTON)
        bst |= Qt::MidButton;
    if (s & MK_RBUTTON)
        bst |= Qt::RightButton;
    
    // Translate from Windows-style "state after event"
    // to X-style "state before event"
    if (type == QEvent::MouseButtonPress ||
        type == QEvent::MouseButtonDblClick)
        bst &= ~button;
    else if (type == QEvent::MouseButtonRelease)
        bst |= (Qt::MouseButton)button;
    
    return bst;
}

static Qt::KeyboardModifiers translateModifierState(int s)
{
    Qt::KeyboardModifiers bst = 0;
    if (s & MK_SHIFT)
        bst |= Qt::ShiftModifier;
    if (s & MK_CONTROL)
        bst |= Qt::ControlModifier;
    if (GetKeyState(VK_MENU) < 0)
        bst |= Qt::AltModifier;

    return bst;
}

/*  \class QAxHostWidget qaxwidget.cpp
    \brief The QAxHostWidget class is the actual container widget.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \internal
*/
class QAxHostWidget : public QWidget
{
    friend class QAxClientSite;
public:
    QAxHostWidget(QWidget *parent, QAxClientSite *ax);
    ~QAxHostWidget();
    
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    
    int qt_metacall(QMetaObject::Call, int isignal, void **argv);
    inline QAxClientSite *clientSite() const
    {
        return axhost;
    }
    
protected:
    bool winEvent(MSG *msg, long *result);
    bool event(QEvent *e);
    bool eventFilter(QObject *o, QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void paintEvent(QPaintEvent *e);
        
private:
    int setFocusTimer;
    bool hasFocus;
    QAxClientSite *axhost;
};

/*  \class QAxClientSite qaxwidget.cpp
    \brief The QAxClientSite class implements the client site interfaces.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \internal
*/
class QAxClientSite : public IDispatch,
                    public IOleClientSite,
                    public IOleControlSite,
                    public IOleInPlaceSiteWindowless,
                    public IOleInPlaceFrame,
                    public IOleDocumentSite,
                    public IAdviseSink
{
    friend class QAxHostWidget;
public:
    QAxClientSite(QAxWidget *c);
    virtual ~QAxClientSite();

    bool activateObject(bool initialized);

    void releaseAll();
    void deactivate();
    inline void reset(QWidget *p)
    {
        if (widget == p)
            widget = 0;
        else if (host == p)
            host = 0;
    }

    inline IOleInPlaceActiveObject *inPlaceObject() const
    {
        return m_spInPlaceActiveObject;
    }
    
    // IUnknown
    unsigned long WINAPI AddRef();
    unsigned long WINAPI Release();
    STDMETHOD(QueryInterface)(REFIID iid, void **iface);
    
    // IDispatch
    HRESULT __stdcall GetTypeInfoCount(unsigned int *) { return E_NOTIMPL; }
    HRESULT __stdcall GetTypeInfo(UINT, LCID, ITypeInfo **) { return E_NOTIMPL; }
    HRESULT __stdcall GetIDsOfNames(const _GUID &, wchar_t **, unsigned int, unsigned long, long *) { return E_NOTIMPL; }
    HRESULT __stdcall Invoke(DISPID dispIdMember, 
        REFIID riid, 
        LCID lcid, 
        WORD wFlags, 
        DISPPARAMS *pDispParams, 
        VARIANT *pVarResult, 
        EXCEPINFO *pExcepInfo, 
        UINT *puArgErr);
    void emitAmbientPropertyChange(DISPID dispid);
    
    // IOleClientSite
    STDMETHOD(SaveObject)();
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
    STDMETHOD(GetContainer)(LPOLECONTAINER FAR* ppContainer);
    STDMETHOD(ShowObject)();
    STDMETHOD(OnShowWindow)(BOOL fShow);
    STDMETHOD(RequestNewObjectLayout)();
    
    // IOleControlSite
    STDMETHOD(OnControlInfoChanged)();
    STDMETHOD(LockInPlaceActive)(BOOL fLock);
    STDMETHOD(GetExtendedControl)(IDispatch** ppDisp);
    STDMETHOD(TransformCoords)(POINTL* pPtlHimetric, POINTF* pPtfContainer, DWORD dwFlags);
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, DWORD grfModifiers);
    STDMETHOD(OnFocus)(BOOL fGotFocus);
    STDMETHOD(ShowPropertyFrame)();
    
    // IOleWindow
    STDMETHOD(GetWindow)(HWND *phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
    
    // IOleInPlaceSite
    STDMETHOD(CanInPlaceActivate)();
    STDMETHOD(OnInPlaceActivate)();
    STDMETHOD(OnUIActivate)();
    STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD(Scroll)(SIZE scrollExtant);
    STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
    STDMETHOD(OnInPlaceDeactivate)();
    STDMETHOD(DiscardUndoState)();
    STDMETHOD(DeactivateAndUndo)();
    STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

// IOleInPlaceSiteEx ###
    STDMETHOD(OnInPlaceActivateEx)(BOOL* /*pfNoRedraw*/, DWORD /*dwFlags*/)
    {
        return S_OK;
    }
    STDMETHOD(OnInPlaceDeactivateEx)(BOOL /*fNoRedraw*/)
    {
        return S_OK;
    }
    STDMETHOD(RequestUIActivate)()
    {
        return S_OK;
    }

// IOleInPlaceSiteWindowless ###
    STDMETHOD(CanWindowlessActivate)()
    {
        return S_OK;
    }
    STDMETHOD(GetCapture)()
    {
        return S_FALSE;
    }
    STDMETHOD(SetCapture)(BOOL /*fCapture*/)
    {
        return S_FALSE;
    }
    STDMETHOD(GetFocus)()
    {
        return S_FALSE;
    }
    STDMETHOD(SetFocus)(BOOL /*fCapture*/)
    {
        return S_FALSE;
    }
    STDMETHOD(GetDC)(LPCRECT /*pRect*/, DWORD /*grfFlags*/, HDC *phDC)
    {
        *phDC = 0;
        return S_OK;
    }
    STDMETHOD(ReleaseDC)(HDC hDC)
    {
        ::ReleaseDC(widget->winId(), hDC);
        return S_OK;
    }
    STDMETHOD(InvalidateRect)(LPCRECT pRect, BOOL fErase)
    {
        ::InvalidateRect(host->winId(), pRect, fErase);
        return S_OK;
    }
    STDMETHOD(InvalidateRgn)(HRGN hRGN, BOOL fErase)
    {
        ::InvalidateRgn(host->winId(), hRGN, fErase);
        return S_OK;
    }
    STDMETHOD(ScrollRect)(int /*dx*/, int /*dy*/, LPCRECT /*pRectScroll*/, LPCRECT /*pRectClip*/)
    {
        return S_OK;
    }
    STDMETHOD(AdjustRect)(LPRECT /*prc*/)
    {
        return S_OK;
    }
#ifdef Q_CC_GNU // signature incorrect in win32api
    STDMETHOD(AdjustRect)(LPCRECT /*prc*/)
    {
        RECT rect;
        return AdjustRect(&rect);
    }
#endif

    STDMETHOD(OnDefWindowMessage)(UINT /*msg*/, WPARAM /*wPara*/, LPARAM /*lParam*/, LRESULT* /*plResult*/)
    {
        return S_FALSE;
    }

    // IOleInPlaceFrame
    STDMETHOD(InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths));
    STDMETHOD(SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject));
    STDMETHOD(RemoveMenus(HMENU hmenuShared));
    STDMETHOD(SetStatusText(LPCOLESTR pszStatusText));
    STDMETHOD(EnableModeless(BOOL fEnable));
    STDMETHOD(TranslateAccelerator(LPMSG lpMsg, WORD grfModifiers));
    
    // IOleInPlaceUIWindow
    STDMETHOD(GetBorder(LPRECT lprectBorder));
    STDMETHOD(RequestBorderSpace(LPCBORDERWIDTHS pborderwidths));
    STDMETHOD(SetBorderSpace(LPCBORDERWIDTHS pborderwidths));
    STDMETHOD(SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName));
    
    // IOleDocumentSite
    STDMETHOD(ActivateMe(IOleDocumentView *pViewToActivate));

    // IAdviseSink
    STDMETHOD_(void, OnDataChange)(FORMATETC* /*pFormatetc*/, STGMEDIUM* /*pStgmed*/)
    {
        AX_DEBUG(QAxClientSite::OnDataChange);
    }
    STDMETHOD_(void, OnViewChange)(DWORD /*dwAspect*/, LONG /*lindex*/)
    {
        AX_DEBUG(QAxClientSite::OnViewChange);
    }
    STDMETHOD_(void, OnRename)(IMoniker* /*pmk*/)
    {
    }
    STDMETHOD_(void, OnSave)()
    {
    }
    STDMETHOD_(void, OnClose)()
    {
    }
    
    QSize sizeHint() const { return sizehint; }
    QSize minimumSizeHint() const;
    inline void resize(QSize sz) { if (host) host->resize(sz); }

    bool translateKeyEvent(int message, int keycode) const
    {
        if (!widget)
            return false;
        return widget->translateKeyEvent(message, keycode);
    }
    
    int qt_metacall(QMetaObject::Call, int isignal, void **argv);
    void windowActivationChange();
    
private:
    struct OleMenuItem {
        OleMenuItem(HMENU hm = 0, int ID = 0, QMenu *menu = 0)
            : hMenu(hm), id(ID), subMenu(menu)
        {}
        HMENU hMenu;
        int id;
        QMenu *subMenu;
    };
    QMenu *generatePopup(HMENU subMenu, QWidget *parent);
    
    IOleObject *m_spOleObject;
    IOleControl *m_spOleControl;
    IOleInPlaceObjectWindowless *m_spInPlaceObject;
    IOleInPlaceActiveObject *m_spInPlaceActiveObject;
    IOleDocumentView *m_spActiveView;
    
    QAxAggregated *aggregatedObject;

    bool inPlaceObjectWindowless :1;
    bool inPlaceModelessEnabled :1;
    
    DWORD m_dwOleObject;
    HWND m_menuOwner;
    CONTROLINFO control_info;
    
    QSize sizehint;
    unsigned long ref;
    QAxWidget *widget;
    QAxHostWidget *host;
    QPointer<QMenuBar> menuBar;
    QMap<QAction*,OleMenuItem> menuItemMap;
};

// The filter procedure listening to user interaction on the control
bool axc_FilterProc(void *m)
{
    MSG *msg = (MSG*)m;
    const uint message = msg->message;
    if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) || (message >= WM_KEYFIRST && message <= WM_KEYLAST)) {
        HWND hwnd = msg->hwnd;
        QWidget *widget = 0;
        QAxWidget *ax = 0;
        while (!ax && hwnd) {
            widget = QWidget::find(hwnd);
            if (widget)
                ax = qobject_cast<QAxWidget*>(widget);
            hwnd = ::GetParent(hwnd);
        }
        if (ax && msg->hwnd != ax->winId()) {
            if (message >= WM_KEYFIRST && message <= WM_KEYLAST) {
                QAxHostWidget *host = qFindChild<QAxHostWidget*>(ax);
                QAxClientSite *site = host ? host->clientSite() : 0;
                if (site && site->inPlaceObject() && site->translateKeyEvent(msg->message, msg->wParam))
                    site->inPlaceObject()->TranslateAccelerator(msg);
            } else {
                int i;
                for (i = 0; (UINT)mouseTbl[i] != message && mouseTbl[i]; i += 3)
                    ;
                if (mouseTbl[i]) {
                    QEvent::Type type = (QEvent::Type)mouseTbl[++i];
                    int button = mouseTbl[++i];
                    DWORD ol_pos = GetMessagePos();
                    QPoint gpos(LOWORD(ol_pos), HIWORD(ol_pos));
                    QPoint pos = widget->mapFromGlobal(gpos);

		    QMouseEvent e(type, pos, gpos, (Qt::MouseButton)button,
			    translateMouseButtonState(msg->wParam, type, button),
			    translateModifierState(msg->wParam));
                    QApplication::sendEvent(ax, &e);
                }
            }
        }
    }

    if (previous_filter)
        return previous_filter(m);

    return false;
}

QAxClientSite::QAxClientSite(QAxWidget *c)
: ref(1), widget(c)
{
    aggregatedObject = widget->createAggregate();
    if (aggregatedObject) {
	aggregatedObject->controlling_unknown = (IUnknown*)(IDispatch*)this;
	aggregatedObject->the_object = c;
    }

    m_spOleObject = 0;
    m_spOleControl = 0;
    m_spInPlaceObject = 0;
    m_spInPlaceActiveObject = 0;
    m_spActiveView = 0;

    inPlaceObjectWindowless = false;
    inPlaceModelessEnabled = true;
    m_dwOleObject = 0;
    m_menuOwner = 0;
    memset(&control_info, 0, sizeof(control_info));
    
    menuBar = 0;
}

bool QAxClientSite::activateObject(bool initialized)
{
    host = new QAxHostWidget(widget, this);

    bool showHost = false;
    HRESULT hr = S_OK;
    m_spOleObject = 0;
    widget->queryInterface(IID_IOleObject, (void**)&m_spOleObject);
    if (m_spOleObject) {
        DWORD dwMiscStatus = 0;
        m_spOleObject->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);

        IOleDocument *document = 0;
        m_spOleObject->QueryInterface(IID_IOleDocument, (void**)&document);
        if (document) {
            // activate as document server
            IStorage *storage = 0;
            ILockBytes * bytes = 0;
            HRESULT hres = ::CreateILockBytesOnHGlobal(0, TRUE, &bytes);
            hres = ::StgCreateDocfileOnILockBytes(bytes, STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &storage);

            IPersistStorage *persistStorage = 0;
            document->QueryInterface(IID_IPersistStorage, (void**)&persistStorage);
            if (persistStorage) {
                persistStorage->InitNew(storage);
                persistStorage->Release();
            }

            storage->Release();
            bytes->Release();

            m_spOleObject->SetClientSite(this);
            OleRun(m_spOleObject);

            document->Release();
        } else {
            // activate as control
            if(dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
                m_spOleObject->SetClientSite(this);
        
            if (!initialized) {
                IPersistStreamInit *spPSI = 0;
                m_spOleObject->QueryInterface(IID_IPersistStreamInit, (void**)&spPSI);
                if (spPSI) {
                    spPSI->InitNew();
                    spPSI->Release();
                }
            }
        
            if(!(dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST))
                m_spOleObject->SetClientSite(this);
        }

        IViewObject *spViewObject = 0;
        m_spOleObject->QueryInterface(IID_IViewObject, (void**) &spViewObject);
        
        m_spOleObject->Advise(this, &m_dwOleObject);
        IAdviseSink *spAdviseSink = 0;
        QueryInterface(IID_IAdviseSink, (void**)&spAdviseSink);
        if (spAdviseSink && spViewObject) {
            if (spViewObject)
                spViewObject->SetAdvise(DVASPECT_CONTENT, 0, spAdviseSink);
            spAdviseSink->Release();
        }
        if (spViewObject)
            spViewObject->Release();
        
        m_spOleObject->SetHostNames(OLESTR("AXWIN"), 0);
        
        if (!(dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME)) {
            SIZEL hmSize;
            hmSize.cx = MAP_PIX_TO_LOGHIM(250, widget->logicalDpiX());
            hmSize.cy = MAP_PIX_TO_LOGHIM(250, widget->logicalDpiY());
            
            m_spOleObject->SetExtent(DVASPECT_CONTENT, &hmSize);
            m_spOleObject->GetExtent(DVASPECT_CONTENT, &hmSize);
            
            sizehint.setWidth(MAP_LOGHIM_TO_PIX(hmSize.cx, widget->logicalDpiX()));
            sizehint.setHeight(MAP_LOGHIM_TO_PIX(hmSize.cy, widget->logicalDpiY()));
            showHost = true;
        } else {
            sizehint = QSize(0, 0);
            host->hide();
        }
        if (!(dwMiscStatus & OLEMISC_NOUIACTIVATE)) {
            host->setFocusPolicy(Qt::StrongFocus);
        } else {
            host->setFocusPolicy(Qt::NoFocus);
        }
        
        RECT rcPos = { host->x(), host->y(), host->x()+sizehint.width(), host->y()+sizehint.height() };
        
        hr = m_spOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, 0, (IOleClientSite*)this, 0, host->winId(), &rcPos);
        
        m_spOleObject->QueryInterface(IID_IOleControl, (void**)&m_spOleControl);
        if (m_spOleControl) {
            m_spOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_BACKCOLOR);
            m_spOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_FORECOLOR);
            m_spOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_FONT);
            
            control_info.cb = sizeof(control_info);
            m_spOleControl->GetControlInfo(&control_info);
        }
        
        BSTR userType;
        m_spOleObject->GetUserType(USERCLASSTYPE_SHORT, &userType);
        widget->setWindowTitle(QString::fromUtf16(userType));
        CoTaskMemFree(userType);
    } else {
        IObjectWithSite *spSite = 0;
        widget->queryInterface(IID_IObjectWithSite, (void**)&spSite);
        if (spSite) {
            spSite->SetSite((IUnknown*)(IDispatch*)this);
            spSite->Release();
        }
    }

    host->resize(widget->size());
    if (showHost)
        host->show();

    if (host->focusPolicy() != Qt::NoFocus) {
        widget->setFocusProxy(host);
        widget->setFocusPolicy(host->focusPolicy());
    }

    return true;
}

QAxClientSite::~QAxClientSite()
{
    if (host) {
        host->axhost = 0;
    }

    if (aggregatedObject)
        aggregatedObject->the_object = 0;
    delete aggregatedObject;
    delete host;
}

void QAxClientSite::releaseAll()
{
    if (m_spOleObject) {
        m_spOleObject->SetClientSite(0);
        m_spOleObject->Unadvise(m_dwOleObject);
        m_spOleObject->Release();
    }
    m_spOleObject = 0;
    if (m_spOleControl) m_spOleControl->Release();
    m_spOleControl = 0;
    if (m_spInPlaceObject) m_spInPlaceObject->Release();
    m_spInPlaceObject = 0;
    if (m_spInPlaceActiveObject) m_spInPlaceActiveObject->Release();
    m_spInPlaceActiveObject = 0;

    inPlaceObjectWindowless = false;
}

void QAxClientSite::deactivate()
{
    if (m_spInPlaceObject) m_spInPlaceObject->InPlaceDeactivate();
    // if this assertion fails the control didn't call OnInPlaceDeactivate
    Q_ASSERT(m_spInPlaceObject == 0); 
}

//**** IUnknown
unsigned long WINAPI QAxClientSite::AddRef()
{
    return ++ref;
}

unsigned long WINAPI QAxClientSite::Release()
{
    if (!--ref) {
        delete this;
        return 0;
    }
    return ref;
}

HRESULT WINAPI QAxClientSite::QueryInterface(REFIID iid, void **iface)
{
    *iface = 0;
    
    if (iid == IID_IUnknown) {
        *iface = (IUnknown*)(IDispatch*)this;
    } else {
	HRESULT res = S_OK;
	if (aggregatedObject)
	    res = aggregatedObject->queryInterface(iid, iface);
	if (*iface)
	    return res;
    }

    if (!(*iface)) {
        if (iid == IID_IDispatch)
            *iface = (IDispatch*)this;
        else if (iid == IID_IOleClientSite)
            *iface = (IOleClientSite*)this;
        else if (iid == IID_IOleControlSite)
            *iface = (IOleControlSite*)this;
        else if (iid == IID_IOleWindow)
            *iface = (IOleWindow*)(IOleInPlaceSite*)this;
        else if (iid == IID_IOleInPlaceSite)
            *iface = (IOleInPlaceSite*)this;
        else if (iid == IID_IOleInPlaceFrame)
            *iface = (IOleInPlaceFrame*)this;
        else if (iid == IID_IOleInPlaceUIWindow)
            *iface = (IOleInPlaceUIWindow*)this;
        else if (iid == IID_IOleDocumentSite)
            *iface = (IOleDocumentSite*)this;
        else if (iid == IID_IAdviseSink)
            *iface = (IAdviseSink*)this;
    }
    if (!*iface)
	return E_NOINTERFACE;
    
    AddRef();
    return S_OK;
}

#if defined(QT_PLUGIN)
extern bool runsInDesignMode;
#endif

//**** IDispatch
HRESULT WINAPI QAxClientSite::Invoke(DISPID dispIdMember,
                                     REFIID /*riid*/,
                                     LCID /*lcid*/,
                                     WORD /*wFlags*/,
                                     DISPPARAMS * /*pDispParams*/,
                                     VARIANT *pVarResult,
                                     EXCEPINFO * /*pExcepInfo*/,
                                     UINT * /*puArgErr*/)
{
    if (!pVarResult)
        return E_POINTER;
    if (!widget || !host)
        return E_UNEXPECTED;
    
    switch(dispIdMember) {
    case DISPID_AMBIENT_USERMODE:
#if defined(QT_PLUGIN)
        pVarResult->vt = VT_BOOL;
        if (runsInDesignMode) {
            bool isPreview = !host->window()->inherits("MainWindow");
            pVarResult->boolVal = isPreview;
        } else {
            pVarResult->boolVal = true;
        }
        return S_OK;
#endif
        
    case DISPID_AMBIENT_AUTOCLIP:	
    case DISPID_AMBIENT_SUPPORTSMNEMONICS:
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = true;
        return S_OK;
        
    case DISPID_AMBIENT_SHOWHATCHING:
    case DISPID_AMBIENT_SHOWGRABHANDLES:
    case DISPID_AMBIENT_DISPLAYASDEFAULT:
    case DISPID_AMBIENT_MESSAGEREFLECT:
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = false;
        return S_OK;
        
    case DISPID_AMBIENT_DISPLAYNAME:
        pVarResult->vt = VT_BSTR;
        pVarResult->bstrVal = QStringToBSTR(widget->windowTitle());
        return S_OK;
        
    case DISPID_AMBIENT_FONT:
        pVarResult->vt = VT_DISPATCH;
        pVarResult->pdispVal = QFontToIFont(widget->font());
        return S_OK;
        
    case DISPID_AMBIENT_BACKCOLOR:
        pVarResult->vt = VT_UI4;
        pVarResult->lVal = QColorToOLEColor(widget->palette().color(widget->backgroundRole()));
        return S_OK;
        
    case DISPID_AMBIENT_FORECOLOR:
        pVarResult->vt = VT_UI4;
        pVarResult->lVal = QColorToOLEColor(widget->palette().color(widget->foregroundRole()));
        return S_OK;
        
    case DISPID_AMBIENT_UIDEAD:
        pVarResult->vt = VT_BOOL;
        pVarResult->boolVal = !widget->isEnabled();
        return S_OK;
        
    default:
        break;
    }
    
    return DISP_E_MEMBERNOTFOUND;
}

void QAxClientSite::emitAmbientPropertyChange(DISPID dispid)
{
    if (m_spOleControl)
        m_spOleControl->OnAmbientPropertyChange(dispid);
}

//**** IOleClientSite
HRESULT WINAPI QAxClientSite::SaveObject()
{
    return E_NOTIMPL;
}

HRESULT WINAPI QAxClientSite::GetMoniker(DWORD, DWORD, IMoniker **ppmk)
{
    if (!ppmk)
        return E_POINTER;
    
    *ppmk = 0;
    return E_NOTIMPL;
}

HRESULT WINAPI QAxClientSite::GetContainer(LPOLECONTAINER *ppContainer)
{
    if (!ppContainer)
        return E_POINTER;
    
    *ppContainer = 0;
    return E_NOINTERFACE;
}

HRESULT WINAPI QAxClientSite::ShowObject()
{
    return S_OK;
}

HRESULT WINAPI QAxClientSite::OnShowWindow(BOOL /*fShow*/)
{
    return S_OK;
}

HRESULT WINAPI QAxClientSite::RequestNewObjectLayout()
{
    return E_NOTIMPL;
}

//**** IOleControlSite
HRESULT WINAPI QAxClientSite::OnControlInfoChanged()
{
    if (m_spOleControl)
        m_spOleControl->GetControlInfo(&control_info);
    
    return S_OK;
}

HRESULT WINAPI QAxClientSite::LockInPlaceActive(BOOL /*fLock*/)
{
    AX_DEBUG(QAxClientSite::LockInPlaceActive);
    return S_OK;
}

HRESULT WINAPI QAxClientSite::GetExtendedControl(IDispatch** ppDisp)
{
    if (!ppDisp)
        return E_POINTER;
    
    *ppDisp = 0;
    return E_NOTIMPL;
}

HRESULT WINAPI QAxClientSite::TransformCoords(POINTL* /*pPtlHimetric*/, POINTF* /*pPtfContainer*/, DWORD /*dwFlags*/)
{
    return S_OK;
}

HRESULT WINAPI QAxClientSite::TranslateAccelerator(LPMSG lpMsg, DWORD /*grfModifiers*/)
{
    QT_WA_INLINE(
        SendMessage(host->winId(), lpMsg->message, lpMsg->wParam, lpMsg->lParam),
        SendMessageA(host->winId(), lpMsg->message, lpMsg->wParam, lpMsg->lParam)
    );
    return S_OK;
}

HRESULT WINAPI QAxClientSite::OnFocus(BOOL bGotFocus)
{
    AX_DEBUG(QAxClientSite::OnFocus);
    if (host) {
        host->hasFocus = bGotFocus;
        qApp->removeEventFilter(host);
        if (bGotFocus)
            qApp->installEventFilter(host);
    }
    return S_OK;
}

HRESULT WINAPI QAxClientSite::ShowPropertyFrame()
{
    return E_NOTIMPL;
}

//**** IOleWindow
HRESULT WINAPI QAxClientSite::GetWindow(HWND *phwnd)
{
    if (!phwnd)
        return E_POINTER;
    
    *phwnd = host->winId();
    return S_OK;
}

HRESULT WINAPI QAxClientSite::ContextSensitiveHelp(BOOL fEnterMode)
{
    if (fEnterMode)
        QWhatsThis::enterWhatsThisMode();
    else
        QWhatsThis::leaveWhatsThisMode();
    
    return S_OK;
}

//**** IOleInPlaceSite
HRESULT WINAPI QAxClientSite::CanInPlaceActivate()
{
    AX_DEBUG(QAxClientSite::CanInPlaceActivate);
    return S_OK;
}

HRESULT WINAPI QAxClientSite::OnInPlaceActivate()
{
    AX_DEBUG(QAxClientSite::OnInPlaceActivate);
    OleLockRunning(m_spOleObject, true, false);
    if (!m_spInPlaceObject) {
/* ### disabled for now
        m_spOleObject->QueryInterface(IID_IOleInPlaceObjectWindowless, (void**) &m_spInPlaceObject);
*/
        if (m_spInPlaceObject) {
            inPlaceObjectWindowless = true;
        } else {
            inPlaceObjectWindowless = false;
            m_spOleObject->QueryInterface(IID_IOleInPlaceObject, (void**) &m_spInPlaceObject);
        }
    }
    
    return S_OK;
}

HRESULT WINAPI QAxClientSite::OnUIActivate()
{
    AX_DEBUG(QAxClientSite::OnUIActivate);
    return S_OK;
}

HRESULT WINAPI QAxClientSite::GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    if (!ppFrame || !ppDoc || !lprcPosRect || !lprcClipRect || !lpFrameInfo)
        return E_POINTER;
    
    QueryInterface(IID_IOleInPlaceFrame, (void**)ppFrame);
    QueryInterface(IID_IOleInPlaceUIWindow, (void**)ppDoc);
    
    ::GetClientRect(host->winId(), lprcPosRect);
    ::GetClientRect(host->winId(), lprcClipRect);
    
    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp = false;
    lpFrameInfo->haccel = 0;
    lpFrameInfo->cAccelEntries = 0;
    lpFrameInfo->hwndFrame = widget ? widget->window()->winId() : 0;
    
    return S_OK;
}

HRESULT WINAPI QAxClientSite::Scroll(SIZE /*scrollExtant*/)
{
    return S_FALSE;
}

HRESULT WINAPI QAxClientSite::OnUIDeactivate(BOOL)
{
    AX_DEBUG(QAxClientSite::OnUIDeactivate);
    if (host && host->hasFocus) {
        qApp->removeEventFilter(host);
        host->hasFocus = FALSE;
    }
    return S_OK;
}

HRESULT WINAPI QAxClientSite::OnInPlaceDeactivate()
{
    AX_DEBUG(QAxClientSite::OnInPlaceDeactivate);
    if (m_spInPlaceObject)
        m_spInPlaceObject->Release();
    m_spInPlaceObject = 0;
    inPlaceObjectWindowless = false;
    
    return S_OK;
}

HRESULT WINAPI QAxClientSite::DiscardUndoState()
{
    return S_OK;
}

HRESULT WINAPI QAxClientSite::DeactivateAndUndo()
{
    if (m_spInPlaceObject)
        m_spInPlaceObject->UIDeactivate();

    return S_OK;
}

HRESULT WINAPI QAxClientSite::OnPosRectChange(LPCRECT /*lprcPosRect*/)
{
    AX_DEBUG(QAxClientSite::OnPosRectChange);
    // ###
    return S_OK;
}

//**** IOleInPlaceFrame
HRESULT WINAPI QAxClientSite::InsertMenus(HMENU /*hmenuShared*/, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    AX_DEBUG(QAxClientSite::InsertMenus);
    QMenuBar *mb = menuBar;
    if (!mb)
	mb = qFindChild<QMenuBar*>(widget->window());
    if (!mb)
	return E_NOTIMPL;
    menuBar = mb;

    QMenu *fileMenu = 0;
    QMenu *viewMenu = 0;
    QMenu *windowMenu = 0;
    QList<QAction*> actions = menuBar->actions();
    for (int i = 0; i < actions.count(); ++i) {
        QAction *action = actions.at(i);
	QString text = action->text();
	if (text == "&File") {
            fileMenu = action->menu();
	} else if (text == "&View") {
	    viewMenu = action->menu();
	} else if (text == "&Window") {
	    windowMenu = action->menu();
	}
    }
    if (fileMenu)
	lpMenuWidths->width[0] = fileMenu->actions().count();
    if (viewMenu)
	lpMenuWidths->width[2] = viewMenu->actions().count();
    if (windowMenu)
	lpMenuWidths->width[4] = windowMenu->actions().count();

    return S_OK;
}

static int menuItemEntry(HMENU menu, int index, MENUITEMINFOA item, QString &text, QPixmap &/*icon*/)
{
    if (item.fType == MFT_STRING && item.cch) {
        char *titlebuf = new char[item.cch+1];
        item.dwTypeData = titlebuf;
        item.cch++;
        ::GetMenuItemInfoA(menu, index, true, &item);
        text = QString::fromLocal8Bit(titlebuf);
        delete []titlebuf;
        return MFT_STRING;
    } 
#if 0
    else if (item.fType == MFT_BITMAP) {
        HBITMAP hbm = (HBITMAP)LOWORD(item.hbmpItem);
        SIZE bmsize;
        GetBitmapDimensionEx(hbm, &bmsize);
        QPixmap pixmap(1,1);
        QSize sz(MAP_LOGHIM_TO_PIX(bmsize.cx, pixmap.logicalDpiX()),
            MAP_LOGHIM_TO_PIX(bmsize.cy, pixmap.logicalDpiY()));
        
        pixmap.resize(bmsize.cx, bmsize.cy);
        if (!pixmap.isNull()) {
            HDC hdc = ::CreateCompatibleDC(pixmap.handle());
            ::SelectObject(hdc, hbm);
            BOOL res = ::BitBlt(pixmap.handle(), 0, 0, pixmap.width(), pixmap.height(), hdc, 0, 0, SRCCOPY);
            ::DeleteObject(hdc);
        }
        
        icon = pixmap;
    }
#endif
    return -1;
}

QMenu *QAxClientSite::generatePopup(HMENU subMenu, QWidget *parent)
{
    QMenu *popup = 0;
    int count = GetMenuItemCount(subMenu);
    if (count)
	popup = new QMenu(parent);
    for (int i = 0; i < count; ++i) {
	MENUITEMINFOA item;
	memset(&item, 0, sizeof(MENUITEMINFOA));
	item.cbSize = sizeof(MENUITEMINFOA);
	item.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
	::GetMenuItemInfoA(subMenu, i, true, &item);

	QAction *action = 0;
	QMenu *popupMenu = 0;
	if (item.fType == MFT_SEPARATOR) {
	    action = popup->addSeparator();
	} else {
	    QString text;
	    QPixmap icon;
	    QKeySequence accel;
	    popupMenu = item.hSubMenu ? generatePopup(item.hSubMenu, popup) : 0;
	    int res = menuItemEntry(subMenu, i, item, text, icon);

	    int lastSep = text.lastIndexOf(QRegExp("[\\s]"));
	    if (lastSep != -1) {
		QString keyString = text.right(text.length() - lastSep);
		accel = keyString;
		if ((int)accel)
		    text = text.left(lastSep);
	    }

            if (popupMenu)
                popupMenu->setTitle(text);

            switch (res) {
	    case MFT_STRING:
                if (popupMenu)
                    action = popup->addMenu(popupMenu);
                else
		    action = popup->addAction(text);
		break;
	    case MFT_BITMAP:
		if (popupMenu)
		    action = popup->addMenu(popupMenu);
		else
		    action = popup->addAction(icon, text);
		break;
	    }

            if (action) {
                if (int(accel))
                    action->setShortcut(accel);
                if (!icon.isNull())
                    action->setIcon(icon);
            }
	}

	if (action) {
	    OleMenuItem oleItem(subMenu, item.wID, popupMenu);
	    menuItemMap.insert(action, oleItem);
	}
    }
    return popup;
}

HRESULT WINAPI QAxClientSite::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    AX_DEBUG(QAxClientSite::SetMenu);

    if (hmenuShared) {
	m_menuOwner = hwndActiveObject;
	QMenuBar *mb = menuBar;
	if (!mb)
	    mb = qFindChild<QMenuBar*>(widget->window());
	if (!mb)
	    return E_NOTIMPL;
	menuBar = mb;

	int count = GetMenuItemCount(hmenuShared);
	for (int i = 0; i < count; ++i) {
	    MENUITEMINFOA item;
	    memset(&item, 0, sizeof(MENUITEMINFOA));
	    item.cbSize = sizeof(MENUITEMINFOA);
	    item.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
	    ::GetMenuItemInfoA(hmenuShared, i, true, &item);

	    QAction *action = 0;
	    QMenu *popupMenu = 0;
	    if (item.fType == MFT_SEPARATOR) {
		action = menuBar->addSeparator();
	    } else {
		QString text;
		QPixmap icon;
                popupMenu = item.hSubMenu ? generatePopup(item.hSubMenu, menuBar) : 0;
                int res = menuItemEntry(hmenuShared, i, item, text, icon);

                if (popupMenu)
                    popupMenu->setTitle(text);

		switch(res) {
		case MFT_STRING:
                    if (popupMenu)
			action = menuBar->addMenu(popupMenu);
                    else
			action = menuBar->addAction(text);
		    break;
		case MFT_BITMAP:
                    if (popupMenu)
			action = menuBar->addMenu(popupMenu);
                    else
			action = menuBar->addAction(text);
		    break;
		default:
		    break;
		}
                if (action && !icon.isNull())
                    action->setIcon(icon);
	    }

	    if (action) {
		OleMenuItem oleItem(hmenuShared, item.wID, popupMenu);
		menuItemMap.insert(action, oleItem);
	    }
	}
	if (count) {
	    const QMetaObject *mbmo = menuBar->metaObject();
	    int index = mbmo->indexOfSignal("triggered(QAction*)");
	    Q_ASSERT(index != -1);
	    menuBar->disconnect(SIGNAL(triggered(QAction*)), host);
            QMetaObject::connect(menuBar, index, host, index, QSIGNAL_CODE);
	}
    } else if (menuBar) {
	m_menuOwner = 0;
	QMap<QAction*, OleMenuItem>::Iterator it;
	for (it = menuItemMap.begin(); it != menuItemMap.end(); ++it) {
	    QAction *action = it.key();
            delete action;
	}
	menuItemMap.clear();
    }

    OleSetMenuDescriptor(holemenu, widget->window()->winId(), m_menuOwner, this, m_spInPlaceActiveObject);
    return S_OK;
}

int QAxClientSite::qt_metacall(QMetaObject::Call call, int isignal, void **argv)
{
    if (!m_spOleObject || call != QMetaObject::InvokeMetaMember || !menuBar)
        return isignal;

    if (isignal != menuBar->metaObject()->indexOfSignal("triggered(QAction*)"))
        return isignal;

    QAction *action = *(QAction**)argv[1];
    // ###

    OleMenuItem oleItem = menuItemMap.value(action);
    if (oleItem.hMenu)
        ::PostMessageA(m_menuOwner, WM_COMMAND, oleItem.id, 0);
    return -1;
}

HRESULT WINAPI QAxClientSite::RemoveMenus(HMENU /*hmenuShared*/)
{
    AX_DEBUG(QAxClientSite::RemoveMenus);
    QMap<QAction*, OleMenuItem>::Iterator it;
    for (it = menuItemMap.begin(); it != menuItemMap.end(); ++it) {
	QAction *action = it.key();
        action->setVisible(false);
        delete action;
    }
    menuItemMap.clear();
    return S_OK;
}

HRESULT WINAPI QAxClientSite::SetStatusText(LPCOLESTR pszStatusText)
{
    QStatusTipEvent tip(QString::fromUtf16((BSTR)pszStatusText));
    QApplication::sendEvent(widget, &tip);
    return S_OK;
}

extern Q_GUI_EXPORT bool qt_win_ignoreNextMouseReleaseEvent;

HRESULT WINAPI QAxClientSite::EnableModeless(BOOL fEnable)
{
    EnableWindow(host->winId(), fEnable);
    if (!fEnable)
        QApplicationPrivate::enterModal(host);
    else 
        QApplicationPrivate::leaveModal(host);
    qt_win_ignoreNextMouseReleaseEvent = false;

    return S_OK;
}

HRESULT WINAPI QAxClientSite::TranslateAccelerator(LPMSG lpMsg, WORD grfModifiers)
{
    return TranslateAccelerator(lpMsg, (DWORD)grfModifiers);
}

//**** IOleInPlaceUIWindow
HRESULT WINAPI QAxClientSite::GetBorder(LPRECT lprectBorder)
{
    AX_DEBUG(QAxClientSite::GetBorder);

    QMainWindow *mw = qobject_cast<QMainWindow*>(widget->window());
    if (!mw)
        return INPLACE_E_NOTOOLSPACE;

    RECT border = { 0,0, 300, 200 };
    *lprectBorder = border;
    return S_OK;
}

HRESULT WINAPI QAxClientSite::RequestBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/)
{
    AX_DEBUG(QAxClientSite::RequestBorderSpace);

    QMainWindow *mw = qobject_cast<QMainWindow*>(widget->window());
    if (!mw)
        return INPLACE_E_NOTOOLSPACE;

    return S_OK;
}

HRESULT WINAPI QAxClientSite::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    AX_DEBUG(QAxClientSite::SetBorderSpace);

    // object has no toolbars and wants container toolbars to remain
    if (!pborderwidths)
        return S_OK;

    QMainWindow *mw = qobject_cast<QMainWindow*>(widget->window());
    if (!mw)
        return OLE_E_INVALIDRECT;

    bool removeToolBars = !(pborderwidths->left || pborderwidths->top || pborderwidths->right || pborderwidths->bottom);

    // object has toolbars, and wants container to remove toolbars
    if (removeToolBars) {
        if (mw) {
            //### remove our toolbars
        }
    }

    if (pborderwidths->left) {
        QDockWidget *left = new QDockWidget(mw);
        left->setFixedWidth(pborderwidths->left);
        mw->addDockWidget(Qt::LeftDockWidgetArea, left);
        left->show();
    }
    if (pborderwidths->top) {
        QDockWidget *top = new QDockWidget(mw);
        top->setFixedHeight(pborderwidths->top);
        mw->addDockWidget(Qt::TopDockWidgetArea, top);
        top->show();
    }

    return S_OK;
}

HRESULT WINAPI QAxClientSite::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
    AX_DEBUG(QAxClientSite::SetActiveObject);

    if (pszObjName && widget)
        widget->setWindowTitle(QString::fromUtf16((BSTR)pszObjName));
    
    if (m_spInPlaceActiveObject)
        m_spInPlaceActiveObject->Release();
    
    m_spInPlaceActiveObject = pActiveObject;
    if (m_spInPlaceActiveObject)
        m_spInPlaceActiveObject->AddRef();
    
    return S_OK;
}

//**** IOleDocumentSite
HRESULT WINAPI QAxClientSite::ActivateMe(IOleDocumentView *pViewToActivate)
{
    AX_DEBUG(QAxClientSite::ActivateMe);

    if (m_spActiveView)
        m_spActiveView->Release();
    m_spActiveView = 0;

    if (!pViewToActivate) {
        IOleDocument *document = 0;
        m_spOleObject->QueryInterface(IID_IOleDocument, (void**)&document);
        if (!document)
            return E_FAIL;

        document->CreateView(this, 0, 0, &pViewToActivate);

        document->Release();
        if (!pViewToActivate)
            return E_OUTOFMEMORY;
    } else {
        pViewToActivate->SetInPlaceSite(this);
    }

    m_spActiveView = pViewToActivate;
    m_spActiveView->AddRef();

    m_spActiveView->UIActivate(TRUE);

    RECT rect;
    GetClientRect(widget->winId(), &rect);
    m_spActiveView->SetRect(&rect);
    m_spActiveView->Show(TRUE);

    return S_OK;
}

QSize QAxClientSite::minimumSizeHint() const
{
    if (!m_spOleObject)
        return QSize();
    
    SIZE sz = { 0, 0 };
    m_spOleObject->SetExtent(DVASPECT_CONTENT, &sz);
    HRESULT res = m_spOleObject->GetExtent(DVASPECT_CONTENT, &sz);
    if (SUCCEEDED(res)) {
        return QSize(MAP_LOGHIM_TO_PIX(sz.cx, widget->logicalDpiX()),
            MAP_LOGHIM_TO_PIX(sz.cy, widget->logicalDpiY()));
    }
    return QSize();
}

void QAxClientSite::windowActivationChange()
{
    AX_DEBUG(QAxClientSite::windowActivationChange);

    if (m_spInPlaceActiveObject && widget) {
        QWidget *modal = QApplication::activeModalWidget();
        if (modal && inPlaceModelessEnabled) {
            m_spInPlaceActiveObject->EnableModeless(false);
            inPlaceModelessEnabled = false;
        } else if (!inPlaceModelessEnabled) {
            m_spInPlaceActiveObject->EnableModeless(true);
            inPlaceModelessEnabled = true;
        }
        m_spInPlaceActiveObject->OnFrameWindowActivate(widget->isActiveWindow());
    }
}


//**** QWidget

QAxHostWidget::QAxHostWidget(QWidget *parent, QAxClientSite *ax)
: QWidget(parent), setFocusTimer(0), hasFocus(false), axhost(ax)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setObjectName("QAxHostWidget");
}

QAxHostWidget::~QAxHostWidget()
{
    if (axhost)
        axhost->reset(this);
}


int QAxHostWidget::qt_metacall(QMetaObject::Call call, int isignal, void **argv)
{
    if (axhost)
        return axhost->qt_metacall(call, isignal, argv);
    return -1;
}

QSize QAxHostWidget::sizeHint() const
{
    return axhost ? axhost->sizeHint() : QWidget::sizeHint();
}

QSize QAxHostWidget::minimumSizeHint() const
{
    QSize size;
    if (axhost)
        size = axhost->minimumSizeHint();
    if (size.isValid())
        return size;
    return QWidget::minimumSizeHint();
}

void QAxHostWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    if (!axhost)
        return;
    
    // document server - talk to view?
    if (axhost->m_spActiveView) {
        RECT rect;
        GetClientRect(winId(), &rect);
        axhost->m_spActiveView->SetRect(&rect);

        return;
    }

    SIZEL hmSize;
    hmSize.cx = MAP_PIX_TO_LOGHIM(width(), logicalDpiX());
    hmSize.cy = MAP_PIX_TO_LOGHIM(height(), logicalDpiY());
    
    if (axhost->m_spOleObject)
        axhost->m_spOleObject->SetExtent(DVASPECT_CONTENT, &hmSize);
    if (axhost->m_spInPlaceObject) {
        RECT rcPos = { x(), y(), x()+width(), y()+height() };
        axhost->m_spInPlaceObject->SetObjectRects(&rcPos, &rcPos);
    }
}

bool QAxHostWidget::winEvent(MSG *msg, long *result)
{
    if (axhost && axhost->inPlaceObjectWindowless) {
        Q_ASSERT(axhost->m_spInPlaceObject);
        IOleInPlaceObjectWindowless *windowless = (IOleInPlaceObjectWindowless*)axhost->m_spInPlaceObject;
        Q_ASSERT(windowless);
        LRESULT lres;
        HRESULT hres = windowless->OnWindowMessage(msg->message, msg->wParam, msg->lParam, &lres);
        if (hres == S_OK)
            return true;
    }
    return false; // ###
}

bool QAxHostWidget::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Timer:
        if (axhost && ((QTimerEvent*)e)->timerId() == setFocusTimer) {
            killTimer(setFocusTimer);
            setFocusTimer = 0;
            RECT rcPos = { x(), y(), x()+size().width(), y()+size().height() };
            axhost->m_spOleObject->DoVerb(OLEIVERB_UIACTIVATE, 0, (IOleClientSite*)axhost, 0, winId(), &rcPos);
            if (axhost->m_spActiveView)
                axhost->m_spActiveView->UIActivate(TRUE);
        }
        break;
    case QEvent::WindowBlocked:
        if (IsWindowEnabled(winId())) {
            EnableWindow(winId(), false);
            if (axhost && axhost->m_spInPlaceActiveObject)
                axhost->m_spInPlaceActiveObject->EnableModeless(false);
        }
        break;
    case QEvent::WindowUnblocked:
        if (!IsWindowEnabled(winId())) {
            EnableWindow(winId(), true);
            if (axhost && axhost->m_spInPlaceActiveObject)
                axhost->m_spInPlaceActiveObject->EnableModeless(true);
        }
        break;
    default:
        break;
    }
    
    return QWidget::event(e);
}

bool QAxHostWidget::eventFilter(QObject *o, QEvent *e)
{
    // focus goes to Qt while ActiveX still has it - deactivate
    QWidget *newFocus = qobject_cast<QWidget*>(o);
    if (e->type() == QEvent::FocusIn && hasFocus 
        && newFocus && newFocus->window() == window()) {
        if (axhost && axhost->m_spInPlaceActiveObject && axhost->m_spInPlaceObject)
            axhost->m_spInPlaceObject->UIDeactivate();
        qApp->removeEventFilter(this);
    }

    return QWidget::eventFilter(o, e);
}

void QAxHostWidget::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);

    if (!axhost || !axhost->m_spOleObject)
        return;

    // this is called by QWidget::setFocus which calls ::SetFocus on "this",
    // so we have to UIActivate the control after all that had happend.
    AX_DEBUG(Setting focus on in-place object);
    setFocusTimer = startTimer(0);
}

void QAxHostWidget::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
    if (setFocusTimer) {
        killTimer(setFocusTimer);
        setFocusTimer = 0;
    }
    if (e->reason() == Qt::PopupFocusReason || e->reason() == Qt::MenuBarFocusReason)
        return;
    
    if (!axhost || !axhost->m_spInPlaceActiveObject || !axhost->m_spInPlaceObject)
        return;
    
    AX_DEBUG(Deactivating in-place object);
    axhost->m_spInPlaceObject->UIDeactivate();
}


void QAxHostWidget::paintEvent(QPaintEvent*)
{
    if (!QPainter::redirected(this))
        return;
    
    IViewObject *view = 0;
    if (axhost)
        axhost->widget->queryInterface(IID_IViewObject, (void**)&view);
    if (!view)
        return;
    
    // somebody tries to grab us!
    QPixmap pm(size());
    pm.fill();
    HDC hdc = pm.getDC();
    
    RECTL bounds;
    bounds.left = 0;
    bounds.right = pm.width();
    bounds.top = 0;
    bounds.bottom = pm.height();
    view->Draw(DVASPECT_CONTENT, -1, 0, 0, 0, hdc, &bounds, 0, 0 /*fptr*/, 0);
    view->Release();
    pm.releaseDC(hdc);
    
    QPainter painter(this);
    painter.drawPixmap(0, 0, pm);
}


/*!
    \class QAxWidget qaxwidget.h
    \brief The QAxWidget class is a QWidget that wraps an ActiveX control.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \extension ActiveQt
    \module QAxContainer

    A QAxWidget can be instantiated as an empty object, with the name
    of the ActiveX control it should wrap, or with an existing
    interface pointer to the ActiveX control. The ActiveX control's
    properties, methods and events which only use \link QAxBase
    supported data types\endlink, become available as Qt properties,
    slots and signals. The base class QAxBase provides an API to
    access the ActiveX directly through the IUnknown pointer.

    QAxWidget is a QWidget and can be used as such, e.g. it can be
    organized in a widget hierarchy, receive events or act as an event
    filter. Standard widget properties, e.g. \link QWidget::enabled
    enabled \endlink are supported, but it depends on the ActiveX
    control to implement support for ambient properties like e.g.
    palette or font. QAxWidget tries to provide the necessary hints.

    \warning
    You can subclass QAxWidget, but you cannot use the Q_OBJECT macro
    in the subclass (the generated moc-file will not compile), so you
    cannot add further signals, slots or properties. This limitation
    is due to the metaobject information generated in runtime. To work
    around this problem, aggregate the QAxWidget as a member of the
    QObject subclass.

    \important dynamicCall() querySubObject()
*/

/*!
    Creates an empty QAxWidget widget and propagates \a parent, \a
    name and \a f to the QWidget constructor. To initialize a control,
    call \link QAxBase::setControl() setControl \endlink.
*/
QAxWidget::QAxWidget(QWidget *parent, Qt::WFlags f)
: QWidget(parent, f), container(0)
{
}

/*!
    Creates an QAxWidget widget and initializes the ActiveX control \a c.
    \a parent, \a name and \a f are propagated to the QWidget contructor.

    \sa setControl()
*/
QAxWidget::QAxWidget(const QString &c, QWidget *parent, Qt::WFlags f)
: QWidget(parent, f), container(0)
{
    setControl(c);
}

/*!
    Creates a QAxWidget that wraps the COM object referenced by \a iface.
    \a parent, \a name and \a f are propagated to the QWidget contructor.
*/
QAxWidget::QAxWidget(IUnknown *iface, QWidget *parent, Qt::WFlags f)
: QWidget(parent, f), QAxBase(iface), container(0)
{
}

/*!
    Shuts down the ActiveX control and destroys the QAxWidget widget,
    cleaning up all allocated resources.

    \sa clear()
*/
QAxWidget::~QAxWidget()
{
    if (container)
        container->reset(this);
    clear();
}

/*!
    \reimp
*/
bool QAxWidget::initialize(IUnknown **ptr)
{
    if (!QAxBase::initialize(ptr))
        return false;

    HRESULT hres = S_OK; // assume that control is not initialized
    return createHostWindow(hres == S_FALSE);
}

/*!
    Creates the client site for the ActiveX control, and returns true if
    the control could be embedded successfully, otherwise returns false.
    If \a initialized is true the control has already been initialized.

    This function is called by initialize(). If you reimplement initialize
    to customize the actual control instantiation, call this function in your
    reimplementation to have the control embedded by the default client side.
    Creates the client site for the ActiveX control, and returns true if
    the control could be embedded successfully, otherwise returns false.
*/
bool QAxWidget::createHostWindow(bool initialized)
{
#ifdef QT_COMPAT
    QApplication::sendPostedEvents(0, QEvent::ChildInserted);
#endif

    container = new QAxClientSite(this);
    container->activateObject(initialized);

    if (!filter_ref)
        previous_filter = QAbstractEventDispatcher::instance()->setEventFilter(axc_FilterProc);
    ++filter_ref;

    if (parentWidget())
        QApplication::postEvent(parentWidget(), new QEvent(QEvent::LayoutRequest));

    return true;
}

/*!
    Reimplement this function when you want to implement additional
    COM interfaces for the client site of the ActiveX control, or when 
    you want to provide alternative implementations of COM interfaces.
    Return a new object of a QAxAggregated subclass.

    The default implementation returns the null pointer.
*/
QAxAggregated *QAxWidget::createAggregate()
{
    return 0;
}

/*!
    \reimp

    Shuts down the ActiveX control.
*/
void QAxWidget::clear()
{
    if (isNull())
        return;
    if (!control().isEmpty()) {
        if (filter_ref) {
            if (!--filter_ref) {
                QAbstractEventDispatcher::instance()->setEventFilter(previous_filter);
                previous_filter = 0;
            }
        }
    }
    
    if (container)
        container->deactivate();
    
    QAxBase::clear();
    setFocusPolicy(Qt::NoFocus);
    
    if (container) {
        container->releaseAll();
        container->Release();
    }
    container = 0;
}

/*!
    \fn QObject *QAxWidget::qObject()
    \reimp
*/

/*!
    \reimp
*/
const QMetaObject *QAxWidget::metaObject() const
{
    return QAxBase::metaObject();
}

/*!
    \reimp
*/
const QMetaObject *QAxWidget::parentMetaObject() const
{
    return &QWidget::staticMetaObject;
}

/*!
    \reimp
*/
void *QAxWidget::qt_metacast(const char *cname)
{
    if (!qstrcmp(cname, "QAxWidget")) return (void*)this;
    if (!qstrcmp(cname, "QAxBase")) return (QAxBase*)this;
    return QWidget::qt_metacast(cname);
}

/*!
    \reimp
*/
const char *QAxWidget::className() const
{
    return "QAxWidget";
}

/*!
    \reimp
*/
int QAxWidget::qt_metacall(QMetaObject::Call call, int id, void **v)
{
    id = QWidget::qt_metacall(call, id, v);
    if (id < 0)
        return id;
    return QAxBase::qt_metacall(call, id, v);
}

/*!
    \reimp
*/
QSize QAxWidget::sizeHint() const
{
    if (container) {
        QSize sh = container->sizeHint();
        if (sh.isValid())
            return sh;
    }
    
    return QWidget::sizeHint();
}

/*!
    \reimp
*/
QSize QAxWidget::minimumSizeHint() const
{
    if (container) {
        QSize sh = container->minimumSizeHint();
        if (sh.isValid())
            return sh;
    }
    
    return QWidget::minimumSizeHint();
}

/*!
    \reimp
*/
void QAxWidget::changeEvent(QEvent *e)
{
    if (isNull())
        return;

    switch (e->type()) {
    case QEvent::EnabledChange:
        if (!isEnabled())
            container->emitAmbientPropertyChange(DISPID_AMBIENT_UIDEAD);
        break;
    case QEvent::FontChange:
        container->emitAmbientPropertyChange(DISPID_AMBIENT_FONT);
        break;
    case QEvent::PaletteChange:
        container->emitAmbientPropertyChange(DISPID_AMBIENT_BACKCOLOR);
        container->emitAmbientPropertyChange(DISPID_AMBIENT_FORECOLOR);
        break;
    case QEvent::ActivationChange:
        container->windowActivationChange();
        break;
    default:
        break;
    }
}

/*!
    \reimp
*/
void QAxWidget::resizeEvent(QResizeEvent *e)
{
    if (container)
        container->resize(size());
    
    QWidget::resizeEvent(e);
}

/*!
    \reimp
*/
void QAxWidget::connectNotify(const char *)
{
    QAxBase::connectNotify();
}


/*!
    Reimplement this function to pass certain key events to the
    ActiveX control. \a message is the Window message identifier
    specifying the message type (ie. WM_KEYDOWN), and \a keycode is
    the virtual keycode (ie. VK_TAB).
    
    If the function returns true the key event is passed on to the 
    ActiveX control, which then either processes the event or passes 
    the event on to Qt.

    If the function returns false the processing of the key event is
    ignored by ActiveQt, ie. the ActiveX control might handle it or
    not.

    The default implementation returns true for the following cases:

    \table
    \header
    \i WM_SYSKEYDOWN
    \i WM_SYSKEYUP
    \i WM_KEYDOWN
    \row
    \i All keycodes
    \i VK_MENU
    \i VK_TAB, VK_DELETE and all non-arrow-keys in combination with VK_SHIFT, 
       VK_CONTROL or VK_MENU
    \endtable

    This table is the result of experimenting with popular ActiveX controls,
    ie. Internet Explorer and Microsoft Office applications, but for some
    controls it might require modification.
*/
bool QAxWidget::translateKeyEvent(int message, int keycode) const
{
    bool translate = false;
    
    switch (message) {
    case WM_SYSKEYDOWN:
        translate = true;
        break;
    case WM_KEYDOWN:
        translate = keycode == VK_TAB
            || keycode == VK_DELETE;
        if (!translate) {
            int state = 0;
            if (GetKeyState(VK_SHIFT) < 0)
                state |= 0x01;
            if (GetKeyState(VK_CONTROL) < 0)
                state |= 0x02;
            if (GetKeyState(VK_MENU) < 0)
                state |= 0x04;
            if (state) {
                state = keycode < VK_LEFT || keycode > VK_DOWN;
            }
            translate = state;
        }
        break;
    case WM_SYSKEYUP:
        translate = keycode == VK_MENU;
        break;
    }
    
    return translate;
}
