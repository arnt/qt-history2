/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Qt integration for Microsoft Visual Studio.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "stdafx.h"
#include "QMsDev.h"
#include "GetStringDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld GetStringDialog 


GetStringDialog::GetStringDialog(CWnd* pParent /*=NULL*/)
	: CDialog(GetStringDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(GetStringDialog)
	m_string = _T("");
	//}}AFX_DATA_INIT
}


void GetStringDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GetStringDialog)
	DDX_Text(pDX, IDC_NAME, m_string);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GetStringDialog, CDialog)
	//{{AFX_MSG_MAP(GetStringDialog)
		// HINWEIS: Der Klassen-Assistent f�gt hier Zuordnungsmakros f�r Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten GetStringDialog 
