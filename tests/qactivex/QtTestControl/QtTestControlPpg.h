#if !defined(AFX_QTTESTCONTROLPPG_H__7795016D_448B_4AA9_B6AB_B8E531EEB782__INCLUDED_)
#define AFX_QTTESTCONTROLPPG_H__7795016D_448B_4AA9_B6AB_B8E531EEB782__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// QtTestControlPpg.h : Declaration of the CQtTestControlPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CQtTestControlPropPage : See QtTestControlPpg.cpp.cpp for implementation.

class CQtTestControlPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CQtTestControlPropPage)
	DECLARE_OLECREATE_EX(CQtTestControlPropPage)

// Constructor
public:
	CQtTestControlPropPage();

// Dialog Data
	//{{AFX_DATA(CQtTestControlPropPage)
	enum { IDD = IDD_PROPPAGE_QTTESTCONTROL };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CQtTestControlPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QTTESTCONTROLPPG_H__7795016D_448B_4AA9_B6AB_B8E531EEB782__INCLUDED)
