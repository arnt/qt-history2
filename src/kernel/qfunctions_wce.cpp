#ifdef Q_OS_TEMP

#include <stdio.h>

#include "qfunctions_wce.h"

#include <windows.h>
#include <winbase.h>
#include <kfuncs.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "winsock.lib")

char *getenv(char const *) { return "\\"; }
char *_getcwd( char *buffer, int maxlen ) { return "\\"; }
char *_tgetcwd( TCHAR *buffer, int maxlen ) { return "\\"; }
char *_getdcwd( int drive, char *buffer, int maxlen ) { return "\\"; }
char *_tgetdcwd( int drive, TCHAR *buffer, int maxlen ) { return "\\"; }
int _chdir( const char *dirname ) { return -1; }
int _tchdir( const TCHAR *dirname ) { return -1; }
int _mkdir( const char *dirname ) { return -1; }
int _tmkdir( const TCHAR *dirname ) { return -1; }
int _rmdir( const char *dirname ) { return -1; }
int _trmdir( const TCHAR *dirname ) { return -1; }
int _access( const char *path, int mode ) { return -1; }
int _taccess( const TCHAR *path, int mode ) { return -1; }
int rename( const char *oldname, const char *newname ) { return -1; }
int _trename( const TCHAR *oldname, const TCHAR *newname ) { return -1; }
int remove( const char *name ) { return -1; }
int _tremove( const TCHAR *name ) { return -1; }
int _topen( const TCHAR *filename, int oflag, int pmode ) { return -1; } // _open( filename, oflag, pmode ); }
int _fstat( int handle, struct _stat *buffer ) { return -1; }
int _stat( const char *path, struct _stat *buffer ) { return -1; }
int _tstat( const TCHAR *path, struct _stat *buffer ) { return -1; }

int _open( const char *filename, int oflag, int ) {
	char *flag;

	if (oflag & _O_APPEND) {
		if (oflag & _O_WRONLY) {
			flag = "a";
		} else if (oflag & _O_RDWR) {
			flag = "a+";
		}
	} else if (oflag & _O_WRONLY) {
		flag = "w";
	} else if (oflag & _O_RDWR) {
		flag = "w+"; // slightly different from "r+" where the file must exist
	} else if (oflag & _O_RDONLY) {
		flag = "r";
	} else {
		flag = "";
	}

	int retval = (int)fopen(filename, flag);

	// Return code for error should be -1 instead of NULL
	if (retval == NULL)
		return -1;
	else
		return retval;
}

long _lseek( int handle, long offset, int origin ) { return fseek((FILE*)handle, offset, origin); }
int _read( int handle, void *buffer, unsigned int count ) { return fread(buffer, 1, count, (FILE*)handle); }
int _write( int handle, const void *buffer, unsigned int count ) { return fwrite(buffer, 1, count, (FILE*)handle);; }
int _close( int handle ) { return fclose((FILE*)handle); }
int _qt_fileno( FILE *filehandle ) { return (int)filehandle; }
FILE *fdopen(int handle, const char *mode) { return (FILE*)handle; }

DWORD GetLogicalDrives(VOID) { return 1; }
int _getdrive( void ) { return 1; }

#ifdef __cplusplus
extern "C" {
#endif
int errno = 0;
void rewind( FILE *stream ) { fseek( stream, 0L, SEEK_SET ); }
FILE *tmpfile(void) { static int i = 0; char name[16]; sprintf(name, "tmp%i", i); i++; return fopen(name, "r+"); }
FILE *_fdopen(int handle, const char *mode) { return (FILE*)handle; }
#ifdef __cplusplus
}
#endif


BOOL ChangeClipboardChain( HWND hWndRemove, HWND hWndNewNext ) { return FALSE; }
HWND SetClipboardViewer( HWND hWndNewViewer ) { return NULL; }


#ifndef POCKET_PC

HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, 
					   LONG lInitialCount, LONG lMaximumCount, LPCTSTR lpName ) { return NULL; }
BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount ) { return TRUE; }

char *strrchr( const char *string, int c )
{
	int len = strlen( string );
	for (int i = len - 1; i >= 0; i--) {
		if ( (int)string[i] == c )
			return (char *)&(string[i]);
	}
	return NULL;
}

// This has been copied from libpng\pngrutil.c
double strtod( const char *nptr, char **endptr )
{
   double result = 0;
   int len;
   wchar_t *str, *end;

   len = MultiByteToWideChar(CP_ACP, 0, nptr, -1, NULL, 0);
   str = (wchar_t *)malloc(len * sizeof(wchar_t));
   if ( NULL != str )
   {
      MultiByteToWideChar(CP_ACP, 0, nptr, -1, str, len);
      result = wcstod(str, &end);
      len = WideCharToMultiByte(CP_ACP, 0, end, -1, NULL, 0, NULL, NULL);
      *endptr = (char *)nptr + (strlen(nptr) - len + 1);
      free(str);
   }
   return result;
}

// This is based on strtod from libpng\pngrutil.c
long strtol( const char *nptr, char **endptr, int base )
{
   long result = 0;
   int len;
   wchar_t *str, *end;

   len = MultiByteToWideChar(CP_ACP, 0, nptr, -1, NULL, 0);
   str = (wchar_t *)malloc(len * sizeof(wchar_t));
   if ( NULL != str )
   {
      MultiByteToWideChar(CP_ACP, 0, nptr, -1, str, len);
      result = wcstol(str, &end, base);
      len = WideCharToMultiByte(CP_ACP, 0, end, -1, NULL, 0, NULL, NULL);
      *endptr = (char *)nptr + (strlen(nptr) - len + 1);
      free(str);
   }
   return result;
}

double atof( const char *string )
{
	return 0.0;
}

// ### very rough approximation of these sets
// A proper implementation uses tables for these sets, but this will do for now
int isprint ( int c ) { return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || isdigit(c) || isspace(c)) ? 1 : 0; }
int isdigit ( int c ) { return (((c >= '1') && (c <= '9')) || (c == '0')) ? 1 : 0; }
int isxdigit( int c ) { return (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')) || isdigit(c)) ? 1 : 0; }
int isspace ( int c ) { return ((c == ' ') || (c == '\t')) ? 1 : 0; }

#endif


//#define	ASSERT(x)
//#define	VERIFY(x)	x
#define	TRACE0(x)
#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

BOOL SetWindowOrgEx( HDC hdc, int X, int Y, LPPOINT lpPoint ) {
	// SetViewportOrgEx( hdc, -X, -Y, lpPoint );
	// return SetViewportOrgEx( hdc, X - 30, Y - 30, lpPoint );
	// return ::SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy,	uFlags);
	return TRUE;
}

BOOL TextOut( HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString ) { 
    return ExtTextOut( hdc, nXStart, nYStart - 16, 0, NULL, lpString, cbString, NULL );
}

BOOL ResizePalette( HPALETTE hpal, UINT nEntries ) {
    return FALSE;
}

COLORREF PALETTEINDEX( WORD wPaletteIndex ) {
    return 0;
}

void *bsearch( const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare ) ( const void *elem1, const void *elem2 ) ) {
    return NULL;
}

struct tm *localtime( const time_t *timer ) {
    return NULL;
}

struct tm *gmtime( const time_t *timer ) {
    return NULL;
}

size_t strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr ) {
    return 0;
}

BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    return FALSE;
}
/*
HMENU GetMenu(HWND hWnd) {
    return NULL;
}

BOOL SetMenu(HWND hWnd, HMENU hMenu) {
    return FALSE;
}

BOOL CALLBACK FirstDlgProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hWnd,nMsg,wParam,lParam);
};

LRESULT CALLBACK FirstDefWindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hWnd,nMsg,wParam,lParam);
}

BOOL PreCreateWindow( CREATESTRUCT& cs) {
    return TRUE;
}

void PostCreateWindow( CREATESTRUCT& cs, HWND hWnd, HMENU nIDorHMenu) {
    // Set the menu (SetMenu just caches it away in CWnd)
    if((hWnd != NULL) && (HIWORD(nIDorHMenu) != NULL))
	SetMenu(hWnd, nIDorHMenu);
}
*/
HRGN CreateRectRgn(int x1, int y1, int x2, int y2) {
    RECT rect = { x1, y1, x2, y2 };
    return ::CreateRectRgnIndirect(&rect); 
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
///////////////// MFC compatibility functions ///////////////////
// This code has been copied from the MFC library source code  //
// and will need replacing. Much of this is not used also, and //
// needs removing                                              //
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

void*  calloc(size_t num, size_t size)
{
	void *ptr = malloc(num*size);
	if(ptr)
		memset(ptr, 0, num*size);
	return ptr;
}

void*  _expand(void* pvMemBlock, size_t iSize)
{
	return realloc(pvMemBlock, iSize);
}

extern "C" void  __cdecl exit(int);	   

void abort() {
    exit(3);																	 
}

unsigned long _beginthreadex(void *security, unsigned stack_size, 
		    unsigned (__stdcall *start_address)(void *),
		    void *arglist, unsigned initflag, unsigned *thrdaddr) {
    return (unsigned long)CreateThread((LPSECURITY_ATTRIBUTES)security, 
	(DWORD)stack_size, (LPTHREAD_START_ROUTINE)start_address, 
	(LPVOID)arglist, (DWORD)initflag | CREATE_SUSPENDED, (LPDWORD)thrdaddr);
}

void _endthreadex(unsigned nExitCode) {
	ExitThread((DWORD)nExitCode);
}


BOOL GetViewportOrgEx(HDC hdc, LPPOINT lpPoint)
{ 
    if (hdc == NULL)
	return FALSE; 
    lpPoint->x = 0;		// origin is always (0,0) 
    lpPoint->y = 0;
    return TRUE;
}

BOOL GetViewportExtEx(HDC hdc, LPSIZE lpSize)
{ 
    if (hdc == NULL)
        return FALSE; 
    lpSize->cx = 1;		// extent is always 1,1 
    lpSize->cy = 1;
    return TRUE;
}

BOOL GetWindowOrgEx(HDC hdc, LPPOINT lpPoint)
{ 
    if (hdc == NULL)
        return FALSE; 
    lpPoint->x = 0;		// origin is always (0,0) 
    lpPoint->y = 0;
    return TRUE;
}

BOOL GetWindowExtEx(HDC hdc, LPSIZE lpSize)
{ 
    if (hdc == NULL)
        return FALSE; 
    lpSize->cx = 1;		// extent is always 1,1 
    lpSize->cy = 1;
    return TRUE;
}

#endif // Q_OS_TEMP
