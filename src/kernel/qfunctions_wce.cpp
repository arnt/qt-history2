#include <stdio.h>

#include "qwinfunctions_wce.h"

#include <windows.h>
#include <winbase.h>
#include <kfuncs.h>

#pragma comment(lib, "commctrl.lib")
#pragma comment(lib, "commdlg.lib")
#pragma comment(lib, "coredll.lib")
#pragma comment(lib, "corelibc.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "winsock.lib")
#pragma comment(lib, "wininet.lib")


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
int _topen( const TCHAR *filename, int oflag, int pmode ) { return -1; }
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


BOOL SetWindowOrgEx(
  HDC hdc,        // handle to device context
  int X,          // new x-coordinate of window origin
  int Y,          // new y-coordinate of window origin
  LPPOINT lpPoint // original window origin
  )
{
	return TRUE; // SetViewportOrgEx( hdc, -X, -Y, lpPoint );
	// return SetViewportOrgEx( hdc, X - 30, Y - 30, lpPoint );
	// return ::SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy,	uFlags);
}


BOOL TextOut(
  HDC hdc,           // handle to DC
  int nXStart,       // x-coordinate of starting position
  int nYStart,       // y-coordinate of starting position
  LPCTSTR lpString,  // character string
  int cbString       // number of characters
  )
{ 
	return ExtTextOut( hdc, nXStart, nYStart - 16, 0, NULL, lpString, cbString, NULL );
}


BOOL ResizePalette( HPALETTE hpal, UINT nEntries )
{
	return FALSE;
}


COLORREF PALETTEINDEX( WORD wPaletteIndex )
{
	return 0;
}


void *bsearch( const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare ) ( const void *elem1, const void *elem2 ) )
{
	return NULL;
}


struct tm *localtime( const time_t *timer )
{
	return NULL;
}


struct tm *gmtime( const time_t *timer )
{
	return NULL;
}


size_t strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr )
{
	return 0;
}

BOOL  SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
{
	return FALSE;
}

HMENU  GetMenu(HWND hWnd)
{
	return NULL;
}

BOOL  SetMenu(HWND hWnd, HMENU hMenu)
{
	return FALSE;
}

BOOL CALLBACK FirstDlgProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd,nMsg,wParam,lParam);
};

LRESULT CALLBACK FirstDefWindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd,nMsg,wParam,lParam);
}

BOOL  PreCreateWindow( CREATESTRUCT& cs)
{
	return TRUE;
}

void  PostCreateWindow( CREATESTRUCT& cs, HWND hWnd, HMENU nIDorHMenu)
{
	// Set the menu (SetMenu just caches it away in CWnd)
	if((hWnd != NULL) && (HIWORD(nIDorHMenu) != NULL))
		SetMenu(hWnd, nIDorHMenu);
}

HRGN  CreateRectRgn(int x1, int y1, int x2, int y2)
{
	RECT rect = { x1, y1, x2, y2 };
	return ::CreateRectRgnIndirect(&rect); 
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
///////////////// MFC compatibility functions ///////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include <ctype.h>

wchar_t*  AsciiToWide(wchar_t* ws, const char* s)
{
	wchar_t* pszSave = ws;
    while(*s) 
	{
		*ws = (wchar_t) *s;
		s++;
		ws++;
	}
	*ws = 0;

	return pszSave;
}

wchar_t*  AsciiToWide(const char* s)
{
	TCHAR *ws = new TCHAR[strlen(s)+1];
	if(s == NULL)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	
	return AsciiToWide(ws, s);
}


int  wsprintfA(LPSTR szBuf, LPCSTR szFormat, UINT nArg);

// NOTE: Required by WCSTOD.OBJ in LIBC.LIB.
extern "C" int WINAPI wctomb(char *s, wchar_t wchar)
{
    *s = (char)wchar;
    return sizeof(char);
}

/*
LPSTR  wce_T2CAHelper(LPSTR lpszDest, LPCTSTR lpcszSrc)
{
	LPSTR lpszReturn = lpszDest;
	LPTSTR lpszSrc = (LPTSTR)lpcszSrc;
    if ((lpszSrc != NULL) && (lpszDest != NULL))
	{
		do {
			*lpszDest++ = (char) *lpszSrc;
		} while(*lpszSrc++);
	}
	return lpszReturn;
}
*/

HMODULE  LoadLibraryA(LPCSTR lpLibFileName)
{
	HMODULE hInst           = 0;
	wchar_t szDLL[MAX_PATH] = _T("");

    AsciiToWide(szDLL, lpLibFileName);
	hInst = LoadLibraryW(szDLL);

	return (HMODULE)hInst;
}											
/*
BOOL  wce_WinHelp(HWND hWndMain, LPCTSTR lpszHelpPath, UINT uCommand, DWORD dwData)
{
	TCHAR szHelpPathEx[MAX_PATH*2] = _T("");
	PROCESS_INFORMATION rProcInfo;

	ASSERT(lpszHelpPath != NULL);
	if (lpszHelpPath == NULL)
		return FALSE;

	_tcscpy(szHelpPathEx, _T("file:"));
	_tcscat(szHelpPathEx, lpszHelpPath);

	return CreateProcess(_T("PegHelp.exe"), (LPWSTR)szHelpPathEx,
		       NULL, NULL, FALSE, 0, NULL, NULL, NULL, &rProcInfo);
}

void  wce_WaitMessage()
{
	MSG msg;

	while(!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		Sleep(100);
}

// Note: we aren't handling variable arguments in this workaround.  The only 
// call we're servicing is the one found in afxtrace.cpp
int  wce_wsprintfA(LPSTR szBuf, LPCSTR szFormat, UINT nArg)
{
	TCHAR *szwBuf = AsciiToWide(szBuf);
	TCHAR *szwFormat = AsciiToWide(szFormat);
	int nRetVal = 0;

	if((szwBuf != NULL) && (szwBuf != szwFormat))
		nRetVal = wsprintf(szwBuf, szwFormat, nArg);

	delete [] szwBuf;
	delete [] szwFormat;
	return nRetVal;
}

int  wce_GetClipboardFormatNameA(UINT format, LPSTR lpszFormatName, int cchMaxCount)
{
	TCHAR *lpwszFormatName = AsciiToWide(lpszFormatName);
	int nRetVal = 0;

	if(lpwszFormatName != NULL)
		nRetVal = GetClipboardFormatName(format, lpwszFormatName, cchMaxCount); 

	delete [] lpwszFormatName;
	return nRetVal;
}

BOOL  wce_IsBadStringPtrA(LPCSTR lpsz, UINT ucchMax)
{
// WinCE: our workaround is to check to see if the first character 
// and last character are readable
	if(IsBadReadPtr(lpsz, 1))
		return FALSE;

	int nLen = (ucchMax == -1) ? strlen(lpsz) : 
		min((int)ucchMax, strlen(lpsz));
	return ::IsBadReadPtr(lpsz + nLen, 1);
}

BOOL  wce_IsBadStringPtrW(LPCWSTR lpsz, UINT ucchMax)
{
// WinCE: our workaround is to check to see if the first character 
// and last character are readable
	if(IsBadReadPtr(lpsz, 1))
		return FALSE;

	int nLen = (ucchMax == -1) ? wcslen(lpsz) : 
		min((int)ucchMax, wcslen(lpsz));
	return ::IsBadReadPtr(lpsz + nLen, 1);
}

#if defined(_WIN32_WCE_EMULATION)
extern "C" DWORD WINAPI GetCurrentDirectoryW(DWORD nBufferLength, LPTSTR lpBuffer); 
BOOL  wce_CheckEmulationLibs(HINSTANCE hInstance)
{
	TCHAR szCurrDir[MAX_PATH] = _T("");  
	TCHAR szMsg[256];
	HANDLE hLoadLibraryInstance;

	// Is MFC Dll in the emulation directory on the hard disk?
	if(!::GetModuleFileName(hInstance, szCurrDir, _MAX_PATH))
	{	
		VERIFY(::GetCurrentDirectoryW(MAX_PATH, szCurrDir));  // NOTE: Previously set by emulation's COREDLL.DLL.
		_stprintf(szMsg, _T("Initialization of the MFC application failed.  ")
						  _T("Is the EXE or DLL in the correct emulation directory?\n(e.g. %s)"),
						  szCurrDir);
		::MessageBox(NULL, szMsg, _T("Error"), MB_ICONEXCLAMATION);
		return FALSE;
	}

	// Is the MFC Dll in the emulation directory on the hard disk?
	TCHAR* szSlash = wcsrchr(szCurrDir,'\\');
	if((hLoadLibraryInstance = LoadLibrary((szSlash == NULL)?szCurrDir:szSlash+1)) == NULL)
	{
		_stprintf(szMsg, _T("Initialization of the MFC application failed.  ")
						  _T("The MFC Dll is not in the emulation object store.\n(e.g. %s)"),
						  szCurrDir);
		::MessageBox(NULL, szMsg, _T("Error"), MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}
#endif // _WIN32_WCE_EMULATION

void  wce_ReportDebugBreak(TCHAR* szFile, int nLine)
{
	TCHAR szBuf[100];
	wsprintf(szBuf,_T("Breakpoint at %s, line %d"),szFile,nLine);
	::MessageBox(NULL,szBuf,NULL,MB_OK);
}

UINT  wce_GetMenuItemID(HMENU hMenu, int nPos)
{	
	MENUITEMINFO mii;
	memset((char *)&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii); 
	mii.fMask  = MIIM_ID; 
	::GetMenuItemInfo(hMenu, nPos, TRUE, &mii);

	return mii.wID; 
}

UINT  wce_GetMenuState(HMENU hMenu, UINT uId, UINT uFlags)
{
	MENUITEMINFO mii;

	memset((char *)&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask  = MIIM_ID;

	if (uFlags & MF_BYPOSITION)
		::GetMenuItemInfo(hMenu, uId, TRUE, &mii);
	else
		::GetMenuItemInfo(hMenu, uId, FALSE, &mii);

	return mii.fState;
}

int  wce_GetMenuItemCount(HMENU hMenu);

BOOL  wce_ModifyMenu(HMENU   hMenu,      // handle of menu 
                     UINT    uPosition,  // menu item to modify 
                     UINT    uFlags,     // menu item flags 
                     UINT    uIDNewItem, // menu item identifier or handle of drop-down menu or submenu 
                     LPCTSTR lpNewItem) // menu item content 
{
	// Handle MF_BYCOMMAND case
	if ((uFlags & MF_BYPOSITION) != MF_BYPOSITION)
	{	
		int nMax = wce_GetMenuItemCount(hMenu);
		int nCount = 0;
		while (uPosition != wce_GetMenuItemID(hMenu, nCount) && (nCount < nMax))
			nCount++;
		ASSERT(nCount < nMax);
		uPosition = nCount;
		uFlags |= MF_BYPOSITION;
	}

	if (!::DeleteMenu(hMenu, uPosition, uFlags))
		return FALSE;

	return ::InsertMenu(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
}

int  wce_GetMenuItemCount(HMENU hMenu)
{
	const int MAX_NUM_ITEMS = 256;
	int  iPos, iCount;

	MENUITEMINFO mii;
	memset((char *)&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);

	iCount = 0;
	for (iPos = 0; iPos < MAX_NUM_ITEMS; iPos++)
	{
		if(!GetMenuItemInfo(hMenu, (UINT)iPos, TRUE, &mii))
			break;
		iCount++;
	}

	return iCount;
}

int  wce_GetMenuString(HMENU hMenu, UINT uIDItem, LPWSTR lpString, int nMaxCount, UINT uFlag)
{ 
	MENUITEMINFO mii;
	memset((char *)&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);

	if (!GetMenuItemInfo(hMenu, 0, TRUE, &mii))
		return 0;
	
	mii.fMask      = MIIM_TYPE;  // to get dwTypeData
	mii.fType      = MFT_STRING; // to get dwTypeData
	mii.dwTypeData = lpString;
	mii.cch        = nMaxCount;

	if (uFlag == MF_BYPOSITION)
		::GetMenuItemInfo(hMenu, uIDItem, TRUE, &mii);
	else
	{
		ASSERT(uFlag == MF_BYCOMMAND);
		::GetMenuItemInfo(hMenu, uIDItem, FALSE, &mii);
	}

	if (mii.dwTypeData != NULL)
		return _tcslen(lpString);

	return 0;
}

HPEN  wce_CreatePen(int nPenStyle, int nWidth, COLORREF crColor)
{ 
	LOGPEN logPen;
	
	if(nPenStyle == PS_DOT)
		nPenStyle = PS_DASH;
	logPen.lopnStyle   = nPenStyle;
	if(nWidth == 0)
		nWidth = 1;
	logPen.lopnWidth.x = nWidth;
	logPen.lopnWidth.y = 1;
	logPen.lopnColor   = crColor;

	return ::CreatePenIndirect(&logPen); 
}

HBRUSH  wce_CreateBrushIndirect(const LOGBRUSH* lplb)
{
	if (lplb->lbStyle == BS_SOLID)
		return ::CreateSolidBrush(lplb->lbColor);
	if (lplb->lbStyle == BS_NULL)	
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	if (lplb->lbStyle == BS_DIBPATTERNPT)
	{
		ASSERT(lplb->lbColor == DIB_RGB_COLORS);
		return ::CreateDIBPatternBrushPt((void*)lplb->lbHatch, lplb->lbColor);
	}
	TRACE0("LOGBRUSH style member is invalid");
	return NULL;
}

HFONT  wce_CreateFont(int nHeight, int nWidth, int nEscapement,
		int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
		BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
		BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
		LPCTSTR lpszFacename)
{
	LOGFONT logFont;
	logFont.lfHeight = nHeight;
	logFont.lfWidth = nWidth;
	logFont.lfEscapement = nEscapement;
	logFont.lfOrientation = nOrientation;
	logFont.lfWeight = nWeight;
	logFont.lfItalic = bItalic;
	logFont.lfUnderline = bUnderline;
	logFont.lfStrikeOut = cStrikeOut;
	logFont.lfCharSet = nCharSet;
	logFont.lfOutPrecision = nOutPrecision;
	logFont.lfClipPrecision = nClipPrecision;
	logFont.lfQuality = nQuality;
	logFont.lfPitchAndFamily = nPitchAndFamily;
	lstrcpyn(logFont.lfFaceName, lpszFacename, _countof(logFont.lfFaceName));

	return ::CreateFontIndirect(&logFont);
}

BOOL  wce_SetRectRgn(HRGN hrgn, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	BOOL bRetVal = TRUE;
	HRGN hrgnCopy;
	hrgnCopy = ::CreateRectRgn(nLeftRect, nTopRect, nRightRect, nBottomRect); 
	if (hrgnCopy == NULL)
		return FALSE;
	if (::CombineRgn(hrgn, hrgnCopy, NULL, RGN_COPY) == ERROR)
		bRetVal = FALSE;
	::DeleteObject(hrgnCopy);
	return bRetVal;
}

BOOL  wce_GetBrushOrgEx(HDC hdc, LPPOINT lppt)
{
	if (::SetBrushOrgEx(hdc, 0, 0, lppt) == FALSE)
		return FALSE;
	return ::SetBrushOrgEx(hdc, lppt->x, lppt->y, NULL);
}

int  wce_OffsetClipRgn(HDC hdc, int nXOffset, int nYOffset)
{
	int nRetVal;
	HRGN hrgn;
	hrgn = ::CreateRectRgn(0,0,1,1); 
	if (::GetClipRgn(hdc, hrgn) == -1)
		nRetVal = ERROR;
	else
	{
		if (::OffsetRgn(hrgn, nXOffset, nYOffset) == ERROR)
			nRetVal = ERROR;
		else
			nRetVal = ::SelectClipRgn(hdc,hrgn);	
	}
	::DeleteObject(hrgn);
	return nRetVal;
}

int  wce_ExtSelectClipRgn(HDC hdc, HRGN hrgn, int fnMode)
{
	ASSERT((hrgn != NULL) || (fnMode == RGN_COPY));
	int nRetVal;
	if (fnMode == RGN_COPY)
		nRetVal = ::SelectClipRgn(hdc, hrgn);	
	else
	{
		HRGN hrgnClip;
		hrgnClip = ::CreateRectRgn(0,0,1,1); 
		if (::GetClipRgn(hdc, hrgnClip) == -1)
			nRetVal = ERROR;
		else 
		{
			if (::CombineRgn(hrgnClip, hrgnClip, hrgn, fnMode) == ERROR)
				nRetVal = ERROR;
			else 
				nRetVal = ::SelectClipRgn(hdc, hrgnClip);
		}
		::DeleteObject(hrgnClip);
	}
	return nRetVal;
}

int  wce_ExcludeUpdateRgn(HDC hDC, HWND hWnd)
{
	int nRetVal;
	HRGN hrgn;
	hrgn = ::CreateRectRgn(0,0,1,1); 
	nRetVal = ::GetUpdateRgn(hWnd, hrgn, FALSE);
	if (nRetVal != ERROR)
	{
		if (::wce_ExtSelectClipRgn(hDC, hrgn, RGN_DIFF) == ERROR)
			nRetVal = ERROR;
	}
	::DeleteObject(hrgn);
	return nRetVal;
}

BOOL  wce_PolyPolyline(HDC hdc, const POINT* lppt, const DWORD* lpdwPolyPoints, DWORD cCount)
{
	if (cCount < 0)
		return FALSE;
	BOOL bResult = TRUE;
	int startIndex = 0;
	for (DWORD i=0; i < cCount && bResult; i++) 
	{
		ASSERT(lpdwPolyPoints[i] >= 2);
		bResult = ::Polyline(hdc, &(lppt[startIndex]), lpdwPolyPoints[i]);
		startIndex += lpdwPolyPoints[i]; // jump to next polyline
	}
	return bResult;
}

BOOL  wce_PolyPolygon(HDC hdc, const POINT* lpPoints, const int* lpPolyCounts, int nCount)
{
	if (nCount < 2)
		return FALSE;
	BOOL bResult = TRUE;
	int startIndex = 0;
	for (int i=0; i < nCount && bResult; i++) 
	{
		ASSERT(lpPolyCounts[i] >= 2);
		bResult = ::Polygon(hdc, &(lpPoints[startIndex]), lpPolyCounts[i]);
		startIndex += lpPolyCounts[i]; // jump to next polygon
	}
	return bResult;
}

int  wce_FrameRect(HDC hdc, const RECT* lprc, HBRUSH hbr)
{
	// Fill a "line-size" rectangle for each edge of the frame, using hbr
	// Note that right/bottom borders not painted by FillRect (or FrameRect)
	RECT rectEdge;	
	if (::SetRect(&rectEdge, lprc->left, lprc->top, lprc->right, 
				  lprc->top + 1) == FALSE) // Top edge of frame
		return FALSE;
	if (::FillRect(hdc, &rectEdge, hbr) == FALSE)
		return FALSE;
	
	if (::SetRect(&rectEdge, lprc->right - 1, lprc->top, lprc->right, 
				  lprc->bottom) == FALSE) // Right edge of frame
		return FALSE;
	if (::FillRect(hdc, &rectEdge, hbr) == FALSE)
		return FALSE;
	if (::SetRect(&rectEdge, lprc->left, lprc->bottom - 1, lprc->right, 
				  lprc->bottom) == FALSE) // Bottom edge of frame
		return FALSE;
	if (::FillRect(hdc, &rectEdge, hbr) == FALSE)
		return FALSE;
	if (::SetRect(&rectEdge, lprc->left, lprc->top, lprc->left + 1, 
				  lprc->bottom) == FALSE) // Left edge of frame
		return FALSE;
	return ::FillRect(hdc, &rectEdge, hbr);
}

LONG  wce_RegCreateKey(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
	long lResult = 0;
	DWORD dwDisposition = 0;

	lResult = RegCreateKeyExW(hKey, lpSubKey, 0, NULL, NULL, KEY_ALL_ACCESS, NULL, 
		                      phkResult, &dwDisposition);
	return lResult;
}

LONG  wce_RegQueryValue(HKEY hKey, LPCWSTR lpRegValue, LPWSTR lpValue, PLONG lpcbValue)
{
	long  lResult     = 0;
	DWORD dwValueType = 0;

	lResult = RegQueryValueExW((HKEY)hKey, (LPTSTR)lpRegValue, (LPDWORD)NULL, 
		(LPDWORD)&dwValueType, (LPBYTE)lpValue, (LPDWORD)lpcbValue);
	return lResult;
}

LONG  wce_RegOpenKey(HKEY     hKey,	        // handle of open key 
                     LPCTSTR  lpSubKey,	    // address of name of subkey to open 
                     PHKEY  phkResult) 	// address of handle of open key 
{
	long lResult = 0;
	HANDLE hBuffer = NULL;

	lResult = RegOpenKeyExW(hKey, lpSubKey, 0, KEY_ALL_ACCESS, phkResult);
	if(lResult == ERROR_SUCCESS)
		return lResult;

	// Workaround:  the KEY_ALL_ACCESS above contains KEY_CREATE_SUB_KEY, except
	// that it's not supported in WinCE.  So if an error results, check to see if its
	// a value that's meant to be converted to a key (which is the behavior of the 
	// desktop RegOpenKey()).

	BYTE *lpBuffer;
	DWORD dwType;
	DWORD dwSize;
	long lPrevResult = lResult;
	
	lResult = RegQueryValueEx(hKey, lpSubKey, 0, &dwType, NULL, &dwSize);
	if(lResult != ERROR_SUCCESS)
		return lResult;

	ASSERT(dwSize > 0);
	hBuffer = LocalAlloc(LMEM_FIXED, dwSize);
	if(hBuffer == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	lpBuffer = (BYTE*)GlobalLock(hBuffer);
	lResult = RegQueryValueEx(hKey, lpSubKey, 0, &dwType, lpBuffer, &dwSize);
	if(lResult != ERROR_SUCCESS)
	{
		lResult = lPrevResult;
		goto wceROK_ERROR;
	}

	lResult = RegDeleteValue(hKey, lpSubKey);
	if(lResult != ERROR_SUCCESS)
	{
		lResult = lPrevResult;
		goto wceROK_ERROR;
	}

	DWORD dwDisposition;
	lResult = RegCreateKeyEx(hKey, lpSubKey, 0, NULL, NULL, KEY_ALL_ACCESS, NULL, 
		                     phkResult, &dwDisposition);
	if(lResult != ERROR_SUCCESS)
	{
		lResult = lPrevResult;
		goto wceROK_ERROR;
	}

	lResult = RegSetValueEx(*phkResult, lpSubKey, 0, dwType, lpBuffer, dwSize);
	if(lResult != ERROR_SUCCESS)
	{
		lResult = lPrevResult;
		goto wceROK_ERROR;
	}

wceROK_ERROR:
	if(hBuffer != NULL)
	{
		LocalLock(hBuffer);
		LocalFree(hBuffer);
	}

	return lResult;

}

// RegSetValueEx is used as a workaround  for RegSetValue, but is not behaviorally
// equivalent. If the specified key doesn't exist and it's not a value, RegSetValue 
// creates a key, and applies the value to it.  
// RegSetValueEx doesn't do this, it rather creates another value under the specified
// prarent key.  Our override is to detect this circumstance, and creating a key
// manually before calling RegSetValueEx.
// Note: this workaround does not claim to be equivalent to RegSetValue.
LONG  wce_RegSetValue(HKEY hKey, LPCTSTR lpSubKey, DWORD dwType, LPCTSTR lpData,  
					 DWORD cbData)
{
	LONG lResult;
	HKEY hSubKey;

	// If lpSubKey is NULL or an empty string, just delegate to the Ex function
	if((lpSubKey == NULL) || (*lpSubKey == _T('\0')))
	{
		return RegSetValueEx(hKey, lpSubKey, 0, dwType,
			(CONST BYTE*)lpData, (cbData + 1) * sizeof(TCHAR));
	}

	// NOTE: in the following, we do not want to pass in KEY_CREATE_SUB_KEY (nevermind the
	// fact that that particular flag isn't supported in WinCE 2.0).  We just want to do a 
	// test and don't want an automatic conversion from a value to a key.
	lResult = RegOpenKeyEx(hKey, lpSubKey, 0, 0, &hSubKey);
	if(lResult != ERROR_SUCCESS) 
	{	// it's not a key, so it's either a value or emptiness. So that means another test. 
		DWORD dwType,dwSize;
		lResult = RegQueryValueEx(hKey, lpSubKey, 0, &dwType, NULL, &dwSize);
		if(lResult == ERROR_SUCCESS) // it's a value
		{
			// We can call RegSetValueEx now because we want to keep it as a value
			return RegSetValueEx(hKey, lpSubKey, 0, dwType,
				(CONST BYTE*)lpData, (cbData + 1) * sizeof(TCHAR));
		}
		else // We have emptiness... create a key
		{
			DWORD dwDisposition;
			lResult = RegCreateKeyEx(hKey, lpSubKey, 0, NULL, NULL, KEY_ALL_ACCESS, NULL, 
									 &hSubKey, &dwDisposition);
			if(lResult != ERROR_SUCCESS)
				return lResult;
		}
	}

	// At this point, we have a hSubKey.  Set its default value.  
	// This can only be done with strings.
	ASSERT(dwType == REG_SZ);
	return RegSetValueEx(hSubKey, _T(""), 0, dwType,
		(CONST BYTE*)lpData, (cbData + 1) * sizeof(TCHAR));
}


LONG  wce_RegEnumKey(HKEY    hKey,	    // handle of key to query 
                     DWORD   dwIndex,	// index of subkey to query 
                     LPTSTR  lpName,	// address of buffer for subkey name  
                     DWORD   cbName)	// size of subkey buffer 
{
	long      lResult;
	FILETIME  rFileTime;

	lResult = RegEnumKeyExW(hKey, dwIndex, lpName, &cbName, NULL, NULL,
                            NULL, &rFileTime);
	return lResult;
}


// Returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName,
// creating it if it doesn't exist.
// Responsibility of the caller to call RegCloseKey() on the returned HKEY.
// INI strings are not localized.
static const TCHAR szSoftware[]    = _T("\\Software");
static const TCHAR szRegistryKey[] = _T("Microsoft\\MFC_WCE");
static const TCHAR szProfileName[] = _T("afx.ini");
HKEY  wce_GetAppRegistryKey()
{
	HKEY hAppKey     = NULL;
	HKEY hSoftKey    = NULL;
	HKEY hCompanyKey = NULL;
	long lResult     = 0;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, szSoftware, 0, KEY_ALL_ACCESS, &hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, szRegistryKey, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			long lResult = RegCreateKeyEx(hCompanyKey, szProfileName, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
				&hAppKey, &dw);
			ASSERT(lResult == ERROR_SUCCESS);
		}
		else
			ASSERT(FALSE);
	}
	if (hSoftKey != NULL)
	{
		lResult = RegCloseKey(hSoftKey);
		ASSERT(lResult == ERROR_SUCCESS);
	}
	if (hCompanyKey != NULL)
	{
		lResult = RegCloseKey(hCompanyKey);
		ASSERT(lResult == ERROR_SUCCESS);
	}

	return hAppKey;
}


// Returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// Responsibility of the caller to call RegCloseKey() on the returned HKEY.
HKEY  wce_GetSectionKey(LPCTSTR lpszSection)
{
	long lResult = 0;

	ASSERT(lpszSection != NULL);

	HKEY hSectionKey = NULL;
	HKEY hAppKey = wce_GetAppRegistryKey();
	if (hAppKey == NULL)
		return NULL;

	DWORD dw;
	lResult = RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	ASSERT(lResult == ERROR_SUCCESS);

	lResult = RegCloseKey(hAppKey);
	ASSERT(lResult == ERROR_SUCCESS);

	return hSectionKey;
}



UINT  wce_GetSystemDirectory(LPWSTR lpBuffer, UINT uSize)
{ 
	wcscpy(lpBuffer, _T("\\WINDOWS")); 
	return (UINT)wcslen(lpBuffer); 
}

UINT  wce_GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize)
{
        strcpy(lpBuffer,    "\\WINDOWS" );
        return (UINT)strlen(lpBuffer);
}

TCHAR*  wce_GetNextArg(TCHAR* pszArgList, TCHAR szArg[])
{
	TCHAR*  pszSpace     = NULL;
	TCHAR*  pszTab       = NULL;
	TCHAR*  pszQuote     = NULL;
	TCHAR*  pszNextArg   = NULL;
	int     iWhiteCount  = 0;

	if ((pszArgList == NULL) || (lstrlen(pszArgList) == 0))
		return NULL;

	// count white space
	iWhiteCount = 0;
	pszSpace    = &pszArgList[0];
	while ((*pszSpace++ == (short)' ') || (*pszSpace++ == (short)'\t'))
		iWhiteCount++;

	// copy next arg to szArg
	lstrcpy(szArg, &pszArgList[iWhiteCount]);

	// null-terminate it
	// point pszNextArg at next arg
	pszQuote = szArg;		
	if (*szArg == (short)'"')
	{
		pszSpace = _tcschr(szArg+sizeof(TCHAR), (int)'"');
		if (pszSpace != NULL)
			pszQuote = pszSpace;
	}
	pszSpace = _tcschr(pszQuote, (int)' ');
	pszTab   = _tcschr(pszQuote, (int)'\t');
	if ((pszSpace == NULL) && (pszTab != NULL))
	{
		pszSpace = pszTab;
	}
	else if ((pszSpace != NULL) && (pszTab != NULL))
	{
		if (pszTab < pszSpace)
			pszSpace = pszTab;
	}
	if (pszSpace)
	{
		*pszSpace = 0;
		pszNextArg = pszSpace + 1;
	}
	else
		pszNextArg = &szArg[lstrlen(szArg)+1];

	// Get rid of quote around strings if there is any
	if (szArg[lstrlen(szArg)-1] == (short)'"')
		szArg[lstrlen(szArg)-1] = 0;
	if (*szArg == (short)'"')
		while (*szArg) 
			*szArg++ = *(szArg+1);

	// return pointer to beginning of next arg in list
	return pszNextArg;
}


int  wce_GetArgCount(TCHAR* pszArgList)
{

	TCHAR*  pszSpace     = NULL;
	TCHAR*  pszTab       = NULL;
	TCHAR*  pszNextArg   = NULL;
	int     iWhiteCount  = 0;
	int     iArgCount    = 0;

	if ((pszArgList == NULL) || (lstrlen(pszArgList) == 0))
		return 0;

	iArgCount  = 0;
	pszNextArg = pszArgList;
	while (pszNextArg != NULL)
	{
		// gobble/count white space
		iWhiteCount = 0;
		pszSpace    = pszNextArg;
		while ((*pszSpace++ == (short)' ') || (*pszSpace++ == (short)'\t'))
			iWhiteCount++;
		// point pszNextArg past the white space
		pszNextArg = &pszNextArg[iWhiteCount];
		// if its not null, then increment the count
		if (pszNextArg != NULL)
			iArgCount++;
		// for next iteration, point pszNextArg at next arg
		if (*pszNextArg == (short)'"')
		{
			pszSpace = _tcschr(pszNextArg+sizeof(TCHAR), (int)'"');
			if (pszSpace != NULL)
				pszNextArg = pszSpace;
		}
		pszSpace = _tcschr(pszNextArg, (int)' ');
		pszTab   = _tcschr(pszNextArg, (int)'\t');
		if ((pszSpace == NULL) && (pszTab != NULL))
		{
			pszSpace = pszTab;
		}
		else if ((pszSpace != NULL) && (pszTab != NULL))
		{
			if (pszTab < pszSpace)
				pszSpace = pszTab;
		}
		if (pszSpace)
			pszNextArg = pszSpace + 1;
		else
			break;  // i.e. we're done
	}

	return iArgCount;
}

BOOL  wce_ArgvW(int argc, char* argv[])
{
	TCHAR szTmp[MAX_PATH*2] = _T("");

	if (!argv)
		return TRUE;

	for (int i = 0; i < argc; i++)
	{
		if (argv[i])
		{
			// Convert the char* to wchar_t*.
			AsciiToWide(szTmp, argv[i]);
			// Free the previous pointer.
			free(argv[i]);
			// Reset argv[i] to point at the wchar_t*.
			argv[i] = (char*)_tcsdup(szTmp);
			ASSERT(argv[i] != NULL);
			if (!argv[i])
				return FALSE;
		}
	}

	return TRUE;
}

short  wce_GetFileTitleW(LPCTSTR lpszFile,    // pointer to full path and filename for file
                         LPTSTR  lpszTitle,   // pointer to buffer that receives filename
                         WORD cbBuf)         // length of buffer
{
	TCHAR* pszSlashPlus1     = 0;
	int    iLen              = 0;

	if ((lpszFile == NULL) || (_tcslen(lpszFile)==0))
		return -1;

	// Find last backslash (if present).
	// Point at first char after last backslash.
	pszSlashPlus1 = _tcsrchr(lpszFile, (int)'\\');
	if (pszSlashPlus1 == NULL)
		pszSlashPlus1 = (TCHAR*)lpszFile;
	else
		pszSlashPlus1 = pszSlashPlus1 + 1;

	// Compute the length of new string and compare with cbBuf.
	iLen = _tcslen(pszSlashPlus1);
	if (iLen > (int)cbBuf)
		return -1;

	// Copy basename to lpszTitle.
	_tcscpy(lpszTitle, pszSlashPlus1);

	return 0;
}

DWORD  wce_GetFullPathName(LPCTSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR* lppFilePart)
{
//	ASSERT(AfxIsValidString(lpFileName));
	ASSERT(lppFilePart != NULL);

	int nLen = _tcslen(lpFileName);
//	ASSERT(AfxIsValidAddress(lpBuffer, nLen, TRUE));
	_tcscpy(lpBuffer, lpFileName);

	LPTSTR lpszSlash = _tcsrchr(lpBuffer, (int)'\\');
	if (lpszSlash == NULL)
		*lppFilePart = lpBuffer;
	else
		*lppFilePart = lpszSlash+1;

	return (DWORD)nLen;
}

BOOL  wce_GetScrollRange(HWND hWnd, int nBar, LPINT lpMinPos, LPINT lpMaxPos)
{ 
	SCROLLINFO si;

	memset( &si, 0, sizeof(SCROLLINFO) );
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_RANGE;

	if(!GetScrollInfo( hWnd, nBar, &si))
		return (FALSE);

	*lpMinPos = si.nMin;
	*lpMaxPos = si.nMax;

	return TRUE; 
}

int  wce_GetScrollPos( HWND hWnd, int nBar)
{ 
	BOOL       bIsOK = FALSE;
	SCROLLINFO si;

	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_POS;

	if(!GetScrollInfo(hWnd, nBar, &si))
		return 0;

	return si.nPos; 
}
*/

/*
void  ScrollChildren(HWND hWnd, int cx, int cy)
{
	// WinCE does not perform any scrolling if the window is
	// not visible.  This leaves child windows unscrolled.
	// To account for this oversight, the child windows are moved
	// directly instead.
	HWND hWndChild = ::GetWindow(hWnd, GW_CHILD);
	if (hWndChild != NULL)
	{
		for (; hWndChild != NULL;
			hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
		{
			RECT rect;
			::GetWindowRect(hWndChild, &rect);
			::ScreenToClient(hWnd,(POINT*)&rect.left);
			::SetWindowPos(hWndChild, NULL,	rect.left-cx, rect.top-cy,
				0, 0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
}
*/
/*
// Added the following workaround for InvalidateRgn.
BOOL  wce_InvalidateRgn(HWND hWnd, HRGN hRgn, BOOL bErase)
{
	BOOL bReturn;
	HDC hdc = ::GetDC(hWnd);
	if(hdc == NULL) // if no DC, just erase the whole thing
		bReturn = ::InvalidateRect(hWnd,NULL,bErase);
	else
	{
		// Using the DC, get the smallest rectangle that encloses the region.
		RECT rect;
		SelectObject(hdc,hRgn);
		::GetClipBox(hdc,&rect);
		bReturn = ::InvalidateRect(hWnd,&rect,bErase);
		::ReleaseDC(hWnd,hdc);
	}

	return bReturn;
}
*/
/*
BOOL  EnumChildWindows( HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam )
{
    if (hWndParent == NULL)
            return (FALSE);

    HWND hWndChild = ::GetWindow( hWndParent, GW_CHILD );
    for (; hWndChild != NULL; hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
    {
		lpEnumFunc( hWndChild, lParam );
        EnumChildWindows( hWndChild, lpEnumFunc, lParam );
    }

    return (TRUE);
}
*/
/*
BOOL  wce_RedrawWindow(HWND hWnd, CONST RECT *prcUpdate, HRGN hrgnUpdate, UINT afuRedraw)
{
    ::InvalidateRect(hWnd, prcUpdate, afuRedraw & RDW_ERASE ? TRUE : FALSE);
    if ((afuRedraw & RDW_ERASENOW) || (afuRedraw & RDW_UPDATENOW)) 
        ::UpdateWindow(hWnd);
    return TRUE;
}

BOOL  wce_ShowScrollBar(HWND hWnd, int nBar, BOOL bShow)
{
// WinCE: this workaround doesn't work with x86em
#if defined(_WIN32_WCE_EMULATION)
	return TRUE;
#else // _WIN32_WCE_EMULATION
	DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
	if(bShow)
		dwStyle |= ((nBar == SB_HORZ) ? WS_HSCROLL : WS_VSCROLL);
	else
		dwStyle &= ~((nBar == SB_HORZ) ? WS_HSCROLL : WS_VSCROLL);
	return (::SetWindowLong(hWnd, GWL_STYLE, dwStyle) != 0);
#endif // _WIN32_WCE_EMULATION
	return TRUE;
}

int  wce_GetSystemMetrics(int nIndex)
{
	if(nIndex == SM_DBCSENABLED)
		return TRUE;
	else
		return ::GetSystemMetrics(nIndex);
}

LRESULT  CALLBACK wce_NullWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	// This window procedure is here to eat all messages after a window
	// has been detached.  The reason for this is that the WM_DESTROY
	// message below is "close" to being the last message, but we treat
	// it as such.  After that, we no longer have a CWnd->hWnd attachment,
	// so we use this WNDPROC to eat up any remaining messages.
	return 0;
}

#if defined(_WIN32_WCE_PSPC) && (_WIN32_WCE >= 300)
BOOL  wce_IsSipUp()
{
	SIPINFO si;
	memset(&si, 0, sizeof(SIPINFO));
	si.cbSize = sizeof(SIPINFO);
	if (!SHSipInfo(SPI_GETSIPINFO, 0, &si, 0))
	{
		TRACE0("Warning: Can not get the SIP Info.");
		return FALSE;
	}
	else
	{
		return ((si.fdwFlags & SIPF_ON) != 0);
	}
}
#endif // _WIN32_WCE_PSPC


HINSTANCE  wce_GetModuleHandleW(LPCWSTR lpModuleName)
{
#if (_WIN32_WCE >= 210)
	return ::GetModuleHandleW(lpModuleName);
#else // _WIN32_WCE
	HINSTANCE hInst = ::LoadLibraryW(lpModuleName);
	if(hInst)
		FreeLibrary(hInst);
	return hInst;
#endif // _WIN32_WCE
}

HICON  wce_ExtractIcon(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex)
{
	return (HICON)::ExtractIconEx(lpszExeFileName, nIconIndex, NULL, NULL, 1);
}

int  wce_MulDiv(int nNumber, int nNumerator, int nDenominator)
{
	__int64 x;

	x = (__int64)nNumber * (__int64)nNumerator;
	x /= (__int64)nDenominator;

	return (int)x;
}

HINSTANCE  wce_GetModuleHandleA(LPCSTR lpModuleName)
{
	HINSTANCE hInst;

	TCHAR *szwBuf = AsciiToWide(lpModuleName);
	hInst = GetModuleHandleW(szwBuf);
	delete[] szwBuf;

	return hInst;
}

short  wce_GetFileTitle(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf)
{
	return wce_GetFileTitleW(lpszFile, lpszTitle, cbBuf);
}
*/

#ifndef POCKET_PC


HGLOBAL  wce_GlobalAlloc(UINT uFlags, DWORD dwBytes)
{
	UINT uLocalFlags = 0;

	if (uFlags & GMEM_ZEROINIT)
		uLocalFlags |= LMEM_ZEROINIT;

	if (uFlags & GMEM_FIXED)
		uLocalFlags |= LMEM_FIXED;

	if (uFlags & GMEM_MOVEABLE)
		uLocalFlags |= LMEM_MOVEABLE;


	return (HGLOBAL)::LocalAlloc(uLocalFlags, (UINT)dwBytes);
}

HGLOBAL  wce_GlobalFree(HGLOBAL hMem)
{
	return ::LocalFree((HLOCAL)hMem);
}
 
HGLOBAL  wce_GlobalReAlloc(HGLOBAL hMem, DWORD dwBytes, UINT uFlags)
{
	UINT uLocalFlags = 0;

	if (uFlags & GMEM_ZEROINIT)
		uLocalFlags |= LMEM_ZEROINIT;

	if (uFlags & GMEM_FIXED)
		uLocalFlags |= LMEM_FIXED;

	if (uFlags & GMEM_MOVEABLE)
		uLocalFlags |= LMEM_MOVEABLE;

	return (HGLOBAL)::LocalReAlloc((HLOCAL)hMem, (UINT)dwBytes, uLocalFlags);
}

DWORD  wce_GlobalSize(HGLOBAL hMem)
{
	return (DWORD)::LocalSize((HLOCAL)hMem);
}

LPVOID  wce_GlobalLock(HGLOBAL hMem)
{
	return LocalLock((HLOCAL)hMem);
}

BOOL  wce_GlobalUnlock(HGLOBAL hMem)
{
	return LocalUnlock((HLOCAL)hMem);
}

HGLOBAL  wce_GlobalHandle(LPCVOID pMem)
{
	return (HGLOBAL)LocalHandle(pMem);
}

UINT  wce_GlobalFlags(HGLOBAL hMem)
{
	return LocalFlags((HLOCAL)hMem);
}


#endif

/*

DWORD  wce_GetVersion()
{
	return 4;
}

*/

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

void  abort()
{
	exit(3);																	 
}

unsigned long  _beginthreadex(void *security, unsigned stack_size, 
                                  unsigned (__stdcall *start_address)(void *),
                                  void *arglist, unsigned initflag, 
                                  unsigned *thrdaddr)
{
	return (unsigned long)CreateThread((LPSECURITY_ATTRIBUTES)security, 
		                                (DWORD)stack_size,
		                                (LPTHREAD_START_ROUTINE)start_address, 
									    (LPVOID)arglist, 
									    (DWORD)initflag | CREATE_SUSPENDED, 
									    (LPDWORD)thrdaddr);
}

 void  _endthreadex(unsigned nExitCode)
{
	ExitThread((DWORD)nExitCode);
}

/*

BOOL  wce_GetCursorPos(LPPOINT lpPoint)
{
#if (_WIN32_WCE >= 210)
	return ::GetCursorPos(lpPoint);
#else // _WIN32_WCE
	return ::GetCaretPos(lpPoint);
#endif // _WIN32_WCE
}

HDWP  wce_BeginDeferWindowPos(int nNumWindows)
{
	return (HDWP)1; // a fake handle
}

HDWP  wce_DeferWindowPos(HDWP hWinPosInfo, HWND hWnd, HWND hWndInsertAfter, 
                        int x, int y, int cx, int cy, UINT uFlags)
{
	::SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy,	uFlags);
	return (HDWP)1; // a fake handle
}

BOOL  wce_EndDeferWindowPos(HDWP hWinPosInfo)
{	
	return TRUE;
}
*/

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


