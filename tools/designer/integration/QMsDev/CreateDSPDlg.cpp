// CreateDSPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "qmsdev.h"
#include "CreateDSPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateDSPDlg dialog


CCreateDSPDlg::CCreateDSPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateDSPDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateDSPDlg)
	m_qtProject = _T("");
	m_processAll = FALSE;
	m_qmakeOpts = _T("");
	//}}AFX_DATA_INIT
}


void CCreateDSPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateDSPDlg)
	DDX_Text(pDX, IDC_QTPROJECT, m_qtProject);
	DDX_Check(pDX, IDC_PROCESSALL, m_processAll);
	DDX_Text(pDX, IDC_QMAKEOPTS, m_qmakeOpts);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateDSPDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateDSPDlg)
	ON_BN_CLICKED(IDC_QTPROJECTBUTTON, OnQtprojectbutton)
	ON_BN_CLICKED(IDC_PROCESSALL, OnClickedProcessAll )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateDSPDlg message handlers

void CCreateDSPDlg::OnQtprojectbutton() 
{
    CFileDialog dlg( true, ".pro", NULL, 0, "Project files (*.pro)|*.pro", this );

    if( dlg.DoModal() == IDOK ) {
	SetDlgItemText( IDC_QTPROJECT, dlg.GetPathName() );
    }
}

void CCreateDSPDlg::OnClickedProcessAll()
{
    GetDlgItem( IDC_QTPROJECT )->EnableWindow( !( (CButton*)GetDlgItem( IDC_PROCESSALL ) )->GetCheck() );
}

BOOL CCreateDSPDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CString qtProject;

	GetDlgItemText( IDC_QTPROJECT, qtProject );
	GetDlgItem( IDC_PROCESSALL )->EnableWindow( qtProject.GetLength() > 0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
