#include <windows.h>
#include "exdll.h"

HINSTANCE g_hInstance;
HWND g_hwndParent;

#define EXPORT_NSIS_FUNCTION(funcName) \
extern "C" void __declspec(dllexport) funcName(HWND hwndParent, int string_size, \
                                               char *variables, stack_t **stacktop)

EXPORT_NSIS_FUNCTION(testMessage)
{
	char buf[1024];

	g_hwndParent = hwndParent;
	EXDLL_INIT();

	wsprintf(buf,"What?");
	MessageBox(g_hwndParent,buf,0,MB_OK);
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance=static_cast<HINSTANCE>(hInst);
	return TRUE;
}
