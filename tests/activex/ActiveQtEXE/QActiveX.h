// QActiveX.h : Declaration of the CQActiveX

#ifndef __QACTIVEX_H_
#define __QACTIVEX_H_

#include "resource.h"       // main symbols
#include <atlctl.h>
#include "TestWidget.h"

/////////////////////////////////////////////////////////////////////////////
// QActiveX
class ATL_NO_VTABLE QActiveX : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IQActiveX, &IID_IQActiveX, &LIBID_ACTIVEQTEXELib>,
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
	public CComCoClass<QActiveX, &CLSID_QActiveX>,
	public IObjectSafety
{
public:
	QActiveX()
	{
		m_bWindowOnly = true;
		curr_ = this;
		debug("QActiveX::QActiveX");
		m_pWidget = new CTestWidget(this);
		debug("QActiveX::QActiveX - inited");
	}
	virtual ~QActiveX()
	{
	    debug("QActiveX::~QActiveX");
		curr_ = 0;
		if ( m_pWidget )
		{
			delete m_pWidget;
		}
	}
	static QActiveX* currentControl()
	{
		return curr_;
	}
	static CTestWidget* currentWidget()
	{
		if (curr_)
		return curr_->m_pWidget;
		return 0;
	}
	IUnknown* GetControllingUnknown( )
	{
		void* iu = 0;
		HRESULT hr = QueryInterface( IID_IUnknown, &iu);
		IUnknown* ip = (IUnknown *)iu;
		ip->Release();
		return ip;
	}
	bool containerIsBrowser() const
	{
		static int value = -1;
		if (value < 0) {
		// This impl. is ad-hoc based on names of known browsers
		QString cmd(GetCommandLine());
		if (cmd.contains("iexplore", FALSE) > 0)
			value = 1;
		else if (cmd.contains("netscape", FALSE) > 0)
			value = 1;
		else if (cmd.contains("opera", FALSE) > 0)
			value = 1;
		else
			value = 0;
		}
		return (value > 0);
	}

DECLARE_REGISTRY_RESOURCEID(IDR_QEXETEST)

DECLARE_PROTECT_FINAL_CONSTRUCT()

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
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

BEGIN_PROP_MAP(QActiveX)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(QActiveX)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
//	MESSAGE_HANDLER(
	//
	CHAIN_MSG_MAP(CComControl<QActiveX>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

	STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
	{
		debug("QActiveX::GetViewStatus");
		*pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
		return S_OK;
	}

//
// IObjectSafety
//
	STDMETHOD(GetInterfaceSafetyOptions)( REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions )
	{
		*pdwSupportedOptions =
		INTERFACESAFE_FOR_UNTRUSTED_CALLER |
		INTERFACESAFE_FOR_UNTRUSTED_DATA;
		*pdwEnabledOptions = *pdwSupportedOptions;
		return S_OK;
	}
	STDMETHOD(SetInterfaceSafetyOptions)( REFIID riid, DWORD dwOptionsSetMask, DWORD dwEnabledOptions )
	{
		return S_OK;
	}
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		debug("QActiveX::OnDraw");
		m_pWidget->drawControlAtl( &di );
		return S_OK;
	}
	LRESULT OnCreate( UINT, WPARAM, LPARAM, BOOL & )
	{
		debug("QActiveX::OnCreate");
//		metiswin_->startEditor();	// METIS specifics?
		debug("QActiveX::OnCreate - before setActive");
		m_pWidget->setActive( TRUE, m_hWnd );
		debug("QActiveX::OnCreate - end");
		return 0;
	}
	LRESULT OnDestroy( UINT, WPARAM, LPARAM, BOOL & )
	{
		debug("QActiveX::OnDestroy");
		m_pWidget->setActive( FALSE, m_hWnd );
//		SendOnDataChange();
//		FireViewChange();
		return 0;
	}
	STDMETHOD(OnDocWindowActivate)( BOOL activate )
	{
		debug("QActiveX::OnDocWindowActivate");
		return S_OK;
	}
	STDMETHOD(OnFrameWindowActivate)( BOOL activate )
	{
		debug("QActiveX::OnFrameWindowActivate");
		return S_OK;
	}
	STDMETHOD(SetObjectRects)(LPCRECT prcPos, LPCRECT prcClip)
	{
		debug("QActiveX::SetObjectRects");
		IOleInPlaceObjectWindowlessImpl < QActiveX>::SetObjectRects(prcPos, prcClip);
		m_pWidget->setGeometry( 0, 0, prcPos->right - prcPos->left,	prcPos->bottom - prcPos->top );
		return S_OK;
	}


// IQActiveX
public:
/*
** Only comment out these functions, don't delete them, as we may want to
** bring them back some day
	LRESULT OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if ( m_pWidget && m_pWidget->m_bWidgetReady )
		{
			m_pWidget->move( LOWORD( lParam ), HIWORD( lParam ) );
		}
		return 0;
	}
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int w,h;
		if ( m_pWidget && m_pWidget->m_bWidgetReady )
		{
			w = LOWORD( lParam );
			h = HIWORD( lParam );
			m_pWidget->resize( LOWORD( lParam ), HIWORD( lParam ) );
		}
		return 0;
	}
	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		return S_OK;
	}
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if ( m_pWidget = new CTestWidget() )
		{
//			ModifyStyle( WS_CHILD, 0, 0 );
//			ModifyStyleEx( 0, WS_EX_CONTROLPARENT );
			m_pWidget->attachToControl( m_hWnd );
			m_pWidget->InitWidget();
		}
		return 0;
	}
*/
private:
	CTestWidget* m_pWidget;
	static QActiveX* curr_;
};

QActiveX* QActiveX::curr_ = 0;
#endif
