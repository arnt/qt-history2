// TestControl.h : Declaration of the CTestControl

#ifndef __TESTCONTROL_H_
#define __TESTCONTROL_H_

#include "resource.h"       // main symbols
#include <atlctl.h>

#if _ATL_VER < 0x0300
#define BEGIN_PROP_MAP	BEGIN_PROPERTY_MAP
#define END_PROP_MAP	END_PROPERTY_MAP
#define PROP_DATA_ENTRY	PROP_ENTRY_EX
#endif


/////////////////////////////////////////////////////////////////////////////
// CTestControl
class ATL_NO_VTABLE CTestControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CStockPropImpl<CTestControl, ITestControl, &IID_ITestControl, &LIBID_ATLTESTLib>,
	public CComControl<CTestControl>,
	public IPersistStreamInitImpl<CTestControl>,
	public IOleControlImpl<CTestControl>,
	public IOleObjectImpl<CTestControl>,
	public IOleInPlaceActiveObjectImpl<CTestControl>,
	public IViewObjectExImpl<CTestControl>,
	public IOleInPlaceObjectWindowlessImpl<CTestControl>,
	public IPersistStorageImpl<CTestControl>,
	public ISpecifyPropertyPagesImpl<CTestControl>,
	public IQuickActivateImpl<CTestControl>,
	public IDataObjectImpl<CTestControl>,
	public IProvideClassInfo2Impl<&CLSID_TestControl, NULL, &LIBID_ATLTESTLib>,
	public CComCoClass<CTestControl, &CLSID_TestControl>
{
public:
	CTestControl()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TESTCONTROL)

#if _ATL_VER >= 0x0300
DECLARE_PROTECT_FINAL_CONSTRUCT()
#endif

BEGIN_COM_MAP(CTestControl)
	COM_INTERFACE_ENTRY(ITestControl)
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
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(CTestControl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	PROP_ENTRY("AutoSize", DISPID_AUTOSIZE, CLSID_NULL)
	PROP_ENTRY("BackColor", DISPID_BACKCOLOR, CLSID_StockColorPage)
	PROP_ENTRY("Caption", DISPID_CAPTION, CLSID_NULL)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(CTestControl)
#if _ATL_VER < 0x0300
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
#else
	CHAIN_MSG_MAP(CComControl<CTestControl>)
	DEFAULT_REFLECTION_HANDLER()
#endif
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);



// IViewObjectEx
#if _ATL_VER < 0x0300
	STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
	{
		ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
		*pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
		return S_OK;
	}
#else
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)
#endif

// ITestControl
public:

	HRESULT OnDraw(ATL_DRAWINFO& di);
	OLE_COLOR m_clrBackColor;
	CComBSTR m_bstrCaption;
};

#endif //__TESTCONTROL_H_
