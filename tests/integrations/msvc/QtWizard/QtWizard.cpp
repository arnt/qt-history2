// QtWizard.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "QtWizard.h"
#include "QtWizardaw.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE QtWizardDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("QTWIZARD.AWX Initializing!\n");
		
		// Extension DLL one-time initialization
		AfxInitExtensionModule(QtWizardDLL, hInstance);

		// Insert this DLL into the resource chain
		new CDynLinkLibrary(QtWizardDLL);

		// Register this custom AppWizard with MFCAPWZ.DLL
		SetCustomAppWizClass(&QtWizardaw);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("QTWIZARD.AWX Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(QtWizardDLL);
	}
	return 1;   // ok
}
