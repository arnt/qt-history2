#include "qwindowsxpstyle.h"

#ifndef QT_NO_STYLE_WINDOWSXP

#define INCLUDE_MENUITEM_DEF
#include "qmenubar.h"
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtabbar.h>
#include <qheader.h>
#include <qspinbox.h>
#include <qgroupbox.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qscrollbar.h>
#include <qslider.h>

#if defined (Q_WS_WIN)
# include <qt_windows.h>
# include <qlibrary.h>

# define Q_DECLARE_THEME_FNC( x ) \
	static x##Fnc x;
# define Q_INIT_THEME_FNC( x )	\
	QWindowsXPStyle::Private::x##Fnc	QWindowsXPStyle::Private::x = NULL;

# if defined(Q_DEBUG)
#  define Q_LOAD_THEME_FNC( x )			\
	x = (x##Fnc)uxtheme->resolve( #x );	\
	if ( !x )				\
	    qDebug( "%s not defined in uxtheme.dll", #x );
# else
#  define Q_LOAD_THEME_FNC( x )			\
	x = (x##Fnc)uxtheme->resolve( #x );
# endif
# define Q_RECT					\
	RECT r;					\
	r.left = x;				\
	r.right = x+w;				\
	r.top = y;				\
	r.bottom = y+h;

#endif

class QWindowsXPStyle::Private
{
public:
    Private()
	: hotWidget( 0 ), hotSpot( -500, -500 ), hotTab( 0 )
    {
#if defined(Q_WS_WIN)
	if ( qWinVersion() == WV_XP && !init_xp ) {
	    init_xp = TRUE;
	    uxtheme = new QLibrary( "uxtheme" );
	    Q_LOAD_THEME_FNC( IsThemeActive )
	    Q_LOAD_THEME_FNC( SetWindowTheme )
	    Q_LOAD_THEME_FNC( IsThemeBackgroundPartiallyTransparent )
	    Q_LOAD_THEME_FNC( GetThemeSysSize )
	    Q_LOAD_THEME_FNC( CloseThemeData )
	    Q_LOAD_THEME_FNC( DrawThemeBackground )
	    Q_LOAD_THEME_FNC( DrawThemeBorder )
	    Q_LOAD_THEME_FNC( DrawThemeIcon )
	    Q_LOAD_THEME_FNC( DrawThemeLine )
	    Q_LOAD_THEME_FNC( DrawThemeText )
	    Q_LOAD_THEME_FNC( FormatThemeMessage )
	    Q_LOAD_THEME_FNC( GetThemeAppProperties )
	    Q_LOAD_THEME_FNC( GetThemeBackgroundContentRect )
	    Q_LOAD_THEME_FNC( GetThemeBackgroundExtent )
	    Q_LOAD_THEME_FNC( GetThemeBackgroundRegion )
	    Q_LOAD_THEME_FNC( GetThemeBool )
	    Q_LOAD_THEME_FNC( GetThemeColor )
	    Q_LOAD_THEME_FNC( GetThemeFilename )
	    Q_LOAD_THEME_FNC( GetThemeEnumValue )
	    Q_LOAD_THEME_FNC( GetThemeSysFont )
	    Q_LOAD_THEME_FNC( GetThemeFont )
	    Q_LOAD_THEME_FNC( GetThemeInt )
//		Q_LOAD_THEME_FNC( GetThemeIntList )		    Not found in beta1
	    Q_LOAD_THEME_FNC( GetThemeLastErrorContext )
	    Q_LOAD_THEME_FNC( GetThemeMargins )
	    Q_LOAD_THEME_FNC( GetThemeMetric )
	    Q_LOAD_THEME_FNC( GetThemePartSize )
	    Q_LOAD_THEME_FNC( GetThemePosition )
	    Q_LOAD_THEME_FNC( GetThemePropertyOrigin )
	    Q_LOAD_THEME_FNC( GetThemeRect )
	    Q_LOAD_THEME_FNC( GetThemeString )
	    Q_LOAD_THEME_FNC( GetThemeSysBool )
	    Q_LOAD_THEME_FNC( GetThemeSysColor )
	    Q_LOAD_THEME_FNC( GetThemeDocumentationProperty )
//		Q_LOAD_THEME_FNC( GetThemeSysColorBrush )	    Not found in beta1
	    Q_LOAD_THEME_FNC( GetThemeSysString )
	    Q_LOAD_THEME_FNC( GetThemeTextExtent )
	    Q_LOAD_THEME_FNC( GetThemeTextMetrics )
	    Q_LOAD_THEME_FNC( GetWindowTheme )
	    Q_LOAD_THEME_FNC( HitTestThemeBackground )
	    Q_LOAD_THEME_FNC( SetThemeAppProperties )
	    Q_LOAD_THEME_FNC( IsAppThemed )
//		Q_LOAD_THEME_FNC( SetWindowThemeStyle )		    Not found in beta1
	    Q_LOAD_THEME_FNC( IsThemePartDefined )
	    Q_LOAD_THEME_FNC( OpenThemeData )

	    limboWidget = new QWidget( 0, "xp_limbo_widget" );
	    hwnd = limboWidget->winId();

	    use_xp = IsThemeActive() && IsAppThemed();
	}
	if ( use_xp )
	    ref++;
#endif
    }
    ~Private()
    {
#if defined(Q_WS_WIN)
	if ( use_xp ) {
	    if ( !--ref ) {
		delete uxtheme;
		uxtheme = 0;
		init_xp = FALSE;
		use_xp  = FALSE;
		delete limboWidget;
		limboWidget = 0;
	    }
	}
#endif
    }

#if defined(Q_WS_WIN)
    static HTHEME getThemeData( TCHAR*c )
    {
	if ( !use_xp || !OpenThemeData )
	    return NULL;

	return OpenThemeData( hwnd, c );
    }

    static bool getThemeResult( HRESULT res ) 
    {
	if ( res == S_OK ) 
	    return TRUE;
	THEME_ERROR_CONTEXT myThemeErrorContext;
	HRESULT hRslt = GetThemeLastErrorContext(&myThemeErrorContext);
	if ( hRslt != S_OK ) {
	    qSystemWarning( "GetThemeLastErrorContext failed!", hRslt );
	} else {
	    FormatThemeMessage( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &myThemeErrorContext );
	}
	return FALSE;
    }

    #define MAX_INTLIST_COUNT 10

    typedef struct {
	int cxLeftWidth;
	int cxRightWidth;
	int cyTopHeight;
	int cyBottomHeight;
    } MARGINS;

    typedef struct {
	HRESULT hr;
	WCHAR szMsgParam1[_MAX_PATH];
	WCHAR szMsgParam2[_MAX_PATH];
	WCHAR szFileName[_MAX_PATH];
	WCHAR szSourceLine[_MAX_PATH];
	int iLineNum;
    } THEME_ERROR_CONTEXT;

    typedef struct {
	int iValueCount;
	int iValues[MAX_INTLIST_COUNT];
    } INTLIST;

    typedef enum {
	PO_STATE = 0,
	PO_PART = 1,
	PO_CLASS = 2,
	PO_GLOBAL = 3,
	PO_NOTFOUND = 4
    } PROPERTYORIGIN;

    typedef enum {
	STAP_ALLOW_NONCLIENT = 1,
	STAP_ALLOW_CONTROLS = 2,
	STAP_ALLOW_WEBCONTENT = 4
    } THEME_CONTROL_FLAGS;

    typedef enum {
	TMT_FLATMENUS = 800,
	TMT_DROPSHADOWS = 801,
	TMT_MOUSEVANISH = 802,
	TMT_CURSORSHADOW = 803,
	TMT_TOOLTIPFADE = 804,
	TMT_TOOLTIPANIMATION = 805,
	TMT_SELECTIONFADE = 806,
	TMT_ANIMATION = 807,
	TMT_LISTBOXSMOOTHSCROLL = 808,
	TMT_COMBOBOXANIMATION = 809,
	TMT_MENUANIMATION = 810,
	TMT_MENUFADE = 811,
	TMT_FIRSTBOOL = TMT_FLATMENUS,
	TMT_LASTBOOL = TMT_MENUFADE
    } THEME_METRICS_BOOLS;

    typedef enum {
	TMT_SCROLLBAR = 1600,
	TMT_BACKGROUND = 1601,
	TMT_ACTIVECAPTION = 1602,
	TMT_INACTIVECAPTION = 1603,
	TMT_MENU = 1604,
	TMT_WINDOW = 1605,
	TMT_WINDOWFRAME = 1606,
	TMT_MENUTEXT = 1607,
	TMT_WINDOWTEXT = 1608,
	TMT_CAPTIONTEXT = 1609,
	TMT_ACTIVEBORDER = 1610,
	TMT_INACTIVEBORDER = 1611,
	TMT_APPWORKSPACE = 1612,
	TMT_HIGHLIGHT = 1613,
	TMT_HIGHLIGHTTEXT = 1614,
	TMT_BTNFACE = 1615,
	TMT_BTNSHADOW = 1616,
	TMT_GRAYTEXT = 1617,
	TMT_BTNTEXT = 1618,
	TMT_INACTIVECAPTIONTEXT = 1619,
	TMT_BTNHIGHLIGHT = 1620,
	TMT_DKSHADOW3D = 1621,
	TMT_LIGHT3D = 1622,
	TMT_INFOTEXT = 1623,
	TMT_INFOBK = 1624,
	TMT_BUTTONALTERNATEFACE = 1625,
	TMT_HOTTRACKING = 1626,
	TMT_GRADIENTACTIVECAPTION = 1627,
	TMT_GRADIENTINACTIVECAPTION = 1628,
	TMT_MENUHILIGHT = 1629,
	TMT_MENUBAR = 1630,
	TMT_FIRSTCOLOR = TMT_SCROLLBAR,
	TMT_LASTCOLOR = TMT_MENUBAR
    } THEME_METRICS_COLORS;

    typedef enum {
	TMT_CAPTIONFONT = 800,
	TMT_SMALLCAPTIONFONT = 801,
	TMT_MENUFONT = 802,
	TMT_STATUSFONT = 803,
	TMT_MSGBOXFONT = 804,
	TMT_ICONTITLEFONT = 805,
	TMT_FIRSTFONT = TMT_CAPTIONFONT,
	TMT_LASTFONT = TMT_ICONTITLEFONT
    } THEME_METRICS_FONTS;

    typedef enum {
	TMT_BORDERWIDTH = 1200,
	TMT_SCROLLBARWIDTH = 1201,
	TMT_SCROLLBARHEIGHT = 1202,
	TMT_CAPTIONBARWIDTH = 1203,
	TMT_CAPTIONBARHEIGHT = 1204,
	TMT_SMCAPTIONBARWIDTH = 1205,
	TMT_SMCAPTIONBARHEIGHT = 1206,
	TMT_MENUBARWIDTH = 1207,
	TMT_MENUBARHEIGHT = 1208,
	TMT_FIRSTSIZE = TMT_BORDERWIDTH,
	TMT_LASTSIZE = TMT_MENUBARHEIGHT
    } THEME_METRICS_SIZES;

    typedef enum {
	TMT_CSSNAME = 1400,
	TMT_XMLNAME = 1401,
	TMT_FIRSTSTRING = TMT_CSSNAME,
    } THEME_METRICS_STRINGS;


    Q_DECLARE_HANDLE(HIMAGELIST);

    typedef HRESULT ( WINAPI *SetWindowThemeFnc)( 
	HWND hwnd, 
	LPCWSTR pstrSubAppName, 
	LPCWSTR pstrSubIdList 
    );
    typedef BOOL ( WINAPI *IsThemeBackgroundPartiallyTransparentFnc)( 
	HTHEME hTheme, 
	int iPartId, 
	int iStateId 
    );
    typedef INT ( WINAPI *GetThemeSysSizeFnc)( 
	HTHEME hTheme, 
	INT iSizeID 
    );
    typedef HRESULT ( WINAPI *CloseThemeDataFnc)( 
	HTHEME hTheme 
    );

    typedef HRESULT ( WINAPI *DrawThemeBackgroundFnc)(
	HTHEME hTheme,
	HDC hdc,
	int iPartId,
	int iStateId,
	const RECT *pRect,
	DWORD dwBgFlags
    );
    typedef HRESULT ( WINAPI *DrawThemeBorderFnc)(
	HTHEME hTheme,
	HDC hdc,
	int iStateId,
	const RECT *pRect
    );
    typedef HRESULT ( WINAPI *DrawThemeIconFnc)(
	HTHEME hTheme,
	HDC hdc,
	int iPartId,
	int iStateId,
	const RECT *pRect,
	HIMAGELIST himl,
	int iImageIndex
    );
    typedef HRESULT ( WINAPI *DrawThemeLineFnc)(
	HTHEME hTheme,
	HDC hdc,
	int iStateId,
	const RECT *pRect,
	DWORD dwDtlFlags
    );
    typedef HRESULT ( WINAPI *DrawThemeTextFnc)(
	HTHEME hTheme,
	HDC hdc,
	int iPartId,
	int iStateId,
	LPCWSTR pstrText,
	DWORD dwCharCount,
	DWORD dwTextFlags,
	DWORD dwTextFlags2,
	const RECT *pRect
    );
    typedef HRESULT ( WINAPI *FormatThemeMessageFnc)(
	DWORD dwLanguageID,
	THEME_ERROR_CONTEXT *pContext
    );
    typedef DWORD ( WINAPI *GetThemeAppPropertiesFnc)();
    typedef HRESULT ( WINAPI *GetThemeBackgroundContentRectFnc)(
	HTHEME hTheme,
	OPTIONAL HDC hdc,
	int iPartId,
	int iStateId,
	const RECT *pBoundingRect,
	RECT *pContentRect
    );
    typedef HRESULT ( WINAPI *GetThemeBackgroundExtentFnc)(
	HTHEME hTheme,
	OPTIONAL HDC hdc,
	int iPartId,
	int iStateId,
	const RECT *pContentRect,
	RECT *pExtentRect
    );
    typedef HRESULT ( WINAPI *GetThemeBackgroundRegionFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	const RECT *pRect,
	HRGN *pRegion
    );
    typedef HRESULT ( WINAPI *GetThemeBoolFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	BOOL *pfVal
    );

    typedef HRESULT ( WINAPI *GetThemeColorFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	COLORREF *pColor
    );
    typedef HRESULT ( WINAPI *GetThemeFilenameFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	LPWSTR *pszBuff,
	DWORD dwMaxBuffChars
    );
    typedef HRESULT ( WINAPI *GetThemeEnumValueFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	int *piVal
    );
    typedef HRESULT ( WINAPI *GetThemeSysFontFnc)(
	HTHEME hTheme,
	INT iFontID,
	LOGFONT *plf
    );
    typedef HRESULT ( WINAPI *GetThemeFontFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	LOGFONT *pFont
    );
    typedef HRESULT ( WINAPI *GetThemeIntFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	int *piVal
    );
    typedef HRESULT ( WINAPI *GetThemeIntListFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	INTLIST *pIntList
    );
    typedef HRESULT ( WINAPI *GetThemeLastErrorContextFnc)(
	THEME_ERROR_CONTEXT *pContext
    );
    typedef HRESULT ( WINAPI *GetThemeMarginsFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	MARGINS *pMargins
    );    
    typedef HRESULT ( WINAPI *GetThemeMetricFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	int *piVal
    );
    typedef HRESULT ( WINAPI *GetThemePartSizeFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	SIZE *psz
    );
    typedef HRESULT ( WINAPI *GetThemePositionFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	POINT *pPoint
    );
    typedef HRESULT ( WINAPI *GetThemePropertyOriginFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	PROPERTYORIGIN *pOrigin
    );
    typedef HRESULT ( WINAPI *GetThemeRectFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	RECT *pRect
    );
    typedef HRESULT ( WINAPI *GetThemeStringFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId,
	int iPropId,
	LPWSTR pstrBuff,
	DWORD dwMaxBuffChars
    );
    typedef BOOL ( WINAPI *GetThemeSysBoolFnc)(
	HTHEME hTheme,
	INT iBoolID
    );
    typedef COLORREF ( WINAPI *GetThemeSysColorFnc)(
	HTHEME hTheme,
	INT iColorID
    );
    typedef HRESULT ( WINAPI *GetThemeDocumentationPropertyFnc)(
	LPCWSTR pszThemeName,
	LPCWSTR pszPropertyName,
	LPWSTR pszValueBuff,
	DWORD dwMaxValChars
    );
    typedef BOOL ( WINAPI *IsThemeActiveFnc)(VOID);
    typedef HBRUSH ( WINAPI *GetThemeSysColorBrushFnc)(
	HTHEME hTheme,
	INT iColorID
    );
    typedef HRESULT ( WINAPI *GetThemeSysStringFnc)(
	HTHEME hTheme,
	INT iStringID,
	LPWSTR pszStringBuff,
	DWORD dwMaxStringChars
    );
    typedef HRESULT ( WINAPI *GetThemeTextExtentFnc)(
	HTHEME hTheme,
	HDC hdc,
	int iPartId,
	int iStateId,
	LPCWSTR pstrText,
	DWORD dwTextFlags,
	const RECT *pBoundingRect,
	RECT *pExtentRect
    );
    typedef HRESULT ( WINAPI *GetThemeTextMetricsFnc)(
	HTHEME hTheme,
	HDC hdc,
	int iPartId,
	int iStateId,
	TEXTMETRIC *ptm
    );
    typedef HTHEME ( WINAPI *GetWindowThemeFnc)(
	HWND hWnd
    );
    typedef HRESULT ( WINAPI *HitTestThemeBackgroundFnc)(
	HTHEME hTheme,
	OPTIONAL HDC hdc,
	int iPartId,
	int iStateId,
	const RECT *pRect,
	POINT ptTest,
	WORD *pwHitTestCode
    );
    typedef void ( WINAPI *SetThemeAppPropertiesFnc)(
	DWORD dwFlags
    );
    typedef BOOL ( WINAPI *IsAppThemedFnc)(VOID);
    typedef HRESULT ( WINAPI *SetWindowThemeStyleFnc)(
	HWND hwnd,
	BOOL fApply,
	BOOL fFrame
    );
    typedef BOOL ( WINAPI *IsThemePartDefinedFnc)(
	HTHEME hTheme,
	int iPartId,
	int iStateId
    ); 
    typedef HTHEME ( WINAPI *OpenThemeDataFnc)(
	HWND hwnd,
	LPCWSTR pstrClassList
    );

    Q_DECLARE_THEME_FNC( SetWindowTheme )
    Q_DECLARE_THEME_FNC( IsThemeBackgroundPartiallyTransparent )
    Q_DECLARE_THEME_FNC( GetThemeSysSize )
    Q_DECLARE_THEME_FNC( CloseThemeData )
    Q_DECLARE_THEME_FNC( DrawThemeBackground )
    Q_DECLARE_THEME_FNC( DrawThemeBorder )
    Q_DECLARE_THEME_FNC( DrawThemeIcon )
    Q_DECLARE_THEME_FNC( DrawThemeLine )
    Q_DECLARE_THEME_FNC( DrawThemeText )
    Q_DECLARE_THEME_FNC( FormatThemeMessage )
    Q_DECLARE_THEME_FNC( GetThemeAppProperties )
    Q_DECLARE_THEME_FNC( GetThemeBackgroundContentRect )
    Q_DECLARE_THEME_FNC( GetThemeBackgroundExtent )
    Q_DECLARE_THEME_FNC( GetThemeBackgroundRegion )
    Q_DECLARE_THEME_FNC( GetThemeBool )
    Q_DECLARE_THEME_FNC( GetThemeColor )
    Q_DECLARE_THEME_FNC( GetThemeFilename )
    Q_DECLARE_THEME_FNC( GetThemeEnumValue )
    Q_DECLARE_THEME_FNC( GetThemeSysFont )
    Q_DECLARE_THEME_FNC( GetThemeFont )
    Q_DECLARE_THEME_FNC( GetThemeInt )
    Q_DECLARE_THEME_FNC( GetThemeIntList )
    Q_DECLARE_THEME_FNC( GetThemeLastErrorContext )
    Q_DECLARE_THEME_FNC( GetThemeMargins )
    Q_DECLARE_THEME_FNC( GetThemeMetric )
    Q_DECLARE_THEME_FNC( GetThemePartSize )
    Q_DECLARE_THEME_FNC( GetThemePosition )
    Q_DECLARE_THEME_FNC( GetThemePropertyOrigin )
    Q_DECLARE_THEME_FNC( GetThemeRect )
    Q_DECLARE_THEME_FNC( GetThemeString )
    Q_DECLARE_THEME_FNC( GetThemeSysBool )
    Q_DECLARE_THEME_FNC( GetThemeSysColor )
    Q_DECLARE_THEME_FNC( GetThemeDocumentationProperty )
    Q_DECLARE_THEME_FNC( IsThemeActive )
    Q_DECLARE_THEME_FNC( GetThemeSysColorBrush )
    Q_DECLARE_THEME_FNC( GetThemeSysString )
    Q_DECLARE_THEME_FNC( GetThemeTextExtent )
    Q_DECLARE_THEME_FNC( GetThemeTextMetrics )
    Q_DECLARE_THEME_FNC( GetWindowTheme )
    Q_DECLARE_THEME_FNC( HitTestThemeBackground )
    Q_DECLARE_THEME_FNC( SetThemeAppProperties )
    Q_DECLARE_THEME_FNC( IsAppThemed )
    Q_DECLARE_THEME_FNC( SetWindowThemeStyle )
    Q_DECLARE_THEME_FNC( IsThemePartDefined )
    Q_DECLARE_THEME_FNC( OpenThemeData )

    /* Undocumented, reveiled by dumpbin /EXPORTS uxtheme.dll
    Q_DECLARE_THEME_FNC( ApplyTheme )
    Q_DECLARE_THEME_FNC( CloseThemeFile )
    Q_DECLARE_THEME_FNC( DrawNCPreview )
    Q_DECLARE_THEME_FNC( DumpLoadedThemeToTextFile )
    Q_DECLARE_THEME_FNC( EnumThemeColors )
    Q_DECLARE_THEME_FNC( EnumThemeSizes )
    Q_DECLARE_THEME_FNC( EnumThemes )
    Q_DECLARE_THEME_FNC( GetCurrentThemeName )
    Q_DECLARE_THEME_FNC( GetThemeDefaults )
    Q_DECLARE_THEME_FNC( OpenThemeDataFromFile )
    Q_DECLARE_THEME_FNC( OpenThemeFileFromData )
    Q_DECLARE_THEME_FNC( ParseThemeIniFile )
    Q_DECLARE_THEME_FNC( QueryThemeServices )
    Q_DECLARE_THEME_FNC( RegisterDefaultTheme )
    Q_DECLARE_THEME_FNC( ThemeInitApiHook )
    */

    static HWND hwnd;
#endif
    static bool use_xp;

    // hot-widget stuff
    QPoint hotSpot;

    QWidget *hotWidget;    
    QTab *hotTab;
    QRect hotHeader;
    QPalette oldPalette;

private:
    static QLibrary *uxtheme;
    static ulong ref;
    static bool init_xp;
    static QWidget *limboWidget;
};

QLibrary *QWindowsXPStyle::Private::uxtheme = NULL;
ulong QWindowsXPStyle::Private::ref = 0;
bool QWindowsXPStyle::Private::use_xp  = FALSE;
bool QWindowsXPStyle::Private::init_xp = FALSE;
QWidget *QWindowsXPStyle::Private::limboWidget = 0;

#if defined(Q_WS_WIN)
Q_INIT_THEME_FNC( SetWindowTheme )
Q_INIT_THEME_FNC( IsThemeBackgroundPartiallyTransparent )
Q_INIT_THEME_FNC( GetThemeSysSize )
Q_INIT_THEME_FNC( CloseThemeData )
Q_INIT_THEME_FNC( DrawThemeBackground )
Q_INIT_THEME_FNC( DrawThemeBorder )
Q_INIT_THEME_FNC( DrawThemeIcon )
Q_INIT_THEME_FNC( DrawThemeLine )
Q_INIT_THEME_FNC( DrawThemeText )
Q_INIT_THEME_FNC( FormatThemeMessage )
Q_INIT_THEME_FNC( GetThemeAppProperties )
Q_INIT_THEME_FNC( GetThemeBackgroundContentRect )
Q_INIT_THEME_FNC( GetThemeBackgroundExtent )
Q_INIT_THEME_FNC( GetThemeBackgroundRegion )
Q_INIT_THEME_FNC( GetThemeBool )
Q_INIT_THEME_FNC( GetThemeColor )
Q_INIT_THEME_FNC( GetThemeFilename )
Q_INIT_THEME_FNC( GetThemeEnumValue )
Q_INIT_THEME_FNC( GetThemeSysFont )
Q_INIT_THEME_FNC( GetThemeFont )
Q_INIT_THEME_FNC( GetThemeInt )
Q_INIT_THEME_FNC( GetThemeIntList )
Q_INIT_THEME_FNC( GetThemeLastErrorContext )
Q_INIT_THEME_FNC( GetThemeMargins )
Q_INIT_THEME_FNC( GetThemeMetric )
Q_INIT_THEME_FNC( GetThemePartSize )
Q_INIT_THEME_FNC( GetThemePosition )
Q_INIT_THEME_FNC( GetThemePropertyOrigin )
Q_INIT_THEME_FNC( GetThemeRect )
Q_INIT_THEME_FNC( GetThemeString )
Q_INIT_THEME_FNC( GetThemeSysBool )
Q_INIT_THEME_FNC( GetThemeSysColor )
Q_INIT_THEME_FNC( GetThemeDocumentationProperty )
Q_INIT_THEME_FNC( IsThemeActive )
Q_INIT_THEME_FNC( GetThemeSysColorBrush )
Q_INIT_THEME_FNC( GetThemeSysString )
Q_INIT_THEME_FNC( GetThemeTextExtent )
Q_INIT_THEME_FNC( GetThemeTextMetrics )
Q_INIT_THEME_FNC( GetWindowTheme )
Q_INIT_THEME_FNC( HitTestThemeBackground )
Q_INIT_THEME_FNC( SetThemeAppProperties )
Q_INIT_THEME_FNC( IsAppThemed )
Q_INIT_THEME_FNC( SetWindowThemeStyle )
Q_INIT_THEME_FNC( IsThemePartDefined )
Q_INIT_THEME_FNC( OpenThemeData )

HWND QWindowsXPStyle::Private::hwnd = NULL;
#endif

QWindowsXPStyle::QWindowsXPStyle()
: QWindowsStyle()
{
    d = new Private;
}

QWindowsXPStyle::~QWindowsXPStyle()
{
    delete d;
}

void QWindowsXPStyle::polish( QWidget *widget )
{
    if ( widget->inherits( "QButton" ) ) {
	widget->installEventFilter( this );
    } else if ( widget->inherits( "QTabBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QHeader" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QComboBox" ) ) {
	widget->installEventFilter( this );
    } else if ( widget->inherits( "QSpinBox" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    }
    QWindowsStyle::polish( widget );
}

void QWindowsXPStyle::unPolish( QWidget *widget )
{
    widget->removeEventFilter( this );
    QWindowsStyle::unPolish( widget );
}

// shapes
void QWindowsXPStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
                const QColorGroup &g, bool sunken, int lineWidth, const QBrush *fill )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TAB" );
    if ( !htheme ) {
	QWindowsStyle::drawPanel( p, x, y, w, h, g, sunken, lineWidth, fill );
	return;
    }

    Q_RECT

    Private::DrawThemeBackground( htheme, p->handle(), 9, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawPanel( p, x, y, w, h, g, sunken, lineWidth, fill );
#endif

#if 0 // strange place for a brute force algorithm...
    HWND hwnd = ((QWidget*)p->device())->winId();
    unsigned short* const bla = new unsigned short[40];
    const int start = 3;
    for ( int a = 0; a < 40; ++a )
	bla[a] = '\0';
    bla[0] = 'P';bla[1] = 'O';bla[2] = 'P';

    while( TRUE ) {
	
	for ( int i = 'A'; i <= 'Z'; i++ ) {
	    HTHEME htheme = Private::OpenThemeData( hwnd, bla );
	    if ( htheme ) {
		QString str = qt_winQString( bla );
		qDebug( "Theme for %s", str.latin1() );
		HRESULT res = Private::CloseThemeData( htheme );
		if ( res != S_OK )
		    qDebug( "Handle couldn't be closed" );
		return;
	    }
	    if ( bla[start] == 'Z' ) {
		int b = start;
		while ( b < 39 && bla[b] == 'Z' ) {
		    bla[b] = 'A';
		    b+=1;
		}
		if ( b < 39 ) {
		    if ( bla[b] == 0 )
			bla[b] = 'A';
		    else
			bla[b] += 1;
		} else {
		    qDebug( "Done!" );
		    return;
		}
	    } else {
		bla[start] += 1;
	    }
	}
    }
#endif
}


void QWindowsXPStyle::drawButton( QPainter *p, int x, int y, int w, int h,
                 const QColorGroup &g, bool sunken, const QBrush *fill )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawButton( p, x, y, w, h, g, sunken, fill );
	return;
    }

    Q_RECT

    if ( sunken )
	Private::DrawThemeBackground( htheme, p->handle(), 1, 2, &r, 0 );
    else
	Private::DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawButton( p, x, y, w, h, g, sunken, fill );
#endif
}

void QWindowsXPStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool sunken, const QBrush *fill )
{
    drawButton( p, x, y, w, h, g, sunken, fill );
}

void QWindowsXPStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool on, bool down, bool enabled,
		 bool autoRaised, const QBrush *fill )
 {
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TOOLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolButton( p, x, y, w, h, g, on, down, enabled, autoRaised, fill );
	return;
    }

    Q_RECT

    int statusId;
    if ( !enabled )
	statusId = 4;
    else if ( down )
	statusId = 3;
    else if ( !g.brightText().isValid() )
	statusId = 2;
    else if ( on )
	statusId = 5;
    else
	statusId = 1;

    Private::DrawThemeBackground( htheme, p->handle(), 1, statusId, &r, 0 );
    
    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawToolButton( p, x, y, w, h, g, on, down, enabled, autoRaised, fill );
#endif
}

void QWindowsXPStyle::drawDropDownButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool down, bool enabled, bool autoRaised,
		 const QBrush *fill )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TOOLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawDropDownButton( p, x, y, w, h, g, down, enabled, autoRaised, fill );
	return;
    }

    Q_RECT

    int statusId;
    if ( !enabled )
	statusId = 4;
    else if ( down )
	statusId = 3;
    else if ( !g.brightText().isValid() )
	statusId = 2;
    else
	statusId = 1;

    Private::DrawThemeBackground( htheme, p->handle(), 3, statusId, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawDropDownButton( p, x, y, w, h, g, down, enabled, autoRaised, fill );
#endif
}

void QWindowsXPStyle::drawPopupPanel( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g,  int lineWidth, const QBrush *fill )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawPopupPanel( p, x, y, w, h, g, lineWidth, fill );
	return;
    }

    Q_RECT

    // ### too dark
    Private::DrawThemeBorder( htheme, p->handle(), 1, &r );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawPopupPanel( p, x, y, w, h, g, lineWidth, fill );
#endif
}

void QWindowsXPStyle::drawArrow( QPainter *p, Qt::ArrowType type, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool enabled, const QBrush *fill )
{
    QWindowsStyle::drawArrow( p, type, down, x, y, w, h, g, enabled, fill );
}


// Push button
void QWindowsXPStyle::getButtonShift( int &x, int &y) const
{
    x = y = 0;
}

void QWindowsXPStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawPushButton( btn, p );
	return;
    }

    RECT r;
    r.left = 0;
    r.right = btn->width();
    r.top = 0;
    r.bottom = btn->height();

    if ( btn->isEnabled() ) {
	int stateId = 1;
	if ( btn->isDown() ) {
	    stateId = 3;
	} else if ( d->hotWidget == btn ) {
	    stateId  =2;
	} else if ( btn->isDefault() ) {
	    stateId = 5;
	}
	Private::DrawThemeBackground( htheme, p->handle(), 1, stateId, &r, 0);
    } else {
	Private::DrawThemeBackground( htheme, p->handle(), 1, 4, &r, 0);
    }

    Private::CloseThemeData( htheme );
#else
     QWindowsStyle::drawPushButton( btn, p );
#endif
}

void QWindowsXPStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawPushButton( btn, p );
	return;
    }

    RECT r;
    r.left = 0;
    r.right = btn->width();
    r.top = 0;
    r.bottom = btn->height();

    int stateId;
    if ( btn->isEnabled() ) {
	if ( btn->isDown() ) {
	    stateId = 3;
	} else if ( btn->isOn() ) {
	    stateId  =2;
	} else if ( btn->hasFocus() ) {
	    stateId = 5;
	} else 
	    stateId = 1;
    } else {
	stateId = 4;
    }

    Private::DrawThemeText( htheme, p->handle(), 1, stateId,
	(TCHAR*)qt_winTchar( btn->text(), FALSE ), btn->text().length(), DT_CENTER | DT_VCENTER | DT_SINGLELINE, 0, &r );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawPushButtonLabel( btn, p );
#endif
}

// Radio button
QSize QWindowsXPStyle::exclusiveIndicatorSize() const
{
    return QSize( 13, 13 );
}

void QWindowsXPStyle::drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
		    const QColorGroup &g, bool on, bool down, bool enabled )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawExclusiveIndicator( p, x, y, w, h, g, on, down, enabled );
	return;
    }

    Q_RECT

    int stateId;
    if ( !enabled ) {
	if ( on )
	    stateId = 8;
	else
	    stateId = 4;
    } else if ( down ) {
	if ( on )
	    stateId = 7;
	else
	    stateId = 3;
    } else if ( !g.brightText().isValid() ) {
	if ( on )
	    stateId = 6;
	else
	    stateId = 2;
    } else {
	if ( on )
	    stateId = 5;
	else
	    stateId = 1;
    }

    Private::DrawThemeBackground( htheme, p->handle(), 2, stateId, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawExclusiveIndicator( p, x, y, w, h, g, on, down, enabled );
#endif
}

QSize QWindowsXPStyle::indicatorSize() const
{
    return QWindowsStyle::indicatorSize();
}

void QWindowsXPStyle::drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
		    int state, bool down, bool enabled )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawIndicator( p, x, y, w, h, g, state, down, enabled );
	return;
    }

    Q_RECT

    int stateId;
    switch ( state ) {
    case QButton::On:
	stateId = 5;
	break;
    case QButton::NoChange:
	stateId = 9;
	break;
    default:
	stateId = 1;
	break;
    }
    if ( down )
	stateId += 2;
    else if ( !enabled )
	stateId += 3;
    else if ( !g.brightText().isValid() )
	stateId += 1;

    Private::DrawThemeBackground( htheme, p->handle(), 3, stateId, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawIndicator( p, x, y, w, h, g, state, down, enabled );
#endif
}

// ComboBox
void QWindowsXPStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
		  const QColorGroup &g, bool sunken, bool editable, bool enabled, const QBrush *fill )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"COMBOBOX" );
    if ( !htheme ) {
	QWindowsStyle::drawComboButton( p, x, y, w, h, g, sunken, editable, enabled, fill );
	return;
    }

    Q_RECT

    Private::DrawThemeBorder( htheme, p->handle(), 1, &r );

    int xpos = x;
    if( !QApplication::reverseLayout() )
        xpos += w - 2 - 16;

    RECT r2;
    r2.left = xpos;
    r2.right = xpos+16;
    r2.top = y+2;
    r2.bottom = y+h-2;

    if ( sunken )
	Private::DrawThemeBackground( htheme, p->handle(), 1, 3, &r2, 0 );
    else
	Private::DrawThemeBackground( htheme, p->handle(), 1, 
	    enabled ? ( d->hotWidget == p->device() ? 2 : 1 ) : 4, &r2, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawComboButton( p, x, y, w, h, g, sunken, editable, enabled, fill );
#endif
}

// Toolbar
int QWindowsXPStyle::toolBarHandleExtent() const
{
    return 15;
}

void QWindowsXPStyle::drawToolBarHandle( QPainter *p, const QRect &r,
				Qt::Orientation orientation,
				bool highlight, const QColorGroup &cg,
				bool drawBorder )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"REBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolBarHandle( p, r, orientation, highlight, cg, drawBorder );
	return;
    }

    RECT rect;
    rect.left = r.left();
    rect.top = r.top();
    rect.right = r.right();
    rect.bottom = r.bottom();

    if ( orientation == Horizontal )
	Private::DrawThemeBackground( htheme, p->handle(), 1, 1, &rect, 0 );
    else
	Private::DrawThemeBackground( htheme, p->handle(), 2, 1, &rect, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawToolBarHandle( p, r, orientation, highlight, cg, drawBorder );
#endif
}

int QWindowsXPStyle::toolBarFrameWidth() const
{
    return QWindowsStyle::toolBarFrameWidth();
}

void QWindowsXPStyle::drawToolBarPanel( QPainter *p, int x, int y, int w, int h,
			       const QColorGroup &g, const QBrush *fill )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"REBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolBarPanel( p, x, y, w, h, g, fill );
	return;
    }

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x+w;
    rect.bottom = y+h;

    Private::DrawThemeBackground( htheme, p->handle(), 3, 1, &rect, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawToolBarPanel( p, x, y, w, h, g, fill );
#endif
}

void QWindowsXPStyle::drawToolBarSeparator( QPainter *p, int x, int y, int w, int h,
				   const QColorGroup & g, Orientation orientation )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TOOLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolBarSeparator( p, x, y, w, h, g, orientation );
	return;
    }

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x+w;
    rect.bottom = y+h;

    if ( orientation == Horizontal )
	Private::DrawThemeBackground( htheme, p->handle(), 5, 1, &rect, 0 );
    else
	Private::DrawThemeBackground( htheme, p->handle(), 6, 1, &rect, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawToolBarSeparator( p, x, y, w, h, g, orientation );
#endif
}

QSize QWindowsXPStyle::toolBarSeparatorSize( Qt::Orientation orientation ) const
{
    return QWindowsStyle::toolBarSeparatorSize( orientation );
}

// TabBar
void QWindowsXPStyle::drawTab( QPainter* p, const QTabBar *bar, QTab *tab, bool selected )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TAB" );
    if ( !htheme ) {
	QWindowsStyle::drawTab( p, bar, tab, selected );
	return;
    }

    QRect rect( tab->rect() );
    RECT r;
    r.left = rect.left();
    r.right = rect.right()+ selected;
    r.top = rect.top();
    r.bottom = rect.bottom()+ selected;

    if ( selected )
	Private::DrawThemeBackground( htheme, p->handle(), 6, d->hotTab == tab ? 5 : 3, &r, 0 );
    else {
	if ( !tab->identitifer() )
	    Private::DrawThemeBackground( htheme, p->handle(), 2, d->hotTab == tab ? 2 : 1, &r, 0 );
	else
	    Private::DrawThemeBackground( htheme, p->handle(), 1, d->hotTab == tab ? 2 : 1, &r, 0 );
    }

    Private::CloseThemeData( htheme );
#else
    QWindowStyle::drawTab( p, bar, tab, selected );
#endif
}

void QWindowsXPStyle::drawTabBarExtension( QPainter * p, int x, int y, int w, int h,
				  const QColorGroup & cg, const QTabWidget * tw )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TAB" );
    if ( !htheme ) {
	QWindowsStyle::drawTabBarExtension( p, x, y, w, h, cg, tw );
	return;
    }
    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawTabBarExtension( p, x, y, w, h, cg, tw );
#endif
}

// ScrollBar
QSize QWindowsXPStyle::scrollBarExtent() const
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"SCROLLBAR" );
    if ( !htheme ) {
	return QWindowsStyle::scrollBarExtent();
    }

    RECT cr;
    RECT r;
    RECT bound;
    bound.left = 0;
    bound.right = 16;
    bound.top = 0;
    bound.bottom = 16;
    Private::GetThemeBackgroundContentRect( htheme, 0, 1, 1, &bound, &cr );
    Private::GetThemeBackgroundExtent( htheme, 0, 1, 1, &cr, &r );

    int hsize = r.right - r.left;
    int vsize = r.bottom - r.top;

    return QSize( vsize, hsize );
#else
    return QWindowsStyle::scrollBarExtent();
#endif
}

void QWindowsXPStyle::drawScrollBarControls( QPainter *p,  const QScrollBar *sb,
			int sliderStart, uint controls, uint activeControl )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"SCROLLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawScrollBarControls( p, sb, sliderStart, controls, activeControl );
	return;
    }

#define HORIZONTAL      (sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL        !HORIZONTAL
#define WINDOWS_BORDER  2
#define SLIDER_MIN      9

    QColorGroup g = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax) // sanity check
        sliderStart = sliderMax;

    int b = 0;
    int dimB = buttonDim;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( HORIZONTAL ) {
        subY = addY = ( extent - dimB ) / 2;
        subX = b;
        addX = length - dimB - b;
    } else {
        subX = addX = ( extent - dimB ) / 2;
        subY = b;
        addY = length - dimB - b;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
        subPageR.setRect( subB.right() + 1, b,
                          sliderStart - subB.right() - 1 , sliderW );
        addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
        sliderR .setRect( sliderStart, b, sliderLength, sliderW );
    } else {
        subPageR.setRect( b, subB.bottom() + 1, sliderW,
                          sliderStart - subB.bottom() - 1 );
        addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
        sliderR .setRect( b, sliderStart, sliderW, sliderLength );
    }

    bool maxedOut = (sb->maxValue() == sb->minValue());

    if ( controls & AddLine ) {
	RECT r;
	r.left = addB.left();
	r.right = addB.right();
	r.top = addB.top();
	r.bottom = addB.bottom();

	int stateId;
	if ( maxedOut )
	    stateId = 8;
	else if ( activeControl == AddLine )
	    stateId = 7;
	else if ( d->hotWidget == (QWidget*)sb && addB.contains( d->hotSpot ) )
	    stateId = 6;
	else
	    stateId = 5;
	if ( HORIZONTAL )
	    stateId += 8;

	Private::DrawThemeBackground( htheme, p->handle(), 1, stateId, &r, 0 );
    } 
    if ( controls & SubLine ) {
	RECT r;
	r.left = subB.left();
	r.right = subB.right();
	r.top = subB.top();
	r.bottom = subB.bottom();
	
	int stateId;
	if ( maxedOut )
	    stateId = 4;
	else if ( activeControl == SubLine )
	    stateId = 3;
	else if ( d->hotWidget == (QWidget*)sb && subB.contains( d->hotSpot ) )
	    stateId = 2;
	else
	    stateId = 1;
	if ( HORIZONTAL )
	    stateId += 8;

	Private::DrawThemeBackground( htheme, p->handle(), 1, stateId, &r, 0 );
    }
    if ( maxedOut ) {
	RECT r;
	r.left = sliderR.left();
	r.right = sliderR.right() + HORIZONTAL;
	r.top = sliderR.top();
	r.bottom = sliderR.bottom() + !HORIZONTAL;

	const int swidth = r.right - r.left;
	const int sheight = r.bottom - r.top;

	RECT gr;
	if ( HORIZONTAL ) {
	    gr.left = r.left + swidth/2 - 5;
	    gr.right = gr.left + 10;
	    gr.top = r.top + sheight/2 - 3;
	    gr.bottom = gr.top + 6;
	} else {
	    gr.left = r.left + swidth/2 - 3;
	    gr.right = gr.left + 6;
	    gr.top = r.top + sheight/2 - 5;
	    gr.bottom = gr.top + 10;
	}

	Private::DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 2 : 3, 4, &r, 0 );
	Private::DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 8 : 9, 4, &gr, 0 );
    } else {
        if (controls & SubPage ) {
	    RECT r;
	    r.left = subPageR.left();
	    r.right = subPageR.right() + HORIZONTAL;
	    r.top = subPageR.top();
	    r.bottom = subPageR.bottom() + !HORIZONTAL;

	    int stateId;
	    if ( SubPage == activeControl )
		stateId = 3;
	    else
		stateId = 1;

	    Private::DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 4 : 5, stateId, &r, 0 );
        } 
	if ( controls  & AddPage ) {
	    RECT r;
	    r.left = addPageR.left() - HORIZONTAL;
	    r.right = addPageR.right();
	    r.top = addPageR.top() - !HORIZONTAL;
	    r.bottom = addPageR.bottom();

	    int stateId;
	    if ( AddPage == activeControl )
		stateId = 3;
	    else
		stateId = 1;

	    Private::DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 4 : 5, stateId, &r, 0 );
	}
        if ( controls & Slider ) {
            if ( !maxedOut ) {
		RECT r;
		r.left = sliderR.left();
		r.right = sliderR.right();
		r.top = sliderR.top();
		r.bottom = sliderR.bottom();

		int stateId;
		if ( activeControl == Slider )
		    stateId = 3;
		else
		    stateId = 1;

		const int swidth = r.right - r.left;
		const int sheight = r.bottom - r.top;

		RECT gr;
		if ( HORIZONTAL ) {
		    gr.left = r.left + swidth/2 - 5;
		    gr.right = gr.left + 10;
		    gr.top = r.top + sheight/2 - 3;
		    gr.bottom = gr.top + 6;
		} else {
		    gr.left = r.left + swidth/2 - 3;
		    gr.right = gr.left + 6;
		    gr.top = r.top + sheight/2 - 5;
		    gr.bottom = gr.top + 10;
		}

		Private::DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 2 : 3, stateId, &r, 0 );
		Private::DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 8 : 9, stateId, &gr, 0 );
            }
        }
    }
    // ### perhaps this should not be able to accept focus if maxedOut?
    if ( sb->hasFocus() && (controls & Slider) )
        drawFocusRect(p, QRect(sliderR.x()+2, sliderR.y()+2,
                               sliderR.width()-5, sliderR.height()-5), g,
                      &sb->backgroundColor());

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawScrollBarControls( p, sb, sliderStart, controls, activeControl );
#endif
}

// Slider
int QWindowsXPStyle::sliderLength() const
{
    return 19;
}

int QWindowsXPStyle::sliderThickness() const
{
    return 19;
}

void QWindowsXPStyle::drawSlider( QPainter *p,
			int x, int y, int w, int h,
			const QColorGroup &g,
			Orientation orientation, bool tickAbove, bool tickBelow)
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TRACKBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawSlider( p, x, y, w, h, g, orientation, tickAbove, tickBelow );
	return;
    }

    Q_RECT

    int statusId;
    int partId = 3;
/*    if ( disabled )
	statusId = 5;
    else if ( down )
	statusId = 3;
    else */if ( d->hotWidget == p->device() )
	statusId = 2;
    else
	statusId = 1;

    if ( orientation == Horizontal ) {
	if ( tickBelow )
	    r.bottom += 6;
	if ( tickAbove )
	    r.top -= 3;
    } else {
	if ( tickBelow )
	    r.right += 6;
	if ( tickAbove )
	    r.left -= 3;
    }

    if ( orientation == Vertical || !tickBelow)
	partId = 4;

    Private::DrawThemeBackground( htheme, p->handle(), partId, statusId, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawSlider( p, x, y, w, h, g, orientation, tickAbove, tickBelow );
#endif
}

void QWindowsXPStyle::drawSliderGroove( QPainter *p,
			int x, int y, int w, int h,
			const QColorGroup& g, QCOORD c,
			Orientation orientation )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"TRACKBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawSliderGroove( p, x, y, w, h, g, c, orientation );
	return;
    }

    RECT r;
    if ( orientation == Horizontal ) {
	r.left = x;
	r.right = x+w;
	r.top = y + 6;
	r.bottom = y+h - 6;
    } else {
	r.left = x + 6;
	r.right = x+w - 6;
	r.top = y;
	r.bottom = y+h;
    }

    Private::DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawSliderGroove( p, x, y, w, h, g, c, orientation );
#endif
}

// Splitter
void QWindowsXPStyle::drawSplitter( QPainter *p,
		    int x, int y, int w, int h,
		    const QColorGroup &g,
		    Orientation orientation)
{
    QWindowsStyle::drawSplitter( p, x, y, w, h, g, orientation );
}

// Popup Menu
void QWindowsXPStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool act, bool dis )
{
    QPointArray a( 7*2 );
    int i, xx, yy;
    xx = x+3;
    yy = y+5;
    for ( i=0; i<3; i++ ) {
        a.setPoint( 2*i,   xx, yy );
        a.setPoint( 2*i+1, xx, yy+2 );
        xx++; yy++;
    }
    yy -= 2;
    for ( i=3; i<7; i++ ) {
        a.setPoint( 2*i,   xx, yy );
        a.setPoint( 2*i+1, xx, yy+2 );
        xx++; yy--;
    }
    p->setPen( g.text() );
    p->drawLineSegments( a );
}

void QWindowsXPStyle::drawPopupMenuItem( QPainter* p, bool checkable,
		    int maxpmw, int tab, QMenuItem* mi,
		    const QPalette& pal,
		    bool act, bool enabled,
		    int x, int y, int w, int h)
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"" );
    if ( !htheme ) {
	QWindowsStyle::drawPopupMenuItem( p, checkable, maxpmw, tab, mi, pal, act, enabled, x, y, w, h );
	return;
    }

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawPopupMenuItem( p, checkable, maxpmw, tab, mi, pal, act, enabled, x, y, w, h );
#endif
}

// MenuBar
void QWindowsXPStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
		    QMenuItem* mi, QColorGroup& g, bool active,
		    bool down, bool hasFocus )
{
    // most whister apps use some theme magic here.
    QRect r( x, y, w, h );
    p->fillRect( r, g.brush( QColorGroup::Button ) );

    if ( active && hasFocus )
	p->fillRect( r, g.highlight() );

    if ( active )
	drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
		g, mi->isEnabled(), mi->pixmap(), mi->text(), -1, &g.highlightedText() );
    else 
	drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
		g, mi->isEnabled(), mi->pixmap(), mi->text(), -1, &g.buttonText() );
}

void QWindowsXPStyle::drawMenuBarPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, const QBrush *fill )
{
}

// TitleBar
void QWindowsXPStyle::drawTitleBar( QPainter *p, int x, int y, int w, int h, 
				   const QColor &left, const QColor &right, 
				   bool active )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawTitleBar( p, x, y, w, h, left, right, active );
	return;
    }

    Q_RECT

    if ( h < 15 )
	Private::DrawThemeBackground( htheme, p->handle(), 2, active ? 1 : 2, &r, 0 );
    else
	Private::DrawThemeBackground( htheme, p->handle(), 6, active ? 1 : 2, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawTitleBar( p, x, y, w, h, left, right, active );
#endif
}

void QWindowsXPStyle::drawTitleBarLabel( QPainter *p, int x, int y, int w, int h, 
					const QString &text, 
					const QColor &tc, bool active )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawTitleBarLabel( p, x, y, w, h, text, tc, active );
	return;
    }

    Q_RECT

    Private::DrawThemeText( htheme, p->handle(), 6, active ? 1 : 2, 
	(TCHAR*)qt_winTchar(text, FALSE), text.length(), DT_LEFT | DT_BOTTOM | DT_SINGLELINE, 0, &r );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawTitleBarLabel( p, x, y, w, h, text, tc, active );
#endif
}

void QWindowsXPStyle::drawTitleBarButton( QPainter *p, int x, int y, int w, int h, 
					 const QColorGroup &g, bool down )
{
}

void QWindowsXPStyle::drawTitleBarButtonLabel( QPainter *p, int x, int y, int w, int h, 
					      const QPixmap *pm, int button, bool down )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawTitleBarButtonLabel( p, x, y, w, h, pm, button, down );
	return;
    }

    Q_RECT

    int stateId = down ? 3 : ( p->device() == d->hotWidget ? 2 : 1 );

    int partId = 0;
    switch ( button ) {
    case 1: // min
	partId = 13;
	break;
    case 2: // max
	partId = 14;
	break;
    case 3: // close
	partId = 16;
	break;
    case 4: // restore 
    case 5:
	partId = 19;
	break;
    case 6: // context help
	partId = 22;
	break;
    case 7: // shade
    case 8:
    default:
	QWindowsStyle::drawTitleBarButtonLabel( p, x, y, w, h, pm, button, down );
	break;
    }
    if ( partId )
	Private::DrawThemeBackground( htheme, p->handle(), partId, stateId, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawTitleBarButtonLabel( p, x, y, w, h, pm, button, down );
#endif
}

// Header
void QWindowsXPStyle::drawHeaderSection( QPainter *p, int x, int y, int w, int h, 
					const QColorGroup &g, bool down )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"HEADER" );
    if ( !htheme ) {
	QWindowsStyle::drawHeaderSection( p, x, y, w, h, g, down );
	return;
    }

    Q_RECT
    
    int stateId;
    if ( down )
	stateId = 3;
    else if ( QRect( x, y, w, h ) == d->hotHeader )
	stateId = 2;
    else
	stateId = 1;

    Private::DrawThemeBackground( htheme, p->handle(), 1, stateId, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawHeaderSection( p, x, y, w, h, g, down );
#endif
}

// SpinBox
int QWindowsXPStyle::spinBoxFrameWidth() const
{
    return 0;
}

void QWindowsXPStyle::drawSpinBoxButton( QPainter *p, int x, int y, int w, int h, 
					const QColorGroup &g, const QSpinBox *sp, 
					bool downbtn, bool enabled, bool down )
{
    if ( !d->use_xp )
	QWindowsStyle::drawSpinBoxButton( p, x, y, w, h, g, sp, downbtn, enabled, down );
}

void QWindowsXPStyle::drawSpinBoxSymbol( QPainter *p, int x, int y, int w, int h, 
					const QColorGroup &g, const QSpinBox *sp, 
					bool downbtn, bool enabled, bool down )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"SPIN" );
    if ( !htheme ) {
	QWindowsStyle::drawSpinBoxSymbol( p, x, y, w, h, g, sp, downbtn, enabled, down );
	return;
    }

    Q_RECT

    int stateId;
    if ( !enabled )
	stateId = 4;
    else if ( down )
	stateId = 3;
    else if ( d->hotWidget == (QWidget*)sp && QRect( x, y, w, h ).contains( d->hotSpot ) )
	stateId = 2;
    else
	stateId = 1;

    Private::DrawThemeBackground( htheme, p->handle(), downbtn ? 2 : 1, stateId, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawSpinBoxSymbol( p, x, y, w, h, g, sp, downbtn, enabled, down );
#endif
}

// GroupBox
void QWindowsXPStyle::drawGroupBoxTitle( QPainter *p, int x, int y, int w, int h, 
					const QColorGroup &g, const QString &text, bool enabled )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawGroupBoxTitle( p, x, y, w, h, g, text, enabled );
	return;
    }

    Q_RECT
    
    Private::DrawThemeText( htheme, p->handle(), 4, 1, (TCHAR*)qt_winTchar( text, FALSE ), text.length(), DT_CENTER | DT_SINGLELINE | DT_VCENTER, 0, &r );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawGroupBoxTitle( p, x, y, w, h, g, text, enabled );
#endif
}

void QWindowsXPStyle::drawGroupBoxFrame( QPainter *p, int x, int y, int w, int h, 
					const QColorGroup &g, const QGroupBox *gb )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawGroupBoxFrame( p, x, y, w, h, g, gb );
	return;
    }

    Q_RECT

    Private::DrawThemeBackground( htheme, p->handle(), 4, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawGroupBoxFrame( p, x, y, w, h, g, gb );
#endif
}

// statusbar
void QWindowsXPStyle::drawStatusBarSection( QPainter *p, int x, int y, int w, int h, 
					   const QColorGroup &g, bool permanent )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"STATUS" );
    if ( !htheme ) {
	QWindowsStyle::drawStatusBarSection( p, x, y, w, h, g, permanent );
	return;
    }

    Q_RECT

    Private::DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawStatusBarSection( p, x, y, w, h, g, permanent );
#endif
}

void QWindowsXPStyle::drawSizeGrip( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"STATUS" );
    if ( !htheme ) {
	QWindowsStyle::drawSizeGrip( p, x, y, w, h, g );
	return;
    }

    Q_RECT

    Private::DrawThemeBackground( htheme, p->handle(), 2, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawSizeGrip( p, x, y, w, h, g );
#endif
}

// progressbar
/*!
 \reimp
 */
int QWindowsXPStyle::progressChunkWidth() const
{
    return 9;
}

/*!
  \reimp
*/
void QWindowsXPStyle::drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"PROGRESS" );
    if ( !htheme ) {
	QWindowsStyle::drawProgressBar( p, x, y, w, h, g );
	return;
    }

    Q_RECT 

    Private::DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawProgressBar( p, x, y, w, h, g );
#endif
}

/*!
 \reimp
 */
void QWindowsXPStyle::drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
#if defined(Q_WS_WIN)
    HTHEME htheme = Private::getThemeData( L"PROGRESS" );
    if ( !htheme ) {
	QWindowsStyle::drawProgressChunk( p, x, y, w, h, g );
	return;
    }

    RECT r;
    r.left = x + 1;
    r.right = x+w-1;
    r.top = y + 1;
    r.bottom = y+h-1;

    Private::DrawThemeBackground( htheme, p->handle(), 3, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#else
    QWindowsStyle::drawProgressChunk( p, x, y, w, h, g );
#endif
}

// HotSpot magic
bool QWindowsXPStyle::eventFilter( QObject *o, QEvent *e )
{
    if ( o && o->isWidgetType() ) {
	switch ( e->type() ) {
	case QEvent::MouseMove: 
	    {
		if ( d->hotWidget != o )
		    break;
		QMouseEvent *me = (QMouseEvent*)e;
		d->hotSpot = me->pos();
		if ( o->inherits( "QTabBar" ) ) {
		    QTabBar* bar = (QTabBar*)o;
		    QTab * t = bar->selectTab( me->pos() );
		    if ( d->hotTab != t ) {
			if ( d->hotTab )
			    d->hotWidget->update( d->hotTab->rect() );
			d->hotTab = t;
			if ( d->hotTab )
			    d->hotWidget->update( d->hotTab->rect() );
		    }
		} else if ( o->inherits( "QHeader" ) ) {
		    QHeader *header = (QHeader*)o;
		    QRect oldHeader = d->hotHeader;

		    if ( header->orientation() == Horizontal )
			d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.x() ) );
		    else
			d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.y() ) );

		    if ( oldHeader != d->hotHeader ) {
			if ( oldHeader.isValid() )
			    header->update( oldHeader );
			if ( d->hotHeader.isValid() )
			    header->update( d->hotHeader );
		    }
		} else if ( o->inherits( "QSpinBox" ) ) {
		    QSpinBox *spinbox = (QSpinBox*)o;
		    QRect rect = spinbox->downRect().unite( spinbox->upRect() );
		    spinbox->update( rect );
		}
	    }
	    break;
	case QEvent::Enter:
	    {
		// since the double buffer pixmap of some widgets is palette-key based
		// we have to change the palette to force a redraw
		if ( d->hotWidget && d->hotWidget->inherits( "QButton" ) ) {
		    QWidget *oldHot = d->hotWidget;
		    d->hotWidget = 0;
		    oldHot->setPalette( d->oldPalette );
		    oldHot->update();
		}
		d->hotWidget = (QWidget*)o;
		d->hotSpot = d->hotWidget->mapFromGlobal( QCursor::pos() );

		if ( d->hotWidget->inherits( "QButton" ) ) {
		    d->oldPalette = d->hotWidget->palette();
		    QPalette newPal = d->oldPalette;
		    newPal.setColor( QColorGroup::BrightText, QColor() );
		    d->hotWidget->setPalette( newPal );
		} else {
		    d->hotWidget->update();
		}
	    }
	    break;
	case QEvent::Leave:
	    if ( o != d->hotWidget )
		break;

	    QPoint curPos = QCursor::pos();
	    d->hotSpot = d->hotWidget->mapFromGlobal( curPos );
	    if ( QApplication::widgetAt( curPos, TRUE ) == d->hotWidget )
		break;

	    QWidget *oldHot = d->hotWidget;
	    QTab *oldTab = d->hotTab;
	    QRect oldHeader = d->hotHeader;
	    d->hotWidget = 0;
	    d->hotTab = 0;
	    d->hotHeader = QRect();

	    if ( !oldHot )
		break;
	    if ( oldHot->inherits( "QButton" ) ) {
		oldHot->setPalette( d->oldPalette );
	    } else if ( oldHot->inherits( "QTabBar" ) ) {
		if ( oldTab )
		    oldHot->update( oldTab->rect() );
	    } else if ( oldHot->inherits( "QHeader" ) ) {
		if ( oldHeader.isValid() )
		    oldHot->update( oldHeader );
	    } else if ( oldHot->inherits( "QSpinBox" ) ) {
		QSpinBox *spinbox = (QSpinBox*)oldHot;
		spinbox->update( spinbox->downRect().unite( spinbox->upRect() ) );
	    } else {
		oldHot->update();
	    }
	    break;
	}
    }

    return QWindowsStyle::eventFilter( o, e );
}

#endif //QT_NO_STYLE_WINDOWSXP
