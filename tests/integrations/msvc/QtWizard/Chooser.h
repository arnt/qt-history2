#if !defined(AFX_CHOOSER_H__84A13CF6_9791_4A57_9F36_46FA7B58FD47__INCLUDED_)
#define AFX_CHOOSER_H__84A13CF6_9791_4A57_9F36_46FA7B58FD47__INCLUDED_

// chooser.h : declaration of the CDialogChooser class
//             This class keeps track of what dialogs to pop up when.

#define LAST_DLG 2

class CDialogChooser
{
public:
	CDialogChooser();
	~CDialogChooser();

	// All calls by mfcapwz.dll to CQtWizardAppWiz::Next
	//  & CQtWizardAppWiz::Back are delegated to these member
	//  functions, which keep track of what dialog is up
	//  now, and what to pop up next.
	CAppWizStepDlg* Next(CAppWizStepDlg* pDlg);
	CAppWizStepDlg* Back(CAppWizStepDlg* pDlg);

protected:
	// Current step's index into the internal array m_pDlgs
	int m_nCurrDlg;

	// Internal array of pointers to the steps
	CAppWizStepDlg* m_pDlgs[LAST_DLG + 1];
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSER_H__84A13CF6_9791_4A57_9F36_46FA7B58FD47__INCLUDED_)
