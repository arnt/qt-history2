#if !defined(AFX_QTWIZARDAW_H__D68FDF34_7778_412D_8DB4_728712B092F7__INCLUDED_)
#define AFX_QTWIZARDAW_H__D68FDF34_7778_412D_8DB4_728712B092F7__INCLUDED_

// QtWizardaw.h : header file
//

class CDialogChooser;

// All function calls made by mfcapwz.dll to this custom AppWizard (except for
//  GetCustomAppWizClass-- see QtWizard.cpp) are through this class.  You may
//  choose to override more of the CCustomAppWiz virtual functions here to
//  further specialize the behavior of this custom AppWizard.
class CQtWizardAppWiz : public CCustomAppWiz
{
public:
	virtual CAppWizStepDlg* Next(CAppWizStepDlg* pDlg);
	virtual CAppWizStepDlg* Back(CAppWizStepDlg* pDlg);
		
	virtual void InitCustomAppWiz();
	virtual void ExitCustomAppWiz();
	virtual void CustomizeProject(IBuildProject* pProject);
	void splitFileName( CString& strFile, CString& strPath, CString& strName, CString& strExt );

protected:
	CDialogChooser* m_pChooser;

public:
	enum
	{
		PROJECT_EMPTY_QT = 1,
		PROJECT_QT_NONGUI = 2,
		PROJECT_QT_GUIREADY = 3,
		PROJECT_QT_GUI = 4
	};
	enum
	{
		WIDGET_LABEL = 0,
		WIDGET_PUSHBUTTON,
		WIDGET_LCDNUMBER,
		WIDGET_TEXTVIEW,
		WIDGET_ICONVIEW,
		WIDGET_LISTBOX,
		WIDGET_LISTVIEW,
		WIDGET_MULTILINEEDIT,
		WIDGET_PROGRESSBAR,
		WIDGET_TEXTBROWSER,
		WIDGET_CUSTOM,

		WIDGET_MAX
	};

	static const CString m_WidgetTypes[];
	static const bool m_bSupportsetText[];
};

// This declares the one instance of the CQtWizardAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global QtWizardaw.  (Its definition is in QtWizardaw.cpp.)
extern CQtWizardAppWiz QtWizardaw;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QTWIZARDAW_H__D68FDF34_7778_412D_8DB4_728712B092F7__INCLUDED_)
