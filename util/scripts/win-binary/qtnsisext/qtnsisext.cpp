#include <windows.h>
#include "exdll.h"
#include "keycheck.h"
#include "licensefinder.h"
#include "binpatch.h"

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

    popstring(buf);
    MessageBox(g_hwndParent,buf,0,MB_OK);
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    g_hInstance=static_cast<HINSTANCE>(hInst);
    return TRUE;
}

EXPORT_NSIS_FUNCTION(IsValidLicense)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    char key1[5];
    char key2[5];
    char key3[5];
    char isValid[2];

    popstring(key1);
    popstring(key2);
    popstring(key3);

    KeyCheck c(key1, key2, key3);
    if (c.isValidWindowsLicense())
        isValid[0] = '1';
    else
        isValid[0] = '0';
    isValid[1] = '\0';

    pushstring(isValid);
}

EXPORT_NSIS_FUNCTION(UsesUSLicense)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    char key1[5];
    char key2[5];
    char key3[5];
    char isValid[2];

    popstring(key1);
    popstring(key2);
    popstring(key3);

    KeyCheck c(key1, key2, key3);
    if (c.usesUSLicense())
        isValid[0] = '1';
    else
        isValid[0] = '0';
    isValid[1] = '\0';

    pushstring(isValid);
}

EXPORT_NSIS_FUNCTION(GetLicenseInfo)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    LicenseFinder f;
    pushstring(f.getLicenseKey(3));
    pushstring(f.getLicenseKey(2));
    pushstring(f.getLicenseKey(1));
    pushstring(f.getLicensee());
}

EXPORT_NSIS_FUNCTION(PatchBinary)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();
    
    char *fileName = (char *)LocalAlloc(LPTR, g_stringsize);
    char *oldStr = (char *)LocalAlloc(LPTR, g_stringsize);
    char *newStr = (char *)LocalAlloc(LPTR, g_stringsize);
    char *endsWith = (char *)LocalAlloc(LPTR, g_stringsize);

    popstring(fileName);
    popstring(oldStr);
    popstring(newStr);
    popstring(endsWith);

    BinPatch::patchFile(fileName, oldStr, newStr, endsWith);

    LocalFree(endsWith);
    LocalFree(newStr);
    LocalFree(oldStr);
    LocalFree(fileName);
}

EXPORT_NSIS_FUNCTION(PatchBinaryInsert)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();
    
    char *oldFile = (char *)LocalAlloc(LPTR, g_stringsize);
    char *newFile = (char *)LocalAlloc(LPTR, g_stringsize);
    char *oldStr = (char *)LocalAlloc(LPTR, g_stringsize);
    char *newStr = (char *)LocalAlloc(LPTR, g_stringsize);

    popstring(oldFile);
    popstring(newFile);
    popstring(oldStr);
    popstring(newStr);

    BinPatch::patchFileInsert(oldFile, newFile, oldStr, newStr, true);

    LocalFree(newStr);
    LocalFree(oldStr);
    LocalFree(newFile);
    LocalFree(oldFile);
}