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

#if !defined(AFX_GETSTRINGDIALOG_H__EA4BFC25_5E50_42EB_B04D_7CA30809BF2E__INCLUDED_)
#define AFX_GETSTRINGDIALOG_H__EA4BFC25_5E50_42EB_B04D_7CA30809BF2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld GetStringDialog 

class GetStringDialog : public CDialog
{
// Konstruktion
public:
	GetStringDialog(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(GetStringDialog)
	enum { IDD = IDD_GETSTRING_DIALOG };
	CString	m_string;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(GetStringDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(GetStringDialog)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_GETSTRINGDIALOG_H__EA4BFC25_5E50_42EB_B04D_7CA30809BF2E__INCLUDED_
