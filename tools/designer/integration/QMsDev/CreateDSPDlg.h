#if !defined(AFX_CREATEDSPDLG_H__A1D09AEF_E698_4781_9D03_2E923A1F7803__INCLUDED_)
#define AFX_CREATEDSPDLG_H__A1D09AEF_E698_4781_9D03_2E923A1F7803__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateDSPDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateDSPDlg dialog

class CCreateDSPDlg : public CDialog
{
// Construction
public:
	CCreateDSPDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateDSPDlg)
	enum { IDD = IDD_CREATEDSPDLG };
	CString	m_qtProject;
	BOOL	m_processAll;
	CString	m_qmakeOpts;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateDSPDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateDSPDlg)
	afx_msg void OnQtprojectbutton();
	afx_msg void OnClickedProcessAll();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATEDSPDLG_H__A1D09AEF_E698_4781_9D03_2E923A1F7803__INCLUDED_)
