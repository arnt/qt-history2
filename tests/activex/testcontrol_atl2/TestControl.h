// TestControl.h : Declaration of the CTestControl

#ifndef __TESTCONTROL_H_
#define __TESTCONTROL_H_

#include "resource.h"       // main symbols


/////////////////////////////////////////////////////////////////////////////
// CTestControl
class ATL_NO_VTABLE CTestControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTestControl, &CLSID_TestControl>,
	public CComControl<CTestControl>,
	public CStockPropImpl<CTestControl, ITestControl, &IID_ITestControl, &LIBID_ATLTESTLib>,
	public IProvideClassInfo2Impl<&CLSID_TestControl, NULL, &LIBID_ATLTESTLib>,
	public IPersistStreamInitImpl<CTestControl>,
	public IPersistStorageImpl<CTestControl>,
	public IQuickActivateImpl<CTestControl>,
	public IOleControlImpl<CTestControl>,
	public IOleObjectImpl<CTestControl>,
	public IOleInPlaceActiveObjectImpl<CTestControl>,
	public IViewObjectExImpl<CTestControl>,
	public IOleInPlaceObjectWindowlessImpl<CTestControl>,
	public IDataObjectImpl<CTestControl>,
	public ISpecifyPropertyPagesImpl<CTestControl>
{
public:
	CTestControl()
	{
 
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TESTCONTROL)

BEGIN_COM_MAP(CTestControl)
	COM_INTERFACE_ENTRY(ITestControl)
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

BEGIN_PROPERTY_MAP(CTestControl)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	PROP_PAGE(CLSID_StockColorPage)
END_PROPERTY_MAP()


BEGIN_MSG_MAP(CTestControl)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()


// IViewObjectEx
	STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
	{
		ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
		*pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
		return S_OK;
	}

// ITestControl
public:
	HRESULT OnDraw(ATL_DRAWINFO& di);

	OLE_COLOR m_clrBackColor;
	CComBSTR m_bstrCaption;
};

#endif //__TESTCONTROL_H_
