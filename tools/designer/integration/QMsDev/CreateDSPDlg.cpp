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
	//}}AFX_DATA_INIT
}


void CCreateDSPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateDSPDlg)
	DDX_Text(pDX, IDC_QTPROJECT, m_qtProject);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateDSPDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateDSPDlg)
	ON_BN_CLICKED(IDC_QTPROJECTBUTTON, OnQtprojectbutton)
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

void CCreateDSPDlg::OnOK() 
{
    CString qtProject;
    CString projectPath;
    CString projectBase;
    CString qmakeOpts;
    CString qtDir( getenv( "QTDIR" ) );

    GetDlgItemText( IDC_QTPROJECT, qtProject );
    GetDlgItemText( IDC_QMAKEOPTS, qmakeOpts );
    projectPath = qtProject.Left( qtProject.ReverseFind( '\\' ) );
    projectBase = qtProject.Right( qtProject.GetLength() - projectPath.GetLength() );
    projectBase = projectBase.Left( projectBase.GetLength() - 4 );

    CString command( qtDir + "\\bin\\qmake.exe " + qtProject + " -o " + projectPath + projectBase + ".dsp" + " -t " );
    if( projectIsLibrary( qtProject ) )
	command += "vclib " + qmakeOpts;
    else
	command += "vcapp " + qmakeOpts;

    if( system( command ) == 0 )
        CDialog::OnOK();
    else
	AfxMessageBox( "An error occurred while processing the project file" );
}

bool CCreateDSPDlg::projectIsLibrary( CString projectPath )
{
    CFile inFile;

    CString buffer;
    char c;

    if( inFile.Open( projectPath, CFile::modeRead ) ) {
	while( inFile.Read( &c, sizeof( c ) ) ) {
	    if( c != '\n' )
		buffer += c;
	    else {
		// A complete line has been read.
		if( buffer.Left( 8 ) == "TEMPLATE" ) {
		    // This is the template line
		    if( buffer.Find( "lib" ) != -1 )
			return true;
		    break;
		}
		buffer.Empty();
	    }
	}
    }

    return false;
}