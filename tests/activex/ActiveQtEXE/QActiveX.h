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
	public IConnectionPointContainerImpl<QActiveX>,
	public IPersistStorageImpl<QActiveX>,
	public ISpecifyPropertyPagesImpl<QActiveX>,
	public IQuickActivateImpl<QActiveX>,
	public IDataObjectImpl<QActiveX>,
	public IProvideClassInfo2Impl<&CLSID_QActiveX, &DIID__IQActiveXEvents, &LIBID_ACTIVEQTEXELib>,
	public IPropertyNotifySinkCP<QActiveX>,
	public CComCoClass<QActiveX, &CLSID_QActiveX>
{
public:
	QActiveX()
	{
		m_bWindowOnly = true;
		m_pWidget = NULL;
	}
	virtual ~QActiveX()
	{
		if ( m_pWidget )
		{
			delete m_pWidget;
		}
	}

DECLARE_REGISTRY_RESOURCEID(IDR_QEXETEST)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(QActiveX)
	COM_INTERFACE_ENTRY(IQActiveX)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
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

BEGIN_CONNECTION_POINT_MAP(QActiveX)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(QActiveX)
	CHAIN_MSG_MAP(CComControl<QActiveX>)
	DEFAULT_REFLECTION_HANDLER()
	MESSAGE_HANDLER(WM_MOVE, OnMove)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// IViewObjectEx
	DECLARE_VIEW_STATUS( 0 )

// IQActiveX
public:
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
private:
	CTestWidget* m_pWidget;
};

#endif
