#ifndef QFUNCTIONS_WCE_H
#define QFUNCTIONS_WCE_H

#ifdef Q_OS_TEMP

#ifndef QT_H
#endif // QT_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <winbase.h>
#include <objbase.h>
#include <kfuncs.h>
#include <ctype.h>


#define POCKET_PC

#define SetWindowLongA		SetWindowLong
#define GetWindowLongA		GetWindowLong
#define SendMessageA		SendMessage


#define PATH_MAX	1024


#ifndef POCKET_PC
int isprint( int c );
int isdigit( int c );
int isxdigit( int c );
int isspace( int c );
#else
#define SM_CXCURSOR             13
#define SM_CYCURSOR             14
#endif


typedef struct _DROPFILES
{ 
    DWORD pFiles; 
    POINT pt; 
    BOOL fNC; 
    BOOL fWide; 
} DROPFILES, FAR *LPDROPFILES; 


struct _stat
{
	int	st_mode;
	int	st_size;
	int st_mtime;
	int st_atime;
	int st_nlink;
};


#define _O_RDONLY		0x0001
#define _O_RDWR			0x0002
#define _O_WRONLY		0x0004
#define _O_CREAT		0x0008
#define _O_TRUNC		0x0010
#define _O_APPEND		0x0020

#define _S_IFMT			0x0100
#define _S_IFDIR		0x0200
#define _S_IFREG		0x0400

#define EMFILE			0x0800
#define ENOSPC			0x1000


#ifndef POCKET_PC
char *strrchr( const char *string, int c );
double strtod( const char *nptr, char **endptr );
long strtol( const char *nptr, char **endptr, int base );
double atof( const char *string );
#endif


BOOL ChangeClipboardChain(
  HWND hWndRemove,  // handle to window to remove
  HWND hWndNewNext  // handle to next window
);

HWND SetClipboardViewer(
  HWND hWndNewViewer   // handle to clipboard viewer window
);


// ###
#define WM_CHANGECBCHAIN	1
#define WM_DRAWCLIPBOARD	2
#ifndef POCKET_PC
#define GHND				GMEM_MOVEABLE | GMEM_ZEROINIT
#endif

char *_getcwd( char *buffer, int maxlen );
char *_tgetcwd( TCHAR *buffer, int maxlen );
char *_getdcwd( int drive, char *buffer, int maxlen );
char *_tgetdcwd( int drive, TCHAR *buffer, int maxlen );
int _chdir( const char *dirname );
int _tchdir( const TCHAR *dirname );
int _mkdir( const char *dirname );
int _tmkdir( const TCHAR *dirname );
int _rmdir( const char *dirname );
int _trmdir( const TCHAR *dirname );
int _access( const char *path, int mode );
int _taccess( const TCHAR *path, int mode );
int rename( const char *oldname, const char *newname );
int _trename( const TCHAR *oldname, const TCHAR *newname );
int remove( const char *name );
int _tremove( const TCHAR *name );
int _open( const char *filename, int oflag, int pmode );
int _topen( const TCHAR *filename, int oflag, int pmode );
int _fstat( int handle, struct _stat *buffer );
int _stat( const char *path, struct _stat *buffer );
int _tstat( const TCHAR *path, struct _stat *buffer );
long _lseek( int handle, long offset, int origin );
int _read( int handle, void *buffer, unsigned int count );
int _write( int handle, const void *buffer, unsigned int count );
int _close( int handle );
int _qt_fileno( FILE *filehandle );
DWORD GetLogicalDrives(VOID);
int _getdrive( void );
FILE *fdopen(int handle, const char *mode);
char *getenv(const char *env);

#ifdef __cplusplus
extern "C" {
#endif
FILE *_fdopen(int handle, const char *mode);
void rewind( FILE *stream );
FILE *tmpfile( void );
extern int errno;
#ifdef __cplusplus
}
#endif

#define calloc	_calloc


BOOL ResizePalette( HPALETTE hpal, UINT nEntries );
COLORREF PALETTEINDEX( WORD wPaletteIndex );


void *bsearch( const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare ) ( const void *elem1, const void *elem2 ) );


typedef struct tagENUMLOGFONTEX {
  LOGFONT  elfLogFont;
  TCHAR  elfFullName[LF_FULLFACESIZE];
  TCHAR  elfStyle[LF_FACESIZE];
  TCHAR  elfScript[LF_FACESIZE];
} ENUMLOGFONTEX, *LPENUMLOGFONTEX;



#ifndef _TM_DEFINED
#define _TM_DEFINED
struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#endif // _TM_DEFINED


struct tm *gmtime( const time_t *timer );
size_t strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr );

BOOL SetWindowOrgEx(
  HDC hdc,        // handle to device context
  int X,          // new x-coordinate of window origin
  int Y,          // new y-coordinate of window origin
  LPPOINT lpPoint // original window origin
);

BOOL TextOut(
  HDC hdc,           // handle to DC
  int nXStart,       // x-coordinate of starting position
  int nYStart,       // y-coordinate of starting position
  LPCTSTR lpString,  // character string
  int cbString       // number of characters
);


// ### not the real values
#define STARTF_USESTDHANDLES	1
#define CREATE_NO_WINDOW		2
#define DETACHED_PROCESS		3
#define CF_HDROP				4

HANDLE GetCurrentProcess(void);
DWORD GetCurrentThreadId(void);
HANDLE GetCurrentThread(void);


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
///////////////// MFC compatibility functions ///////////////////
// This code has been copied from the MFC library source code  //
// and will need replacing. Some of this is not used also, and //
// needs removing					       //
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Missing typedefs
#ifndef _TIME_T_DEFINED
typedef unsigned long time_t;
#define _TIME_T_DEFINED 
#endif
typedef HANDLE  HDWP;
typedef HANDLE  HDROP;
typedef wchar_t _TUCHAR;
typedef LPVOID  LPPRINTER_DEFAULTS;
typedef LPVOID  LPCHOOSEFONT;
#if (_WIN32_WCE < 210)
typedef LPVOID  LPPAGESETUPDLG;
#endif // _WIN32_WCE < 210
typedef UINT    UWORD;

// Missing definitions: not necessary equal to their Win32 values 
// (the goal is to just have a clean compilation of MFC)
#define BS_USERBUTTON             BS_PUSHBUTTON
#define WS_THICKFRAME             WS_DLGFRAME
#define WS_MAXIMIZE               0
#define WS_MINIMIZE               0
#define WS_EX_CONTROLPARENT       0x00010000L
#define WS_EX_LEFTSCROLLBAR       0
#ifndef WS_EX_TOOLWINDOW
#define WS_EX_TOOLWINDOW          0
#endif
#define WS_EX_NOPARENTNOTIFY      0 
#define WM_ENTERIDLE              0x0121
#define WM_PRINT                  WM_PAINT
#define WM_NCCREATE               (0x0081)
#define WM_PARENTNOTIFY           0
#define WM_NCDESTROY              (WM_APP-1)
#ifndef SW_RESTORE
#define SW_RESTORE                (SW_SHOWNORMAL)
#endif
#define SW_NORMAL                 (SW_SHOWNORMAL)
#define SW_SHOWMINNOACTIVE	      (SW_HIDE)
#define MB_TYPEMASK               (0x0000000FL)
#define MB_ICONMASK               (0x000000F0L)
#define CTLCOLOR_SCROLLBAR        CTLCOLOR_EDIT
#define PSM_CANCELTOCLOSE         (WM_USER + 107)
#define ESB_ENABLE_BOTH           (0x0000)
#define RDW_INVALIDATE            (0x0001)
#define RDW_INTERNALPAINT         (0x0002)
#define RDW_ERASE                 (0x0004)
#define RDW_VALIDATE              (0x0008)
#define RDW_NOINTERNALPAINT       (0x0010)
#define RDW_NOERASE               (0x0020)
#define RDW_NOCHILDREN            (0x0040)
#define RDW_ALLCHILDREN           (0x0080)
#define RDW_UPDATENOW             (0x0100)
#define RDW_ERASENOW              (0x0200)
#define RDW_FRAME                 (0x0400)
#define RDW_NOFRAME               (0x0800)
#ifndef DCX_CACHE
#define DCX_CACHE                 (0x00000002L)
#endif
#define WAIT_OBJECT_0             0x00000000L
#define PRF_CHILDREN              0x00000010L
#define PRF_CLIENT                0x00000004L
#define HELP_HELPFILE             (0x0000L)
#define MSGF_MENU                 2
#define pshHelp                   0x040E
#define SM_DBCSENABLED            42 
#define MF_BITMAP                 0x00000004L
#define MF_DISABLED               0
#define FW_REGULAR                FW_NORMAL
#define MB_TASKMODAL              0
#define MB_SYSTEMMODAL            MB_APPLMODAL
#define PDERR_DNDMMISMATCH	      0x1009
#define PDERR_DEFAULTDIFFERENT    0x100C
#define IDB_HIST_SMALL_COLOR      8
#define IDB_HIST_LARGE_COLOR      9
#define DEFAULT_GUI_FONT          SYSTEM_FONT
#define SFGAO_LINK                0x00010000L   
#ifndef _MAX_FNAME
#define _MAX_FNAME                 64
#endif
#ifndef SWP_NOREDRAW
#define SWP_NOREDRAW               0
#endif
#ifndef SBS_SIZEBOX
#define SBS_SIZEBOX               0
#endif
#ifndef SBS_SIZEGRIP
#define SBS_SIZEGRIP              0
#endif
#define SC_SIZE                   (0xF000)
#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)
#define WSAGETSELECTERROR(lParam) HIWORD(lParam)
#define HWND_TOPMOST              ((HWND)-1)
#define HWND_NOTOPMOST 	          ((HWND)-2)
#define HCBT_CREATEWND            (3)
#define CC_SHOWHELP               0
#define PS_DOT                    2
#define PD_ALLPAGES               0
#define PD_USEDEVMODECOPIES       0
#define PD_NOSELECTION            0
#define PD_HIDEPRINTTOFILE        0
#define PD_NOPAGENUMS             0
#define CF_METAFILEPICT           3
#define CWP_ALL                   0x0000
#define CWP_SKIPINVISIBLE         0x0001
#define CWP_SKIPDISABLED          0x0002
#define CWP_SKIPTRANSPARENT       0x0004
#define MM_LOMETRIC               2
#define MM_HIMETRIC               3
#define MM_LOENGLISH              4
#define MM_HIENGLISH              5
#define MM_TWIPS                  6
#define MM_ISOTROPIC              7
#define MM_ANISOTROPIC            8
#define OLEUI_FALSE               0
#define OLEUI_SUCCESS             1     
#define OLEUI_OK                  1    
#define OLEUI_CANCEL              2     
#define KF_EXTENDED               0x0100
#define KF_DLGMODE                0x0800
#define KF_MENUMODE               0x1000
#define KF_ALTDOWN                0x2000
#define KF_REPEAT                 0x4000
#define KF_UP                     0x8000
#define IDB_STD_SMALL_MONO        2     
#define IDB_STD_LARGE_MONO        3     
#define IDB_VIEW_SMALL_MONO       6    
#define IDB_VIEW_LARGE_MONO       7  
#define SPI_GETWORKAREA           48
#define LBSELCHSTRING             TEXT("commdlg_LBSelChangedNotify")
#define SHAREVISTRING             TEXT("commdlg_ShareViolation")
#define FILEOKSTRING              TEXT("commdlg_FileNameOK")
#define COLOROKSTRING             TEXT("commdlg_ColorOK")
#define SETRGBSTRING              TEXT("commdlg_SetRGBColor")
#define HELPMSGSTRING             TEXT("commdlg_help")
#define FINDMSGSTRING             TEXT("commdlg_FindReplace")
#define DRAGLISTMSGSTRING         TEXT("commctrl_DragListMsg")

#define OFN_ENABLESIZING 0

#ifndef WM_SETCURSOR
	#define WM_SETCURSOR 0x0020
	#define IDC_ARROW           MAKEINTRESOURCE(32512)
	#define IDC_IBEAM           MAKEINTRESOURCE(32513)
	#define IDC_WAIT            MAKEINTRESOURCE(32514)
	#define IDC_CROSS           MAKEINTRESOURCE(32515)
	#define IDC_UPARROW         MAKEINTRESOURCE(32516)
	#define IDC_SIZE            MAKEINTRESOURCE(32646)
	#define IDC_ICON            MAKEINTRESOURCE(32512)
	#define IDC_SIZENWSE        MAKEINTRESOURCE(32642)
	#define IDC_SIZENESW        MAKEINTRESOURCE(32643)
	#define IDC_SIZEWE          MAKEINTRESOURCE(32644)
	#define IDC_SIZENS          MAKEINTRESOURCE(32645)
	#define IDC_SIZEALL         MAKEINTRESOURCE(32646)
	#define IDC_NO              MAKEINTRESOURCE(32648)
	#define IDC_APPSTARTING     MAKEINTRESOURCE(32650)
	#define IDC_HELP            MAKEINTRESOURCE(32651)
	#define IDC_HAND	    MAKEINTRESOURCE(32649)
#endif 

#if defined(_MIPS_)
extern "C" void _asm(char *, ...);
#endif

#define GMEM_MOVEABLE             LMEM_MOVEABLE
#define GMEM_FIXED                LMEM_FIXED
#define GMEM_ZEROINIT             LMEM_ZEROINIT
#define GMEM_INVALID_HANDLE       LMEM_INVALID_HANDLE
#define GMEM_LOCKCOUNT            LMEM_LOCKCOUNT
#define GPTR                      LPTR
#if (_WIN32_WCE < 300)
#define GMEM_SHARE                0
#endif // _WIN32_WCE

// WinCE: CESYSGEN prunes the following FRP defines, 
// and INTERNET_TRANSFER_TYPE_ASCII breaks in wininet.h
#undef FTP_TRANSFER_TYPE_ASCII
#define FTP_TRANSFER_TYPE_ASCII 0x00000001
#undef FTP_TRANSFER_TYPE_BINARY
#define FTP_TRANSFER_TYPE_BINARY 0x00000002

#define MM_TEXT 1
typedef DWORD OLE_COLOR;
#define WS_OVERLAPPEDWINDOW 0 

#ifndef MF_BITMAP
#define MF_BITMAP 0x00000004L
#endif

#ifndef WS_EX_CAPTIONOKBTN
#define WS_EX_CAPTIONOKBTN 0x80000000L
#endif

#ifndef WS_EX_NODRAG
#define WS_EX_NODRAG       0x40000000L
#endif

#define FR_DOWN                         0x00000001
#define FR_WHOLEWORD                    0x00000002
#define FR_MATCHCASE                    0x00000004
#define FR_FINDNEXT                     0x00000008
#define FR_REPLACE                      0x00000010
#define FR_REPLACEALL                   0x00000020
#define FR_DIALOGTERM                   0x00000040
#define FR_SHOWHELP                     0x00000080
#define FR_ENABLEHOOK                   0x00000100
#define FR_ENABLETEMPLATE               0x00000200
#define FR_NOUPDOWN                     0x00000400
#define FR_NOMATCHCASE                  0x00000800
#define FR_NOWHOLEWORD                  0x00001000
#define FR_ENABLETEMPLATEHANDLE         0x00002000
#define FR_HIDEUPDOWN                   0x00004000
#define FR_HIDEMATCHCASE                0x00008000
#define FR_HIDEWHOLEWORD                0x00010000
typedef UINT (APIENTRY *LPFRHOOKPROC) (HWND, UINT, WPARAM, LPARAM);

#ifndef POCKET_PC
HGLOBAL GlobalAlloc(UINT uFlags, DWORD dwBytes);
HGLOBAL GlobalFree(HGLOBAL hMem);
HGLOBAL GlobalReAlloc(HGLOBAL hMem, DWORD dwBytes, UINT uFlags);
DWORD   GlobalSize(HGLOBAL hMem);
LPVOID  GlobalLock(HGLOBAL hMem);
BOOL    GlobalUnlock(HGLOBAL hMem);
HGLOBAL GlobalHandle(LPCVOID pMem);
UINT    GlobalFlags(HGLOBAL hMem);
#endif

/*
typedef struct tagNCCALCSIZE_PARAMS {
    RECT       rgrc[3];
    PWINDOWPOS lppos;
} NCCALCSIZE_PARAMS, *LPNCCALCSIZE_PARAMS;


// The WinCE OS headers #defines the following, 
// but it interferes with MFC member functions.
#undef TrackPopupMenu
#undef DrawIcon
#undef SendDlgItemMessage
#undef SetDlgItemText
#undef GetDlgItemText
#undef LoadCursor

// ATLCONV.H support
#ifndef _ASSERTE
#define _ASSERTE ASSERT
#endif


typedef struct tagFINDREPLACEW 
{
   DWORD        lStructSize;        // size of this struct 0x20
   HWND         hwndOwner;          // handle to owner's window
   HINSTANCE    hInstance;          // instance handle of.EXE that
                                    //   contains cust. dlg. template
   DWORD        Flags;              // one or more of the FR_??
   LPWSTR       lpstrFindWhat;      // ptr. to search string
   LPWSTR       lpstrReplaceWith;   // ptr. to replace string
   WORD         wFindWhatLen;       // size of find buffer
   WORD         wReplaceWithLen;    // size of replace buffer
   LPARAM       lCustData;          // data passed to hook fn.
   LPFRHOOKPROC lpfnHook;           // ptr. to hook fn. or NULL
   LPCWSTR      lpTemplateName;     // custom template name
} FINDREPLACE, *LPFINDREPLACE;

#if defined(_WIN32_WCE_PSPC) && (_WIN32_WCE == 201)
	#define OFN_READONLY         0 
	#define OFN_ENABLEHOOK       0
	#define OFN_ENABLETEMPLATE   0
	#define OFN_ALLOWMULTISELECT 0
#endif // _WIN32_WCE_PSPC

#if defined(_WIN32_WCE_PSPC) && (_WIN32_WCE >= 300)
	#ifndef SHA_SHOWDONE
	#define SHA_SHOWDONE 0x0001
	#endif
	#ifndef SHA_HIDEDONE
	#define SHA_HIDEDONE 0x0002
	#endif
	#ifndef SHA_AUTODONE
	#define SHA_AUTODONE 0x0003
	#endif
	#ifndef MENU_HEIGHT
	#define MENU_HEIGHT  26
	#endif
	BOOL AFXAPI wce_IsSipUp();
#endif // _WIN32_WCE_PSPC

#if !defined(_WIN32_WCE_NO_WININET)
#include <wininet.h>
#endif

#define wce_T2CA(lpszSrc) \
	(lpszSrc ? wce_T2CAHelper((LPSTR)_alloca((_tcslen(lpszSrc)+1)*2), lpszSrc) : NULL)

#if defined(_WIN32_WCE_NO_OLE)
// WinCE: compensate for not including atlconv.h in afxconv.h
#ifndef _DEBUG
#define USES_CONVERSION int _convert; _convert
#else
#define USES_CONVERSION int _convert = 0;
#endif
#define A2CT(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		_convert = (lstrlenA(lpa)+1),\
		(LPCWSTR)AfxA2WHelper((LPWSTR) alloca(_convert*2), lpa, _convert)\
	)\
)
#endif // _WIN32_WCE_NO_OLE

// Missing headers
#include "prsht.h"    

// Missing string functions aliases
#ifndef _istlead
#define _istlead(ch) (FALSE)  
#endif
#ifndef _vsnwprintf
#define _vsnwprintf(a,b,c,d) vswprintf(a,c,d)
#endif
#define lstrcpyn     _tcsncpy

__inline wchar_t * __cdecl _wcsdec(const wchar_t * start, const wchar_t * current)
	{ return (wchar_t *) ( (start>=current) ? NULL : (current-1) ); }
__inline wchar_t * __cdecl _wcsinc(const wchar_t * _pc) { return (wchar_t *)(_pc+1); }
__inline size_t    __cdecl _tclen( const wchar_t *_cpc) { return 2; } // for UNICODE

long	GetMessageTime();
time_t	mktime(struct tm* );
struct tm *localtime(const time_t *);
char	*ctime(const time_t* );
time_t	time(time_t *);
HINSTANCE GetModuleHandleW(LPCWSTR lpModuleName);
HICON   ExtractIcon(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex);
int     MulDiv(int nNumber, int nNumerator, int nDenominator);
HINSTANCE GetModuleHandleA(LPCSTR lpModuleName);
short   GetFileTitle(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf);

DWORD   GetVersion();
void*   calloc(size_t num, size_t size);
void*	_expand(void* pvMemBlock, size_t iSize);
void    abort();
unsigned long _beginthreadex(void *security, unsigned stack_size,
		unsigned (__stdcall *start_address)(void *), void *arglist, unsigned initflag, unsigned *thrdaddr);
void    _endthreadex(unsigned nExitCode);
BOOL    GetCursorPos(LPPOINT lpPoint);
HDWP    BeginDeferWindowPos(int nNumWindows);
HDWP    DeferWindowPos(HDWP hWinPosInfo, HWND hWnd, HWND hWndInsertAfter, 
                                        int x, int y, int cx, int cy, UINT uFlags);
BOOL    EndDeferWindowPos(HDWP hWinPosInfo);
BOOL    SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
FARPROC GetProcAddressA(HMODULE hModule, LPCSTR lpProcName);
HMODULE LoadLibraryA(LPCSTR lpLibFileName);
BOOL    WinHelp(HWND hWndMain, LPCTSTR lpszHelp, UINT uCommand, DWORD dwData); 
wchar_t*AsciiToWide(wchar_t* ws, const char* s);
wchar_t*AsciiToWide2(const char* s);
PSTR    T2CAHelper(LPSTR lpszDest, LPCTSTR lpszSrc);
BOOL    CheckEmulationLibs(HINSTANCE hInstance);
int     GetClipboardFormatNameA(UINT format, LPSTR lpszFormatName, int cchMaxCount);
void    WaitMessage();
BOOL    IsBadStringPtrA(LPCSTR lpsz, UINT ucchMax);
BOOL    IsBadStringPtrW(LPCWSTR lpsz, UINT ucchMax);
void    ReportDebugBreak(TCHAR *szFile, int nLine);
HPEN    CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
HBRUSH  CreateBrushIndirect(const LOGBRUSH* lplb);
HFONT   CreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline, BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision, BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily, LPCTSTR lpszFacename);
HRGN    CreateRectRgn(int x1, int y1, int x2, int y2);
BOOL    SetRectRgn(HRGN hrgn, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect);
BOOL    GetBrushOrgEx(HDC hdc, LPPOINT lppt);
int     OffsetClipRgn(HDC hdc, int nXOffset, int nYOffset);
int     ExtSelectClipRgn(HDC hdc, HRGN hrgn, int fnMode);
int     ExcludeUpdateRgn(HDC hDC, HWND hWnd);
BOOL    PolyPolyline(HDC hdc, const POINT* lppt, const DWORD* lpdwPolyPoints, DWORD cCount);
BOOL    PolyPolygon(HDC hdc, const POINT* lpPoints, const int* lpPolyCounts, int nCount);
int     FrameRect(HDC hdc, const RECT* lprc, HBRUSH hbr);
UINT    GetSystemDirectory(LPWSTR lpBuffer, UINT uSize);
UINT    GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize);
TCHAR*  GetNextArg(TCHAR* pszArgList, TCHAR szArg[]);
int     GetArgCount(TCHAR* pszArgList);
BOOL    ArgvW(int argc, char* argv[]);
DWORD   GetObjectStoreFreeSpace();
UINT    GetTempFileName(LPCTSTR lpPathName, LPCTSTR lpPrefixString, UINT uUnique, LPTSTR lpTempFileName);
DWORD   GetFullPathName(LPCTSTR lpFileName, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart);
short   GetFileTitleW(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf);
HMENU   GetMenu(HWND hWnd);
BOOL    SetMenu(HWND hWnd, HMENU hMenu);
UINT    GetMenuItemID(HMENU hMenu, int nPos);
BOOL    ModifyMenu(HMENU hMnu, UINT uPosition, UINT uFlags, UINT uIDNewItem, LPCTSTR lpNewItem);
int     GetMenuItemCount(HMENU hMenu);
int     GetMenuString(HMENU hMenu, UINT uIDItem, LPWSTR lpString, int nMaxCount, UINT uFlag);
UINT    GetMenuState(HMENU hMenu, UINT uId, UINT uFlags);
BOOL    InvalidateRgn(HWND hWnd, HRGN hRgn, BOOL bErase);
int     GetScrollPos(HWND hWnd, int nBar);
BOOL    GetScrollRange(HWND hWnd, int nBar, LPINT lpMinPos, LPINT lpMaxPos);
void    ScrollChildren(HWND hWnd, int cx, int cy);
BOOL    ShowScrollBar(HWND hWnd, int nBar, BOOL bShow);
BOOL    EnumChildWindows(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam);
int     GetSystemMetrics(int nIndex);
BOOL    RedrawWindow(HWND hWnd, CONST RECT *prcUpdate, HRGN hrgnUpdate, UINT afuRedraw);
BOOL	CALLBACK FirstDlgProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
LRESULT	CALLBACK NullWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
LRESULT	CALLBACK FirstDefWindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
BOOL    WriteProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString);
DWORD   GetProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize);
LONG    RegQueryValue(HKEY hKey, LPCWSTR lpSubKey, LPWSTR lpValue, PLONG   lpcbValue);
LONG    RegOpenKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult);
LONG    RegSetValue(HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData);
LONG    RegCreateKey(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
LONG    RegEnumKey(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cbName);
LONG    RegQueryValue(HKEY hKey, LPCWSTR lpSubKey, LPWSTR lpValue, PLONG   lpcbValue);
HWND	FindText(LPFINDREPLACE lpfr);
HWND	ReplaceText(LPFINDREPLACE lpfr);
UINT	WINAPI CoTaskMemSize(LPVOID);
LRESULT CALLBACK IMMWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
void    UnregisterOleWindowClasses();
HRESULT OleTranslateColor(OLE_COLOR clr, HPALETTE hpal, COLORREF* lpcolorref);
HRESULT OleCreateFontIndirect(void* lpFontDesc, REFIID riid, LPVOID* lplpvObj);
HRESULT RegGetMiscStatus(REFCLSID clsid, DWORD *pdwStatus);
BOOL    LockWindowUpdate(HWND hWnd); 
UINT    IsDlgButtonChecked(HWND hDlg, int nIDButton);
BOOL    SetDlgItemText(HWND hDlg, int nIDDlgItem, LPCWSTR lpString);
LONG    SendDlgItemMessage(HWND hDlg, int nIDDlgItem, UINT uMsg, WPARAM wParam, LPARAM lParam);
UINT    GetDlgItemText(HWND hDlg, int nIDDlgItem, LPWSTR lpString, int nMaxCount);
void    CheckDlgButton(HWND hDlg, int nIDButton, UINT uCheck);
BOOL    IsIconic(HWND hWnd);
HWND    GetTopWindow(HWND hWnd);
HWND    GetNextWindow(HWND hWnd, UINT nDirection);
HWND    GetDesktopWindow();
HWND    GetLastActivePopup(HWND hWnd);
BOOL    ScrollWindow(HWND hWnd, int xAmount, int yAmount, LPCRECT lpRect, LPCRECT lpClipRect);
BOOL    IsMenu(HMENU hMenu);
BOOL    TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT *prcRect);
BOOL    InvertRect(HDC hdc, const RECT* lprc);
int     GetMapMode(HDC hdc);
BOOL    GetViewportOrgEx(HDC hdc, LPPOINT lpPoint);
BOOL    GetViewportExtEx(HDC hdc, LPSIZE lpSize);
BOOL    GetWindowOrgEx(HDC hdc, LPPOINT lpPoint);
BOOL    GetWindowExtEx(HDC hdc, LPSIZE lpSize);
BOOL    DPtoLP(HDC hdc, LPPOINT lpPoints, int nCount);
BOOL    LPtoDP(HDC hdc, LPPOINT lpPoints, int nCount);
BOOL    DrawIcon(HDC hDC, int X, int Y, HICON hIcon);
HCURSOR LoadCursor(HINSTANCE hInstance,LPCWSTR lpCursorName);
HBITMAP CreateBitmapIndirect(LPBITMAP lpBitmap);

*/

void *_expand(void* pvMemBlock, size_t iSize);
void *calloc(size_t num, size_t size);
BOOL GetViewportOrgEx(HDC hdc, LPPOINT lpPoint);
BOOL GetViewportExtEx(HDC hdc, LPSIZE lpSize);
BOOL GetWindowOrgEx(HDC hdc, LPPOINT lpPoint);
BOOL GetWindowExtEx(HDC hdc, LPSIZE lpSize);
void _endthreadex(unsigned nExitCode);
unsigned long _beginthreadex(void *security, unsigned stack_size, 
		    unsigned (__stdcall *start_address)(void *),
		    void *arglist, unsigned initflag, unsigned *thrdaddr);

#endif // Q_OS_TEMP

#endif // QFUNCTIONS_WCE_H
