// cstm2dlg.cpp : implementation file
//

#include "stdafx.h"
#include "QtWizard.h"
#include "cstm2dlg.h"
#include "QtWizardaw.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustom2Dlg dialog


CCustom2Dlg::CCustom2Dlg()
	: CAppWizStepDlg(CCustom2Dlg::IDD)
{
	CString strTmp;
	char buffer[ 256 ];

	GetEnvironmentVariable( "QTDIR", &buffer[ 0 ], 256 );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\qtlogo.bmp";
	m_hQtLogo = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );

	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\nodetail.bmp";
	m_hNoDetail = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\nobars.bmp";
	m_hNoBars = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\onlymenu.bmp";
	m_hOnlyMenu = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\onlytoolbar.bmp";
	m_hOnlyToolbar = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\onlystatusbar.bmp";
	m_hOnlyStatusbar = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\allbars.bmp";
	m_hAllBars = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\nomenu.bmp";
	m_hNoMenu = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\notoolbar.bmp";
	m_hNoToolbar = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	strTmp = CString( &buffer[ 0 ] ) + "\\tests\\integrations\\msvc\\QtWizard\\graphics\\nostatusbar.bmp";
	m_hNoStatusbar = (HBITMAP)LoadImage( AfxGetInstanceHandle(), strTmp, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	
	//{{AFX_DATA_INIT(CCustom2Dlg)
	m_bMenuBar = true;
	m_bStatusBar = true;
	m_bToolBar = true;
	m_bComments = true;
	//}}AFX_DATA_INIT
}

CCustom2Dlg::~CCustom2Dlg()
{
	if ( m_hQtLogo )
	{
		DeleteObject( m_hQtLogo );
	}
	if ( m_hNoDetail )
	{
		DeleteObject( m_hNoDetail );
	}
	if ( m_hNoBars )
	{
		DeleteObject( m_hNoBars );
	}
	if ( m_hAllBars )
	{
		DeleteObject( m_hAllBars );
	}
	if ( m_hOnlyMenu )
	{
		DeleteObject( m_hOnlyMenu );
	}
	if ( m_hOnlyToolbar )
	{
		DeleteObject( m_hOnlyToolbar );
	}
	if ( m_hOnlyStatusbar )
	{
		DeleteObject( m_hOnlyStatusbar );
	}
	if ( m_hNoMenu )
	{
		DeleteObject( m_hNoMenu );
	}
	if ( m_hNoToolbar )
	{
		DeleteObject( m_hNoToolbar );
	}
	if ( m_hNoStatusbar )
	{
		DeleteObject( m_hNoStatusbar );
	}
}

void CCustom2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CAppWizStepDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustom2Dlg)
	DDX_Control(pDX, IDC_CENTRALWIDGET, m_ctrlCentralWidget);
	DDX_Check(pDX, IDC_MENUBAR, m_bMenuBar);
	DDX_Check(pDX, IDC_STATUSBAR, m_bStatusBar);
	DDX_Check(pDX, IDC_TOOLBAR, m_bToolBar);
	DDX_Check(pDX, IDC_COMMENTS, m_bComments);
	//}}AFX_DATA_MAP
}

// This is called whenever the user presses Next, Back, or Finish with this step
//  present.  Do all validation & data exchange from the dialog in this function.
BOOL CCustom2Dlg::OnDismiss()
{
	CString strTmp;

	if (!UpdateData(TRUE))
		return FALSE;

	if ( m_ctrlCentralWidget.GetCurSel() != -1 )
	{
		QtWizardaw.m_Dictionary[ "QT_CENTRAL_WIDGET_TYPE" ] = CQtWizardAppWiz::WidgetTypes[ m_ctrlCentralWidget.GetItemData( m_ctrlCentralWidget.GetCurSel() ) ];
	}
	else
	{
		QtWizardaw.m_Dictionary[ "QT_CENTRAL_WIDGET_TYPE" ] = CQtWizardAppWiz::WidgetTypes[ CQtWizardAppWiz::WIDGET_TEXTVIEW ];
	}
	if ( m_bMenuBar )
	{
		QtWizardaw.m_Dictionary[ "QT_MENUBAR" ] = "Yes";
	}
	else
	{
		QtWizardaw.m_Dictionary.RemoveKey( "QT_MENUBAR" );
	}
	if ( m_bToolBar )
	{
		QtWizardaw.m_Dictionary[ "QT_TOOLBAR" ] = "Yes";
	}
	else
	{
		QtWizardaw.m_Dictionary.RemoveKey( "QT_TOOLBAR" );
	}
	if ( m_bStatusBar )
	{
		QtWizardaw.m_Dictionary[ "QT_STATUSBAR" ] = "Yes";
	}
	else
	{
		QtWizardaw.m_Dictionary.RemoveKey( "QT_STATUSBAR" );
	}
	if ( m_bComments )
	{
		QtWizardaw.m_Dictionary[ "QT_COMMENTS" ] = "Yes";
	}
	else
	{
		QtWizardaw.m_Dictionary.RemoveKey( "QT_COMMENTS" );
	}

	if ( QtWizardaw.m_Dictionary[ "QT_CENTRAL_WIDGET_TYPE" ] == CQtWizardAppWiz::WidgetTypes[ CQtWizardAppWiz::WIDGET_WORKSPACE ] )
	{
		QtWizardaw.m_Dictionary[ "QT_BACKGROUND" ] = "background.png";
	}
	else
	{
		QtWizardaw.m_Dictionary.RemoveKey( "QT_BACKGROUND" );
	}

	return TRUE;	// return FALSE if the dialog shouldn't be dismissed
}


BEGIN_MESSAGE_MAP(CCustom2Dlg, CAppWizStepDlg)
	//{{AFX_MSG_MAP(CCustom2Dlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_MENUBAR, OnOptions)
	ON_BN_CLICKED(IDC_TOOLBAR, OnOptions)
	ON_BN_CLICKED(IDC_STATUSBAR, OnOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCustom2Dlg message handlers

BOOL CCustom2Dlg::OnInitDialog() 
{
	HBITMAP hBm;
	int i;

	CAppWizStepDlg::OnInitDialog();
	
	hBm = (HBITMAP)LoadImage( AfxGetInstanceHandle(), "%QTDIR%\\msvc\\qtpage2.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );
	m_ctrlPicture.SetBitmap( hBm );
	
	m_ctrlPicture.Create( "", SS_BITMAP | WS_CHILD | WS_VISIBLE, CRect( 10, 10, 210, 167 ), this );
	m_ctrlDetail.Create( "", SS_BITMAP | WS_CHILD | WS_VISIBLE, CRect( 10, 167, 210, 310 ), this );

	m_ctrlPicture.SetBitmap( m_hQtLogo );
	m_ctrlDetail.SetBitmap( m_hAllBars );

	for ( i = 0; i < CQtWizardAppWiz::WIDGET_MAX; i++ )
	{
		m_ctrlCentralWidget.SetItemData( m_ctrlCentralWidget.AddString( CQtWizardAppWiz::WidgetTypes[ i ] ), i );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustom2Dlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	if ( bShow )
	{
		if ( QtWizardaw.m_Dictionary[ "PROJECT_QT_GUI" ] == "Y" )
		{
			GetDlgItem( IDC_MENUBAR )->EnableWindow( TRUE );
			GetDlgItem( IDC_TOOLBAR )->EnableWindow( TRUE );
			GetDlgItem( IDC_STATUSBAR )->EnableWindow( TRUE );
			GetDlgItem( IDC_CENTRALWIDGET )->EnableWindow( TRUE );
		}
		else
		{
			GetDlgItem( IDC_MENUBAR )->EnableWindow( FALSE );
			GetDlgItem( IDC_TOOLBAR )->EnableWindow( FALSE );
			GetDlgItem( IDC_STATUSBAR )->EnableWindow( FALSE );
			GetDlgItem( IDC_CENTRALWIDGET )->EnableWindow( FALSE );
		}
	}
	CAppWizStepDlg::OnShowWindow(bShow, nStatus);
}

void CCustom2Dlg::OnOptions() 
{
	UpdateData( TRUE );

	if ( m_bMenuBar )
	{
		if ( m_bToolBar )
		{
			if ( m_bStatusBar )
			{
				m_ctrlDetail.SetBitmap( m_hAllBars );
			}
			else
			{
				m_ctrlDetail.SetBitmap( m_hNoStatusbar );
			}
		}
		else
		{
			if ( m_bStatusBar )
			{
				m_ctrlDetail.SetBitmap( m_hNoToolbar );
			}
			else
			{
				m_ctrlDetail.SetBitmap( m_hOnlyMenu );
			}
		}
	}
	else
	{
		if ( m_bToolBar )
		{
			if ( m_bStatusBar )
			{
				m_ctrlDetail.SetBitmap( m_hNoMenu );
			}
			else
			{
				m_ctrlDetail.SetBitmap( m_hOnlyToolbar );
			}
		}
		else
		{
			if ( m_bStatusBar )
			{
				m_ctrlDetail.SetBitmap( m_hOnlyStatusbar );
			}
			else
			{
				m_ctrlDetail.SetBitmap( m_hNoBars );
			}
		}
	}
}
