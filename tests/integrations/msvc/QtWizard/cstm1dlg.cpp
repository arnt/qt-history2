// cstm1dlg.cpp : implementation file
//

#include "stdafx.h"
#include "QtWizard.h"
#include "cstm1dlg.h"
#include "QtWizardaw.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg dialog


CCustom1Dlg::CCustom1Dlg()
	: CAppWizStepDlg(CCustom1Dlg::IDD)
{
	CString strTmp;
	char buffer[ 256 ];

	GetEnvironmentVariable( "QTDIR", &buffer[ 0 ], 256 );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\qtlogo.bmp";
	m_hQtLogo = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );

	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\nodetail.bmp";
	m_hNoDetail = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\allbars.bmp";
	m_hFullGUI = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\qtgui.bmp";
	m_hQtGUI = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\nogui.bmp";
	m_hNoGUI = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	//{{AFX_DATA_INIT(CCustom1Dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CCustom1Dlg::~CCustom1Dlg()
{
	if ( m_hQtLogo )
	{
		DeleteObject( m_hQtLogo );
	}
	if ( m_hNoDetail )
	{
		DeleteObject( m_hNoDetail );
	}
	if ( m_hNoGUI )
	{
		DeleteObject( m_hNoDetail );
	}
	if ( m_hQtGUI )
	{
		DeleteObject( m_hNoDetail );
	}
}

void CCustom1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CAppWizStepDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustom1Dlg)
	DDX_Control(pDX, IDC_PROJECTTYPE, m_ctrlProjectTypes);
	//}}AFX_DATA_MAP
}

// This is called whenever the user presses Next, Back, or Finish with this step
//  present.  Do all validation & data exchange from the dialog in this function.
BOOL CCustom1Dlg::OnDismiss()
{
	CString strTmp;

	if (!UpdateData(TRUE))
		return FALSE;

	strTmp.Format( "%d", m_ctrlProjectTypes.GetItemData( m_ctrlProjectTypes.GetCurSel() ) );
	QtWizardaw.m_Dictionary.RemoveKey( "PROJECT_EMPTY_QT" );
	QtWizardaw.m_Dictionary.RemoveKey( "PROJECT_QT_NONGUI" );
	QtWizardaw.m_Dictionary.RemoveKey( "PROJECT_QT_GUIREADY" );
	QtWizardaw.m_Dictionary.RemoveKey( "PROJECT_QT_GUI" );

	switch ( m_ctrlProjectTypes.GetItemData( m_ctrlProjectTypes.GetCurSel() ) )
	{
	case CQtWizardAppWiz::PROJECT_EMPTY_QT:
		QtWizardaw.m_Dictionary[ "PROJECT_EMPTY_QT" ] = "Y";
		break;
	case CQtWizardAppWiz::PROJECT_QT_NONGUI:
		QtWizardaw.m_Dictionary[ "PROJECT_QT_NONGUI" ] = "Y";
		break;
	case CQtWizardAppWiz::PROJECT_QT_GUIREADY:
		QtWizardaw.m_Dictionary[ "PROJECT_QT_GUIREADY" ] = "Y";
		break;
	case CQtWizardAppWiz::PROJECT_QT_GUI:
		QtWizardaw.m_Dictionary[ "PROJECT_QT_GUI" ] = "Y";
		break;
	}

	return TRUE;	// return FALSE if the dialog shouldn't be dismissed
}


BEGIN_MESSAGE_MAP(CCustom1Dlg, CAppWizStepDlg)
	//{{AFX_MSG_MAP(CCustom1Dlg)
	ON_LBN_SELCHANGE(IDC_PROJECTTYPE, OnSelchangeProjecttype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg message handlers

BOOL CCustom1Dlg::OnInitDialog() 
{
	CAppWizStepDlg::OnInitDialog();
	
	m_ctrlProjectTypes.SetItemData( m_ctrlProjectTypes.AddString( "Empty Qt-enabled project" ), CQtWizardAppWiz::PROJECT_EMPTY_QT );
	m_ctrlProjectTypes.SetItemData( m_ctrlProjectTypes.AddString( "A non-GUI Qt-enabled application" ), CQtWizardAppWiz::PROJECT_QT_NONGUI );
	m_ctrlProjectTypes.SetItemData( m_ctrlProjectTypes.AddString( "A GUI-ready Qt-enabled application" ), CQtWizardAppWiz::PROJECT_QT_GUIREADY );
	m_ctrlProjectTypes.SetItemData( m_ctrlProjectTypes.AddString( "A Qt application with a basic GUI" ), CQtWizardAppWiz::PROJECT_QT_GUI );

	m_ctrlPicture.Create( "", SS_BITMAP | WS_CHILD | WS_VISIBLE, CRect( 10, 10, 210, 167 ), this );
	m_ctrlDetail.Create( "", SS_BITMAP | WS_CHILD | WS_VISIBLE, CRect( 10, 167, 210, 310 ), this );

	m_ctrlPicture.SetBitmap( m_hQtLogo );
	m_ctrlDetail.SetBitmap( m_hNoDetail );

	m_ctrlProjectTypes.SetCurSel( 0 );
	QtWizardaw.m_Dictionary[ "PROJECT_EMPTY_QT" ] = "Y";
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustom1Dlg::OnSelchangeProjecttype() 
{
	CString strDescription;
	DWORD dwCurrent( m_ctrlProjectTypes.GetItemData( m_ctrlProjectTypes.GetCurSel() ) );

	strDescription.LoadString( dwCurrent );
	SetDlgItemText( IDC_DESCRIPTION, strDescription );

	switch( dwCurrent )
	{
	case CQtWizardAppWiz::PROJECT_EMPTY_QT:
		m_ctrlDetail.SetBitmap( m_hNoDetail );
		break;
	case CQtWizardAppWiz::PROJECT_QT_NONGUI:
		m_ctrlDetail.SetBitmap( m_hNoGUI );
		break;
	case CQtWizardAppWiz::PROJECT_QT_GUIREADY:
		m_ctrlDetail.SetBitmap( m_hQtGUI );
		break;
	case CQtWizardAppWiz::PROJECT_QT_GUI:
		m_ctrlDetail.SetBitmap( m_hFullGUI );
		break;
	}
}
