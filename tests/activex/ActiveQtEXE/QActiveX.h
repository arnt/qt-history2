// QActiveX.h : Declaration of the CQActiveX

#ifndef __QACTIVEX_H_
#define __QACTIVEX_H_

#include "resource.h"       // main symbols
#include <atlctl.h>
#include <qvbox.h>

Q_EXPORT LRESULT QtWndProcGate( HWND, UINT, WPARAM, LPARAM );

extern "C" QWidget *axmain( QWidget *parent );


class ATL_NO_VTABLE QActiveXTypeInfo : public ITypeInfo
{
public:
    QActiveXTypeInfo( const QObject *qo ) : object( qo ), ref( 0 )
    {}

    // IUnknown
    unsigned long __stdcall AddRef()
    {
	return ref++;
    }
    unsigned long __stdcall Release()
    {
	if ( !--ref ) {
	    delete this;
	    return 0;
	}
	return ref;
    }
    HRESULT __stdcall QueryInterface( REFIID riid, void **ppvObject )
    {
	*ppvObject = 0;
	if ( riid == IID_IUnknown)
	    *ppvObject = (IUnknown*)this;
	else if ( riid == IID_ITypeInfo )
	    *ppvObject = (ITypeInfo*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    // ITypeInfo


    const QObject *qObject() { return object; }
    void setObject( const QObject *o ) { object = o; }

private:
    const QObject *object;
    unsigned long ref;
};

/////////////////////////////////////////////////////////////////////////////
// QActiveX
class ATL_NO_VTABLE QActiveX : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IQActiveX, &IID_IQActiveX, &LIBID_ACTIVEQTEXELib >,
    public CComControl<QActiveX>,
    public IPersistStreamInitImpl<QActiveX>,
    public IOleControlImpl<QActiveX>,
    public IOleObjectImpl<QActiveX>,
    public IOleInPlaceActiveObjectImpl<QActiveX>,
    public IViewObjectExImpl<QActiveX>,
    public IOleInPlaceObjectWindowlessImpl<QActiveX>,
    public IPersistStorageImpl<QActiveX>,
    public ISpecifyPropertyPagesImpl<QActiveX>,
    public IQuickActivateImpl<QActiveX>,
    public IDataObjectImpl<QActiveX>,
    public IProvideClassInfo2Impl<&CLSID_QActiveX, &DIID__IQActiveXEvents, &LIBID_ACTIVEQTEXELib>,
    public CComCoClass<QActiveX, &CLSID_QActiveX>
{
public:
    QActiveX()
	: m_pTypeInfo( 0 )
    {
	m_pWidget = new QVBox( 0, 0, Qt::WStyle_Customize );
	::SetWindowLong( m_pWidget->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
	object = axmain( m_pWidget );
    }

    ~QActiveX()
    {
	if ( m_pTypeInfo ) {
	    m_pTypeInfo->setObject( 0 );
	    m_pTypeInfo->Release();
	    m_pTypeInfo = 0;
	}
	if ( object ) {
	    delete object;
	}
	if ( m_pWidget ) {
	    delete m_pWidget;
	}
    }

DECLARE_REGISTRY_RESOURCEID(IDR_QEXETEST)

BEGIN_COM_MAP(QActiveX)
    COM_INTERFACE_ENTRY(IQActiveX)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY_IMPL(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(QActiveX)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
    // Example entries
    // PROP_ENTRY("Property Description", dispid, clsid)
    // PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(QActiveX)
    CHAIN_MSG_MAP(CComControl<QActiveX>)
    DEFAULT_REFLECTION_HANDLER()
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow )
    MESSAGE_HANDLER(WM_PAINT, OnPaint )
    MESSAGE_HANDLER(WM_SIZE, ForwardMessage )
    MESSAGE_HANDLER(WM_ACTIVATE, ForwardMessage)
    MESSAGE_HANDLER(WM_KEYUP, ForwardMessage)
    MESSAGE_HANDLER(WM_KEYDOWN, ForwardMessage)
    MESSAGE_HANDLER(WM_CHAR, ForwardMessage)
    MESSAGE_HANDLER(WM_SETFOCUS, ForwardFocusMessage )
    MESSAGE_HANDLER(WM_KILLFOCUS, ForwardFocusMessage )
    MESSAGE_HANDLER(WM_ACTIVATE, ForwardMessage )
END_MSG_MAP()

// IViewObjectEx
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)
/*
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
	*pptinfo = 0;
	if ( itinfo )
	    return ResultFromScode( DISP_E_BADINDEX );
	if ( !m_pTypeInfo ) {
	    m_pTypeInfo = new QActiveXTypeInfo( object );
	    m_pTypeInfo->AddRef();
	}
	m_pTypeInfo->AddRef();
	*pptinfo = m_pTypeInfo;
    }
*/
private:
    QVBox* m_pWidget;
    QObject *object;
    QActiveXTypeInfo *m_pTypeInfo;

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
	::SetParent( m_pWidget->winId(), m_hWnd );
	m_pWidget->raise();
	m_pWidget->move( 0, 0 );

	return 0;
    }
    LRESULT OnShowWindow( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
    {
	if( m_pWidget ) {
	    if( wParam )
		m_pWidget->show();
	    else
		m_pWidget->hide();
	}
	return 0;
    }
    LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
    {
	if ( m_pWidget )
	    m_pWidget->update();
	return 0;
    }
    LRESULT ForwardMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
    {
	if( m_pWidget )
	    return ::SendMessage( m_pWidget->winId(), uMsg, wParam, lParam );
	return 0;
    }

    LRESULT ForwardFocusMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
    {
	if( m_pWidget ) {
	    if ( uMsg == WM_SETFOCUS )
		::SendMessage( m_pWidget->winId(), WM_ACTIVATE, MAKEWPARAM( WA_ACTIVE, 0 ), 0 );
	    return ::SendMessage( m_pWidget->winId(), uMsg, wParam, lParam );
	}
	return 0;
    }
};

#endif
