// QtTestControlCtl.cpp : Implementation of the CQtTestControlCtrl ActiveX Control class.

#include "stdafx.h"
#include "QtTestControl.h"
#include "QtTestControlCtl.h"
#include "QtTestControlPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CQtTestControlCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CQtTestControlCtrl, COleControl)
	//{{AFX_MSG_MAP(CQtTestControlCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CQtTestControlCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CQtTestControlCtrl)
	DISP_PROPERTY_NOTIFY(CQtTestControlCtrl, "Double", m_double, OnDoubleChanged, VT_R8)
	DISP_PROPERTY_NOTIFY(CQtTestControlCtrl, "String", m_string, OnStringChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CQtTestControlCtrl, "Date", m_date, OnDateChanged, VT_DATE)
	DISP_PROPERTY_NOTIFY(CQtTestControlCtrl, "ErrorCode", m_errorCode, OnErrorCodeChanged, VT_ERROR)
	DISP_PROPERTY_NOTIFY(CQtTestControlCtrl, "Color", m_color, OnColorChanged, VT_COLOR)
	DISP_PROPERTY_NOTIFY(CQtTestControlCtrl, "Width", m_width, OnWidthChanged, VT_XSIZE_PIXELS)
	DISP_PROPERTY_NOTIFY(CQtTestControlCtrl, "Height", m_height, OnHeightChanged, VT_YSIZE_PIXELS)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Int", GetInt, SetInt, VT_I4)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Short", GetShort, SetShort, VT_I2)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Float", GetFloat, SetFloat, VT_R4)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Currency", GetCurrency, SetCurrency, VT_CY)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Bool", GetBool, SetBool, VT_BOOL)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Variant", GetVariant, SetVariant, VT_VARIANT)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Dispinterface", GetDispinterface, SetDispinterface, VT_DISPATCH)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "UnknownInterface", GetUnknownInterface, SetUnknownInterface, VT_UNKNOWN)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "xpos", GetXpos, SetXpos, VT_XPOS_PIXELS)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "ypos", GetYpos, SetYpos, VT_YPOS_PIXELS)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Handle", GetHandle, SetHandle, VT_HANDLE)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Tristate", GetTristate, SetTristate, VT_TRISTATE)
	DISP_PROPERTY_EX(CQtTestControlCtrl, "Exclusive", GetExclusive, SetExclusive, VT_OPTEXCLUSIVE)
	DISP_FUNCTION(CQtTestControlCtrl, "SetString", SetString, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CQtTestControlCtrl, "Add", Add, VT_I4, VTS_I4 VTS_I4)
	DISP_FUNCTION(CQtTestControlCtrl, "Sub", Sub, VT_EMPTY, VTS_I4 VTS_I4 VTS_PI4)
	DISP_STOCKPROP_FONT()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CQtTestControlCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CQtTestControlCtrl, COleControl)
	//{{AFX_EVENT_MAP(CQtTestControlCtrl)
	EVENT_CUSTOM("Foo", FireFoo, VTS_NONE)
	EVENT_CUSTOM("StringChanged", FireStringChanged, VTS_BSTR)
	EVENT_CUSTOM("GetInt", FireGetInt, VTS_PI4)
	EVENT_STOCK_CLICK()
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CQtTestControlCtrl, 1)
	PROPPAGEID(CQtTestControlPropPage::guid)
END_PROPPAGEIDS(CQtTestControlCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CQtTestControlCtrl, "QTTESTCONTROL.QtTestControlCtrl.1",
	0xf063a297, 0xbe8a, 0x434d, 0xa3, 0x4, 0x8c, 0x33, 0x8, 0xe7, 0xb2, 0x89)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CQtTestControlCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DQtTestControl =
		{ 0xb26ea295, 0x44af, 0x4057, { 0x9e, 0xb7, 0xc6, 0xb1, 0x7a, 0xfb, 0x13, 0xf } };
const IID BASED_CODE IID_DQtTestControlEvents =
		{ 0xc5d7ec42, 0xf0a3, 0x4949, { 0x8e, 0xc6, 0x2b, 0x7f, 0x20, 0x1e, 0xea, 0x13 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwQtTestControlOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CQtTestControlCtrl, IDS_QTTESTCONTROL, _dwQtTestControlOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl::CQtTestControlCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CQtTestControlCtrl

BOOL CQtTestControlCtrl::CQtTestControlCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_QTTESTCONTROL,
			IDB_QTTESTCONTROL,
			afxRegApartmentThreading,
			_dwQtTestControlOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl::CQtTestControlCtrl - Constructor

CQtTestControlCtrl::CQtTestControlCtrl()
{
	InitializeIIDs(&IID_DQtTestControl, &IID_DQtTestControlEvents);

	// TODO: Initialize your control's instance data here.
	m_double = 0.0;
	m_string = _T("");
	m_int = 0;
	m_date = 0;
	m_errorCode = S_OK;
	m_width = 120;
	m_height = 100;
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl::~CQtTestControlCtrl - Destructor

CQtTestControlCtrl::~CQtTestControlCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl::OnDraw - Drawing function

void CQtTestControlCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
	CRect cr = rcBounds;
	pdc->DrawText( m_string, cr, DT_CENTER | DT_VCENTER );
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl::DoPropExchange - Persistence support

void CQtTestControlCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl::OnResetState - Reset control to default state

void CQtTestControlCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl::AboutBox - Display an "About" box to the user

void CQtTestControlCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_QTTESTCONTROL);
	dlgAbout.DoModal();
	FireFoo();
	long l = 0;
	FireGetInt( &l );
	if ( l )
	    dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl message handlers

void CQtTestControlCtrl::OnDoubleChanged() 
{
	FireFoo();

	SetModifiedFlag();
}

long CQtTestControlCtrl::GetInt() 
{
	// TODO: Add your property handler here

	return m_int;
}

void CQtTestControlCtrl::SetInt(long nNewValue) 
{
	// TODO: Add your property handler here
	m_int = nNewValue;
	
	SetModifiedFlag();
}

void CQtTestControlCtrl::OnStringChanged() 
{
	// TODO: Add notification handler code

	Refresh();
	SetModifiedFlag();
}

void CQtTestControlCtrl::SetString(LPCTSTR string) 
{
	// TODO: Add your dispatch handler code here
	m_string = string;
	Refresh();
	FireStringChanged( m_string );
}

void CQtTestControlCtrl::OnDateChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

long CQtTestControlCtrl::Add(long x, long y) 
{
	return x + y;
}

short CQtTestControlCtrl::GetShort() 
{
	// TODO: Add your property handler here

	return 0;
}

void CQtTestControlCtrl::SetShort(short nNewValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

float CQtTestControlCtrl::GetFloat() 
{
	// TODO: Add your property handler here

	return 0.0f;
}

void CQtTestControlCtrl::SetFloat(float newValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

CURRENCY CQtTestControlCtrl::GetCurrency() 
{
	CURRENCY cyResult={0,0};
	// TODO: Add your property handler here

	return cyResult;
}

void CQtTestControlCtrl::SetCurrency(CURRENCY newValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

BOOL CQtTestControlCtrl::GetBool() 
{
	// TODO: Add your property handler here

	return TRUE;
}

void CQtTestControlCtrl::SetBool(BOOL bNewValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

VARIANT CQtTestControlCtrl::GetVariant() 
{
	VARIANT vaResult;
	VariantInit(&vaResult);
	// TODO: Add your property handler here
	vaResult.vt = VT_I4;
	vaResult.lVal = 42;

	return vaResult;
}

void CQtTestControlCtrl::SetVariant(const VARIANT FAR& newValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

void CQtTestControlCtrl::OnErrorCodeChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

LPDISPATCH CQtTestControlCtrl::GetDispinterface() 
{
	// TODO: Add your property handler here

	return NULL;
}

void CQtTestControlCtrl::SetDispinterface(LPDISPATCH newValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

LPUNKNOWN CQtTestControlCtrl::GetUnknownInterface() 
{
	// TODO: Add your property handler here

	return NULL;
}

void CQtTestControlCtrl::SetUnknownInterface(LPUNKNOWN newValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

void CQtTestControlCtrl::OnColorChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

OLE_XPOS_PIXELS CQtTestControlCtrl::GetXpos() 
{
	// TODO: Add your property handler here

	return 0;
}

void CQtTestControlCtrl::SetXpos(OLE_XPOS_PIXELS nNewValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

OLE_YPOS_PIXELS CQtTestControlCtrl::GetYpos() 
{
	// TODO: Add your property handler here

	return 0;
}

void CQtTestControlCtrl::SetYpos(OLE_YPOS_PIXELS nNewValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

void CQtTestControlCtrl::OnWidthChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

void CQtTestControlCtrl::OnHeightChanged() 
{
	// TODO: Add notification handler code

	SetModifiedFlag();
}

OLE_HANDLE CQtTestControlCtrl::GetHandle() 
{
	// TODO: Add your property handler here

	return NULL;
}

void CQtTestControlCtrl::SetHandle(OLE_HANDLE nNewValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

OLE_TRISTATE CQtTestControlCtrl::GetTristate() 
{
	// TODO: Add your property handler here

	return (OLE_TRISTATE)0;
}

void CQtTestControlCtrl::SetTristate(OLE_TRISTATE nNewValue) 
{
	// TODO: Add your property handler here

	SetModifiedFlag();
}

BOOL CQtTestControlCtrl::GetExclusive() 
{
	// TODO: Add your property handler here

	return m_exclusive;
}

void CQtTestControlCtrl::SetExclusive(BOOL bNewValue) 
{
	// TODO: Add your property handler here

	m_exclusive = bNewValue;
	SetModifiedFlag();
}

void CQtTestControlCtrl::Sub(long x, long y, long FAR* res) 
{
	// TODO: Add your dispatch handler code here

}
