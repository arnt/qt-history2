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

#if !defined(AFX_DSADDIN_H__A7A23BC1_13F2_488C_B5F5_57B23FAA215B__INCLUDED_)
#define AFX_DSADDIN_H__A7A23BC1_13F2_488C_B5F5_57B23FAA215B__INCLUDED_

#include "commands.h"

// {6CED40CF-FF72-4369-8AA4-6A917FEF55DD}
DEFINE_GUID(CLSID_DSAddIn,
0x6ced40cf, 0xff72, 0x4369, 0x8a, 0xa4, 0x6a, 0x91, 0x7f, 0xef, 0x55, 0xdd);

/////////////////////////////////////////////////////////////////////////////
// CDSAddIn

class CDSAddIn : 
	public IDSAddIn,
	public CComObjectRoot,
	public CComCoClass<CDSAddIn, &CLSID_DSAddIn>
{
public:
	DECLARE_REGISTRY(CDSAddIn, "QMsDev.DSAddIn.1",
		"QMSDEV Developer Studio Add-in", IDS_QMSDEV_LONGNAME,
		THREADFLAGS_BOTH)

	CDSAddIn() {}
	BEGIN_COM_MAP(CDSAddIn)
		COM_INTERFACE_ENTRY(IDSAddIn)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CDSAddIn)

// IDSAddIns
public:
	STDMETHOD(OnConnection)(THIS_ IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection);
	STDMETHOD(OnDisconnection)(THIS_ VARIANT_BOOL bLastTime);

protected:
	CCommandsObj* m_pCommands;
	DWORD m_dwCookie;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_DSADDIN_H__A7A23BC1_13F2_488C_B5F5_57B23FAA215B__INCLUDED)
