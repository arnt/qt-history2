#include <windows.h>
#include <io.h>
#include "exdll.h"

#include <stdio.h>

HANDLE hChildStdoutWr, hChildStdoutRd;

#define BUFFER_SIZE 1024
#define VERSION_SIZE 30
#define WIN32_VERSION_STRING "__W32API_VERSION 3.2"

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

BOOL CreateChildProcess(char *command) 
{ 
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFOA siStartInfo;
    BOOL bFuncRetn = FALSE; 

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

    ZeroMemory( &siStartInfo, sizeof(STARTUPINFOA) );
    siStartInfo.cb = sizeof(STARTUPINFOA); 
    siStartInfo.hStdError = hChildStdoutWr;
    siStartInfo.hStdOutput = hChildStdoutWr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bFuncRetn = CreateProcessA(NULL, 
        command,
        NULL,            // process security attributes 
        NULL,            // thread security attributes 
        TRUE,            // inherit handles
        CREATE_NO_WINDOW,
        NULL,            // use environment 
        NULL,            // use current directory 
        &siStartInfo, 
        &piProcInfo);

    if (bFuncRetn == 0) 
        return 0;
    else 
    {
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        return bFuncRetn;
    }
}

void getMinGWVersion(char *path, int *major, int *minor, int *patch)
{
    char command[BUFFER_SIZE];
    char instr[VERSION_SIZE];

    strcpy(command, path);
    strcat(command, "\\bin\\gcc.exe --version");

    SECURITY_ATTRIBUTES saAttr; 
 
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 

    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) 
        return;;

    if (CreateChildProcess(command) == 0)
        return;

    DWORD nBytes = 0;
    ReadFile(hChildStdoutRd, instr, VERSION_SIZE, &nBytes, NULL);
    instr[VERSION_SIZE-1] = '\0'; 

    char *gcc = strstr(instr, "(GCC)");
    if (gcc == NULL)
        return;

   sscanf(gcc, "(GCC) %d.%d.%d ", major, minor, patch);
}

bool hasValidIncludeFiles(char *path)
{
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    strcpy(filename, path);
    strcat(filename, "\\include\\w32api.h");

    FILE *finc;
    if ((finc = fopen(filename, "rb")) == NULL)
        return false;

    memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);
    fread(buffer, sizeof(char), BUFFER_SIZE-1, finc);

    if (strstr(buffer, WIN32_VERSION_STRING) != NULL)
        return true;

    return false;
}

EXPORT_NSIS_FUNCTION(HasValidWin32Library)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    char isValid[2];
    char *path = (char *)LocalAlloc(LPTR, g_stringsize+1);
    popstring(path);

    if (hasValidIncludeFiles(path))
        strcpy(isValid, "1");
    else
        strcpy(isValid, "0");

    LocalFree(path);
    pushstring(isValid);
}

EXPORT_NSIS_FUNCTION(GetMinGWVersion)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();

    char *path = (char *)LocalAlloc(LPTR, g_stringsize+1);
    popstring(path);

    int major = 0, minor = 0, patch = 0;
    char versionstr[BUFFER_SIZE];

    getMinGWVersion(path, &major, &minor, &patch);
    sprintf(versionstr, "%d.%d.%d", major, minor, patch);

    LocalFree(path);
    pushstring(versionstr);
}

bool shInEnvironment()
{
    char chpath[_MAX_PATH];
    char *env = getenv("PATH");
    char seps[] = ";";
    char *path;

    path = strtok(env, seps);
    while(path != NULL)
    {
        sprintf(chpath, "%s\\sh.exe", path);
        if(_access(chpath, 0) != -1)
            return true;

        path = strtok(NULL, seps);
    }

    return false;
}

EXPORT_NSIS_FUNCTION(ShInPath)
{
    g_hwndParent = hwndParent;
    EXDLL_INIT();
    char res[2];

    if (shInEnvironment)
        res[0] = '1';
    else
        res[0] = '0';

    res[1] = '\0';

    pushstring(res);
}
