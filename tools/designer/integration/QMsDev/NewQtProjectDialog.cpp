/****************************************************************************
** $Id$
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt integration for Microsoft Visual Studio.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "stdafx.h"
#include "QMsDev.h"
#include <afxdlgs.h>
#include <direct.h>
#include "NewQtProjectDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld NewQtProjectDialog 


NewQtProjectDialog::NewQtProjectDialog(CWnd* pParent /*=NULL*/)
	: CDialog(NewQtProjectDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(NewQtProjectDialog)
	m_mdi = FALSE;
	m_dialog = TRUE;
	m_name = _T("NewProject");
	m_location = _T("");
	//}}AFX_DATA_INIT	
}

void NewQtProjectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NewQtProjectDialog)
	DDX_Control(pDX, IDC_PROJECTLOCATION, c_location);
	DDX_Control(pDX, IDC_PROJECTNAME, c_name);
	DDX_Control(pDX, IDC_APPMDI, c_mdi);
	DDX_Check(pDX, IDC_APPMDI, m_mdi);
	DDX_Text(pDX, IDC_PROJECTLOCATION, m_location);
	DDX_Text(pDX, IDC_PROJECTNAME, m_name);
	DDX_Check(pDX, IDC_APPDIALOG, m_dialog);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NewQtProjectDialog, CDialog)
	//{{AFX_MSG_MAP(NewQtProjectDialog)
	ON_BN_CLICKED(IDC_PROJECTLOOKUP, OnProjectLookup)
	ON_BN_CLICKED(IDC_APPDIALOG, OnAppDialog)
	ON_BN_CLICKED(IDC_APPMAIN, OnAppMain)
	ON_EN_CHANGE(IDC_PROJECTNAME, OnProjectNameChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

extern bool getGlobalQtSettings( bool & );

BOOL NewQtProjectDialog::OnInitDialog()
{
    bool shared;
    bool threaded = getGlobalQtSettings( shared );
    m_shared = shared;
    m_static = !shared;

    CDialog::OnInitDialog();
    c_mdi.EnableWindow( !m_dialog );
    char cwd[256];
    m_location = _T(getcwd((char*)&cwd, 256 ));
    if ( m_location.GetAt( m_location.GetLength()-1 ) != '\\' )
	c_location.SetWindowText( m_location+ "\\" + m_name );
    else 
	c_location.SetWindowText( m_location+m_name );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten NewQtProjectDialog 

void NewQtProjectDialog::OnProjectLookup() 
{
    TCHAR path[MAX_PATH];
    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = 0;
    bi.lpszTitle = "Select a directory";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
    LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bi);
    if (pItemIDList) {
	SHGetPathFromIDList(pItemIDList, path);
	IMalloc *pMalloc;
	if (SHGetMalloc(&pMalloc) != NOERROR)
	    return;
	else {
	    pMalloc->Free(pItemIDList);
	    pMalloc->Release();
	}

	m_location = CString( path );
	m_location+='\\';

	CString name;
	c_name.GetWindowText( name );

	c_location.SetWindowText( m_location + name );
    }
}

void NewQtProjectDialog::OnProjectNameChange()
{
    CString location, name;
    c_location.GetWindowText( location );
    int endpath = location.ReverseFind( '\\' );
    if ( endpath != -1 )
	location = location.Left( endpath );

    c_name.GetWindowText( name );

    if ( location.GetAt( location.GetLength()-1 ) != '\\' )
	location+='\\';

    c_location.SetWindowText( location + name );
}

void NewQtProjectDialog::OnAppDialog() 
{
    m_dialog = TRUE;
    c_mdi.EnableWindow( FALSE );
}

void NewQtProjectDialog::OnAppMain() 
{
    m_dialog = FALSE;
    c_mdi.EnableWindow( TRUE );
}


