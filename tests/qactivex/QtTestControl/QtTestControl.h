#if !defined(AFX_QTTESTCONTROL_H__817D1421_4D1F_408C_AAAC_24B5E57DE708__INCLUDED_)
#define AFX_QTTESTCONTROL_H__817D1421_4D1F_408C_AAAC_24B5E57DE708__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// QtTestControl.h : main header file for QTTESTCONTROL.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CQtTestControlApp : See QtTestControl.cpp for implementation.

class CQtTestControlApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QTTESTCONTROL_H__817D1421_4D1F_408C_AAAC_24B5E57DE708__INCLUDED)
