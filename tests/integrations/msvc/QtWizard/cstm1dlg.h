#if !defined(AFX_CSTM1DLG_H__1E98FD80_C87A_4676_8177_7C41CC9633DA__INCLUDED_)
#define AFX_CSTM1DLG_H__1E98FD80_C87A_4676_8177_7C41CC9633DA__INCLUDED_

// cstm1dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg dialog

class CCustom1Dlg : public CAppWizStepDlg
{
// Construction
public:
	CCustom1Dlg();
	virtual ~CCustom1Dlg();
	virtual BOOL OnDismiss();

// Dialog Data
	//{{AFX_DATA(CCustom1Dlg)
	enum { IDD = IDD_CUSTOM1 };
	CListBox	m_ctrlProjectTypes;
	//}}AFX_DATA
	CStatic	m_ctrlPicture;
	CStatic m_ctrlDetail;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustom1Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCustom1Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeProjecttype();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HBITMAP m_hQtLogo;
	HBITMAP m_hNoDetail;
	HBITMAP m_hFullGUI;
	HBITMAP m_hQtGUI;
	HBITMAP m_hNoGUI;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSTM1DLG_H__1E98FD80_C87A_4676_8177_7C41CC9633DA__INCLUDED_)
