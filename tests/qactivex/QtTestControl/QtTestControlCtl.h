#if !defined(AFX_QTTESTCONTROLCTL_H__E7DA17E1_71B6_4ABD_8AD0_71A30806CE68__INCLUDED_)
#define AFX_QTTESTCONTROLCTL_H__E7DA17E1_71B6_4ABD_8AD0_71A30806CE68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// QtTestControlCtl.h : Declaration of the CQtTestControlCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CQtTestControlCtrl : See QtTestControlCtl.cpp for implementation.

class CQtTestControlCtrl : public COleControl
{
	DECLARE_DYNCREATE(CQtTestControlCtrl)

// Constructor
public:
	CQtTestControlCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQtTestControlCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CQtTestControlCtrl();

	DECLARE_OLECREATE_EX(CQtTestControlCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CQtTestControlCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CQtTestControlCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CQtTestControlCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CQtTestControlCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CQtTestControlCtrl)
	double m_double;
	afx_msg void OnDoubleChanged();
	CString m_string;
	afx_msg void OnStringChanged();
	DATE m_date;
	afx_msg void OnDateChanged();
	SCODE m_errorCode;
	afx_msg void OnErrorCodeChanged();
	OLE_COLOR m_color;
	afx_msg void OnColorChanged();
	OLE_XSIZE_PIXELS m_width;
	afx_msg void OnWidthChanged();
	OLE_YSIZE_PIXELS m_height;
	afx_msg void OnHeightChanged();
	afx_msg long GetInt();
	afx_msg void SetInt(long nNewValue);
	afx_msg short GetShort();
	afx_msg void SetShort(short nNewValue);
	afx_msg float GetFloat();
	afx_msg void SetFloat(float newValue);
	afx_msg CURRENCY GetCurrency();
	afx_msg void SetCurrency(CURRENCY newValue);
	afx_msg BOOL GetBool();
	afx_msg void SetBool(BOOL bNewValue);
	afx_msg VARIANT GetVariant();
	afx_msg void SetVariant(const VARIANT FAR& newValue);
	afx_msg LPDISPATCH GetDispinterface();
	afx_msg void SetDispinterface(LPDISPATCH newValue);
	afx_msg LPUNKNOWN GetUnknownInterface();
	afx_msg void SetUnknownInterface(LPUNKNOWN newValue);
	afx_msg OLE_XPOS_PIXELS GetXpos();
	afx_msg void SetXpos(OLE_XPOS_PIXELS nNewValue);
	afx_msg OLE_YPOS_PIXELS GetYpos();
	afx_msg void SetYpos(OLE_YPOS_PIXELS nNewValue);
	afx_msg OLE_HANDLE GetHandle();
	afx_msg void SetHandle(OLE_HANDLE nNewValue);
	afx_msg OLE_TRISTATE GetTristate();
	afx_msg void SetTristate(OLE_TRISTATE nNewValue);
	afx_msg BOOL GetExclusive();
	afx_msg void SetExclusive(BOOL bNewValue);
	afx_msg void SetString(LPCTSTR string);
	afx_msg long Add(long x, long y);
	afx_msg void Sub(long x, long y, long FAR* res);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();
	long m_int;

// Event maps
	//{{AFX_EVENT(CQtTestControlCtrl)
	void FireFoo()
		{FireEvent(eventidFoo,EVENT_PARAM(VTS_NONE));}
	void FireStringChanged(LPCTSTR string)
		{FireEvent(eventidStringChanged,EVENT_PARAM(VTS_BSTR), string);}
	void FireGetInt(long FAR* i)
		{FireEvent(eventidGetInt,EVENT_PARAM(VTS_PI4), i);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CQtTestControlCtrl)
	dispidDouble = 1L,
	dispidInt = 8L,
	dispidString = 2L,
	dispidDate = 3L,
	dispidShort = 9L,
	dispidFloat = 10L,
	dispidCurrency = 11L,
	dispidBool = 12L,
	dispidVariant = 13L,
	dispidErrorCode = 4L,
	dispidDispinterface = 14L,
	dispidUnknownInterface = 15L,
	dispidColor = 5L,
	dispidXpos = 16L,
	dispidYpos = 17L,
	dispidWidth = 6L,
	dispidHeight = 7L,
	dispidHandle = 18L,
	dispidTristate = 19L,
	dispidExclusive = 20L,
	dispidSetString = 21L,
	dispidAdd = 22L,
	dispidSub = 23L,
	eventidFoo = 1L,
	eventidStringChanged = 2L,
	eventidGetInt = 3L,
	//}}AFX_DISP_ID
	};
private:
	BOOL m_exclusive;
	VARIANT m_variant;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QTTESTCONTROLCTL_H__E7DA17E1_71B6_4ABD_8AD0_71A30806CE68__INCLUDED)
