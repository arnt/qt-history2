#if !defined(AFX_CSTM2DLG_H__09D4671E_9E02_40E1_AD4E_75C9A13CC192__INCLUDED_)
#define AFX_CSTM2DLG_H__09D4671E_9E02_40E1_AD4E_75C9A13CC192__INCLUDED_

// cstm2dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustom2Dlg dialog

class CCustom2Dlg : public CAppWizStepDlg
{
// Construction
public:
	CCustom2Dlg();
	~CCustom2Dlg();
	virtual BOOL OnDismiss();

// Dialog Data
	//{{AFX_DATA(CCustom2Dlg)
	enum { IDD = IDD_CUSTOM2 };
	CComboBox	m_ctrlCentralWidget;
	BOOL	m_bMenuBar;
	BOOL	m_bStatusBar;
	BOOL	m_bToolBar;
	BOOL	m_bComments;
	//}}AFX_DATA
	CStatic	m_ctrlPicture;
	CStatic m_ctrlDetail;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustom2Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCustom2Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBITMAP m_hQtLogo;
	HBITMAP m_hNoDetail;
	HBITMAP m_hNoBars;
	HBITMAP m_hOnlyMenu;
	HBITMAP m_hOnlyToolbar;
	HBITMAP m_hOnlyStatusbar;
	HBITMAP m_hAllBars;
	HBITMAP m_hNoMenu;
	HBITMAP m_hNoToolbar;
	HBITMAP m_hNoStatusbar;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSTM2DLG_H__09D4671E_9E02_40E1_AD4E_75C9A13CC192__INCLUDED_)
