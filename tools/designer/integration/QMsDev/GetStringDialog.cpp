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
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten GetStringDialog 
