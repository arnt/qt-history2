// QtWizardaw.cpp : implementation file
//

#include "stdafx.h"
#include "QtWizard.h"
#include "QtWizardaw.h"
#include "chooser.h"

#include <objmodel/bldguid.h>

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// This is called immediately after the custom AppWizard is loaded.  Initialize
//  the state of the custom AppWizard here.
void CQtWizardAppWiz::InitCustomAppWiz()
{
	// Create a new dialog chooser; CDialogChooser's constructor initializes
	//  its internal array with pointers to the steps.
	m_pChooser = new CDialogChooser;

	// Set the maximum number of steps.
	SetNumberOfSteps(LAST_DLG);

	// TODO: Add any other custom AppWizard-wide initialization here.
}

// This is called just before the custom AppWizard is unloaded.
void CQtWizardAppWiz::ExitCustomAppWiz()
{
	// Deallocate memory used for the dialog chooser
	ASSERT(m_pChooser != NULL);
	delete m_pChooser;
	m_pChooser = NULL;

	// TODO: Add code here to deallocate resources used by the custom AppWizard
}

// This is called when the user clicks "Create..." on the New Project dialog
//  or "Next" on one of the custom AppWizard's steps.
CAppWizStepDlg* CQtWizardAppWiz::Next(CAppWizStepDlg* pDlg)
{
	// Delegate to the dialog chooser
	return m_pChooser->Next(pDlg);
}

// This is called when the user clicks "Back" on one of the custom
//  AppWizard's steps.
CAppWizStepDlg* CQtWizardAppWiz::Back(CAppWizStepDlg* pDlg)
{
	// Delegate to the dialog chooser
	return m_pChooser->Back(pDlg);
}

const GUID IID_IConfiguration = { 0x96961263L,0xA819,0x11CF,0xAD,0x07,0x00,0xA0,0xC9,0x03,0x49,0x65 };

void CQtWizardAppWiz::CustomizeProject(IBuildProject* pProject)
{
	// TODO: Add code here to customize the project.  If you don't wish
	//  to customize project, you may remove this virtual override.
	
	// This is called immediately after the default Debug and Release
	//  configurations have been created for each platform.  You may customize
	//  existing configurations on this project by using the methods
	//  of IBuildProject and IConfiguration such as AddToolSettings,
	//  RemoveToolSettings, and AddCustomBuildStep. These are documented in
	//  the Developer Studio object model documentation.
	IConfiguration* pConfiguration;
	IConfigurations* pConfigurations;
	CString strTmp;
	char buffer[ 256 ];
	VARIANT varConfig, var;
	IUnknown* pUnk;
	IEnumVARIANT* pEnum;

	GetEnvironmentVariable( "QTDIR", &buffer[ 0 ], 256 );

	if( SUCCEEDED( pProject->get_Configurations( &pConfigurations ) ) )
	{
		if( SUCCEEDED( pConfigurations->get__NewEnum( &pUnk ) ) )
		{
			if( SUCCEEDED( pUnk->QueryInterface( IID_IEnumVARIANT, (void**)&pEnum ) ) )
			{
				while( pEnum->Next( 1, &varConfig, NULL ) == S_OK )
				{
					if( SUCCEEDED( varConfig.pdispVal->QueryInterface( IID_IConfiguration, (void**)&pConfiguration ) ) )
					{
						var.lVal = 0;
						var.vt = VT_I4;
						
						strTmp = CString( "/I \"" ) + &buffer[ 0 ] + "\\include\"";
						pConfiguration->AddToolSettings( CString( "cl.exe" ).AllocSysString(), strTmp.AllocSysString(), var );
						
						strTmp = CString( "/subsystem:console /libpath:\"" ) + &buffer[ 0 ] + "\\lib\" qt221.lib";
						pConfiguration->AddToolSettings( CString( "link.exe" ).AllocSysString(), strTmp.AllocSysString(), var );
						
						pConfiguration->AddToolSettings( CString( "mfc" ).AllocSysString(), CString( "0" ).AllocSysString(), var );
						pConfiguration->Release();
						pConfiguration = NULL;
					}
					varConfig.pdispVal->Release();
				}
				pEnum->Release();
			}
			pUnk->Release();
		}
		pConfigurations->Release();
	}
}


// Here we define one instance of the CQtWizardAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global QtWizardaw.
CQtWizardAppWiz QtWizardaw;

const CString CQtWizardAppWiz::m_WidgetTypes[] =
{
	"QLabel",
	"QPushButton",
	"QLCDNumber",
	"QTextView",
	"QIconView",
	"QListBox",
	"QListView",
	"QMultiLineEdit",
	"QProgressBar",
	"QTextBrowser",
	"QWorkspace"
};

const bool CQtWizardAppWiz::m_bSupportsetText[] =
{
	1,
	1,
	0,
	1,
	0,
	0,
	0,
	1,
	0,
	1,
	0
};
