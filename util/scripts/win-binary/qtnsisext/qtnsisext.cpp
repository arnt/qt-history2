#include <windows.h>
#include "exdll.h"
#include "keydec.h"
#include "licensefinder.h"
#include "binpatch.h"

HINSTANCE g_hInstance;
HWND g_hwndParent;

#define EXPORT_NSIS_FUNCTION(funcName) \
extern "C" void __declspec(dllexport) funcName(HWND hwndParent, int string_size, \
                                               char *variables, stack_t **stacktop)

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    g_hInstance=static_cast<HINSTANCE>(hInst);
    return TRUE;
}

EXPORT_NSIS_FUNCTION(IsValidLicense)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    char isValid[2];
    char *key = (char *)LocalAlloc(LPTR, g_stringsize+1);
    popstring(key);

    KeyDecoder kdec(key);
    if (kdec.IsValid() && (kdec.getPlatforms() & KeyDecoder::PlatformWindows)
        && ((kdec.getProducts() & KeyDecoder::QtUniversal)
        || (kdec.getProducts() & KeyDecoder::QtDesktop)
        || (kdec.getProducts() & KeyDecoder::QtDesktopLight)))
        strcpy(isValid, "1");
    else
        strcpy(isValid, "0");

    LocalFree(key);
    pushstring(isValid);
}

EXPORT_NSIS_FUNCTION(GetLicenseProduct)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    char *key = (char *)LocalAlloc(LPTR, g_stringsize+1);
    popstring(key);

    char lcnsprod[512];
    lcnsprod[0] = '\0';

    KeyDecoder kdec(key);
    if (kdec.IsValid()) {
        uint qtschema = kdec.getLicenseSchema();
        if(qtschema & KeyDecoder::SupportedEvaluation)
            strcpy(lcnsprod, "Evaluation");
        else if(qtschema & KeyDecoder::UnsupportedEvaluation)
            strcpy(lcnsprod, "Evaluation");
        else if(qtschema & KeyDecoder::FullSourceEvaluation)
            strcpy(lcnsprod, "Evaluation");
        else if(qtschema & KeyDecoder::FullCommercial)
            strcpy(lcnsprod, "Commercial");

        uint qtproduct = kdec.getProducts();
        if (qtproduct & KeyDecoder::QtUniversal)
            strcat(lcnsprod, "Universal");
        else if(qtproduct & KeyDecoder::QtDesktop)
            strcat(lcnsprod, "Desktop");
        else if(qtproduct & KeyDecoder::QtDesktopLight)
            strcat(lcnsprod, "DesktopLight");
    }

    LocalFree(key);
    pushstring(lcnsprod);
}

EXPORT_NSIS_FUNCTION(UsesUSLicense)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    char isUSCustomer[2];
    char *key = (char *)LocalAlloc(LPTR, g_stringsize+1);
    popstring(key);

    KeyDecoder kdec(key);
    if (kdec.IsValid() 
        && (kdec.getLicenseFeatures() & KeyDecoder::USCustomer))
        strcpy(isUSCustomer, "1");
    else
        strcpy(isUSCustomer, "0");

    LocalFree(key);
    pushstring(isUSCustomer);
}

EXPORT_NSIS_FUNCTION(GetLicenseInfo)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    LicenseFinder f;
    pushstring(f.getLicenseKey());
    pushstring(f.getLicensee());
}

EXPORT_NSIS_FUNCTION(PatchVC6Binary)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();
    
    char *fileName = (char *)LocalAlloc(LPTR, g_stringsize+1);
    char *oldStr = (char *)LocalAlloc(LPTR, g_stringsize+1);
    char *newStr = (char *)LocalAlloc(LPTR, g_stringsize+1);

    popstring(fileName);
    popstring(oldStr);
    popstring(newStr);

    BinPatch binFile(fileName);
    binFile.enableInsertReplace(true);
    binFile.enableUseLength(true);
    binFile.setEndTokens(".cpp;.h;.moc;.pdb");
    binFile.patch(oldStr, newStr);

    LocalFree(newStr);
    LocalFree(oldStr);
    LocalFree(fileName);
}

EXPORT_NSIS_FUNCTION(PatchVC7Binary)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();
    
    char *fileName = (char *)LocalAlloc(LPTR, g_stringsize+1);
    char *oldStr = (char *)LocalAlloc(LPTR, g_stringsize+1);
    char *newStr = (char *)LocalAlloc(LPTR, g_stringsize+1);

    popstring(fileName);
    popstring(oldStr);
    popstring(newStr);

    BinPatch binFile(fileName);
    binFile.enableInsertReplace(true);
    binFile.setEndTokens(".cpp;.h;.moc;.pdb");
    binFile.patch(oldStr, newStr);

    LocalFree(newStr);
    LocalFree(oldStr);
    LocalFree(fileName);
}

EXPORT_NSIS_FUNCTION(PatchBinary)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();
    
    char *fileName = (char *)LocalAlloc(LPTR, g_stringsize+1);
    char *oldStr = (char *)LocalAlloc(LPTR, g_stringsize+1);
    char *newStr = (char *)LocalAlloc(LPTR, g_stringsize+1);

    popstring(fileName);
    popstring(oldStr);
    popstring(newStr);

    BinPatch binFile(fileName);
    binFile.patch(oldStr, newStr);

    LocalFree(newStr);
    LocalFree(oldStr);
    LocalFree(fileName);
}
