#include "qwindowsxpstyle.h"

#ifndef QT_NO_STYLE_WINDOWSXP

#include <qpainter.h>
#include <qpushbutton.h>
#include <qapplication.h>

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

# define Q_GET_HANDLES( painter, winclass )			    \
	 HWND hwnd;						    \
	 HDC hdc = painter->handle();				    \
	 {							    \
	     QWidget *widget = (QWidget*)painter->device();	    \
	     hwnd = widget->winId();				    \
	 }							    \
	 HTHEME htheme = Private::OpenThemeData( hwnd, winclass );
# define Q_GET_HANDLES2( widget, painter, winclass )		    \
	 HWND hwnd = widget->winId();				    \
	 HDC hdc = painter->handle();				    \
	 HTHEME htheme = Private::OpenThemeData( hwnd, winclass );
# define Q_GET_HANDLES3( widget, winclass )			    \
	 HWND hwnd = widget->winId();				    \
	 HTHEME htheme = Private::OpenThemeData( hwnd, winclass );
#endif

class QWindowsXPStyle::Private
{
public:
    Private()
    {
#if defined(Q_WS_WIN)
	if ( qWinVersion() == WV_XP && !init_xp ) {
	    init_xp = TRUE;
	    uxtheme = new QLibrary( "uxtheme" );
	    Q_LOAD_THEME_FNC( IsThemeActive )

	    if ( !IsThemeActive ) {
		delete uxtheme;
	    } else {
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
		
		use_xp = IsThemeActive() && IsAppThemed();
		DWORD resultFlags  = GetThemeAppProperties();
		bool ctrlsAreThemed = ((resultFlags & STAP_ALLOW_CONTROLS) != 0);
	    }
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
	    }
	}
#endif
    }

#if defined(Q_WS_WIN)
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
    
#endif

    static bool use_xp;

private:
    static QLibrary *uxtheme;
    static ulong ref;
    static bool init_xp;
};

QLibrary *QWindowsXPStyle::Private::uxtheme = NULL;
ulong QWindowsXPStyle::Private::ref = 0;
bool QWindowsXPStyle::Private::use_xp  = FALSE;
bool QWindowsXPStyle::Private::init_xp = FALSE;

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

#if defined(Q_WS_WIN)
#endif

void QWindowsXPStyle::drawButton( QPainter *p, int x, int y, int w, int h,
                 const QColorGroup &g, bool sunken, const QBrush *fill )
{
    if ( !d->use_xp ) {
	QWindowsStyle::drawButton( p, x, y, w, h, g, sunken, fill );
	return;
    }
#if defined(Q_WS_WIN)
    Q_GET_HANDLES( p, L"BUTTON" )

    if ( !htheme ) {
	QWindowsStyle::drawButton( p, x, y, w, h, g, sunken, fill );
	return;
    }

    RECT r;
    r.left = x;
    r.right = w-x+1;
    r.top = y;
    r.bottom = h-y+1;

    if ( sunken )
	Private::DrawThemeBackground( htheme, hdc, 1, 2, &r, 0 );
    else
	Private::DrawThemeBackground( htheme, hdc, 1, 1, &r, 0 );

    Private::CloseThemeData( htheme );
#endif
}

void QWindowsXPStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
    if ( !d->use_xp ) {
	QWindowsStyle::drawPushButton( btn, p );
	return;
    }
#if defined(Q_WS_WIN)
    Q_GET_HANDLES2( btn, p, L"BUTTON" )

    if ( !htheme ) {
	QWindowsStyle::drawPushButton( btn, p );
	return;
    }

    RECT r;
    r.left = 0;
    r.right = btn->width();
    r.top = 0;
    r.bottom = btn->height();
    HRESULT res = 0;

    if ( btn->isEnabled() ) {
	int stateId = 0;
	if ( btn->isDown() ) {
	    stateId = 3;
	} else if ( btn->isOn() ) {
	    stateId  =2;
	} else if ( btn->hasFocus() ) {
	    stateId = 5;
	}
	res = Private::DrawThemeBackground( htheme, hdc, 1, stateId, &r, 0);
    } else {
	res = Private::DrawThemeBackground( htheme, hdc, 1, 3, &r, 0);
    }

    if ( res != S_OK ) {
	Private::THEME_ERROR_CONTEXT myThemeErrorContext;
	HRESULT hRslt = Private::GetThemeLastErrorContext(&myThemeErrorContext);
	if ( hRslt != S_OK ) {
	    qSystemWarning( "GetThemeLastErrorContext failed!", hRslt );
	} else {
	    Private::FormatThemeMessage( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &myThemeErrorContext );
	}
	return;
    } 

    Private::CloseThemeData( htheme );
#endif
}

void QWindowsXPStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
                const QColorGroup &g, bool sunken, int lineWidth, const QBrush *fill )
{
    if ( !d->use_xp ) {
	QWindowsStyle::drawPanel( p, x, y, w, h, g, sunken, lineWidth, fill );
	return;
    }
#if 0 // strange place for a brute force algorithm...
    HWND hwnd = ((QWidget*)p->device())->winId();
    unsigned short* const bla = new unsigned short[40];
    for ( int a = 0; a < 40; ++a )
	bla[a] = '\0';
    bla[0] = 'A';bla[1] = 'K';bla[2] = 'S';bla[3] = 'C';bla[4] = 'U';bla[5] = 'B';bla[6] = 'L';

    while( TRUE ) {
	
	for ( int i = 'A'; i <= 'Z'; i++ ) {
	    HTHEME htheme = Private::OpenThemeData( hwnd, bla );
	    if ( htheme ) {
		QString str = qt_winQString( bla );
		qDebug( "Theme for %s", str.latin1() );
		HRESULT res = Private::CloseThemeData( htheme );
		if ( res != S_OK )
		    qDebug( "Handle couldn't be closed" );
	    }
	    /*
	    else {
		QString str = qt_winQString( bla );
		qDebug( "No theme for %s", str.latin1() );
	    }
	    */
	    if ( bla[0] == 'Z' ) {
		int b = 0;
		while ( b < 39 && bla[b] == 'Z' ) {
		    bla[b] = 'A';
		    b+=1;
		}
		if ( b < 39 ) {
		    if ( bla[b] == 0 )
			bla[b] = 'A';
		    else
			bla[b] = bla[b] + 1;
		} else {
		    qDebug( "Done!" );
		    while ( TRUE );
		}
	    } else {
		bla[0] = bla[0] + 1;
	    }
	}
    }
#endif
}

QSize QWindowsXPStyle::exclusiveIndicatorSize() const
{
    return QSize( 13, 13 );
/*#if defined(Q_WS_WIN) //this doesn't work yet...
    if ( d->use_xp ) {
	Q_GET_HANDLES3( qApp->mainWidget(), L"BUTTON" )
	if ( !htheme )
	    return QWindowsStyle::exclusiveIndicatorSize();
	SIZE size;
	Private::GetThemePartSize( htheme, 2, 1, &size );
	return QSize( size.cx, size.cy );
    }
#endif
    return QWindowsStyle::exclusiveIndicatorSize();
*/
}

void QWindowsXPStyle::drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
		    const QColorGroup &g, bool on, bool down, bool enabled )
{
    if ( !d->use_xp ) {
	QWindowsStyle::drawExclusiveIndicator( p, x, y, w, h, g, on, down, enabled );
	return;
    }
#if defined(Q_WS_WIN)
    Q_GET_HANDLES2( qApp->mainWidget(), p, L"BUTTON" )

    if ( !htheme ) {
	QWindowsStyle::drawExclusiveIndicator( p, x, y, w, h, g, on, down, enabled );
	return;
    }

    RECT r;
    r.left = x;
    r.right = w-x+1;
    r.top = y;
    r.bottom = h-y+1;
    int stateId = 1;
    if ( down )
	stateId = 3;
    HRESULT res = Private::DrawThemeBackground( htheme, hdc, 2, stateId, &r, 0 );
/*
    if ( res != S_OK ) {
	Private::THEME_ERROR_CONTEXT myThemeErrorContext;
	HRESULT hRslt = Private::GetThemeLastErrorContext(&myThemeErrorContext);
	if ( hRslt != S_OK ) {
	    qSystemWarning( "GetThemeLastErrorContext failed!", hRslt );
	} else {
	    Private::FormatThemeMessage( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &myThemeErrorContext );
	}
	return;
    } 
*/
    if ( on ) {
        p->setPen( NoPen );
        p->setBrush( down ? g.brightText() : g.text() );
        p->drawRect( x+5, y+4, 2, 4 );
        p->drawRect( x+4, y+5, 4, 2 );
    }

    Private::CloseThemeData( htheme );
#endif
}

QSize QWindowsXPStyle::indicatorSize() const
{
#if 0 // defined(Q_WS_WIN), but this doesn't work...
    if ( d->use_xp ) {
	Q_GET_HANDLES3( qApp->mainWidget(), L"BUTTON" )
	if ( !htheme )
	    return QWindowsStyle::indicatorSize();
	SIZE size;
	Private::GetThemePartSize( htheme, 3, 1, &size );
	return QSize( size.cx, size.cy );
    }
#endif
    return QWindowsStyle::indicatorSize();
}

void QWindowsXPStyle::drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
		    int state, bool down, bool enabled )
{
    if ( !d->use_xp ) {
	QWindowsStyle::drawIndicator( p, x, y, w, h, g, state, down, enabled );
	return;
    }
#if defined(Q_WS_WIN)
    Q_GET_HANDLES2( qApp->mainWidget(), p, L"BUTTON" )

    if ( !htheme ) {
	QWindowsStyle::drawIndicator( p, x, y, w, h, g, state, down, enabled );
	return;
    }

    RECT r;
    r.left = x;
    r.right = w-x+1;
    r.top = y;
    r.bottom = h-y+1;

    int stateId = 1;
    if ( down )
	stateId = 3;
    HRESULT res = Private::DrawThemeBackground( htheme, hdc, 3, stateId, &r, 0 );
    if ( state != QButton::Off ) {
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
        if ( state == QButton::NoChange ) {
            p->setPen( g.dark() );
        } else {
            p->setPen( down ? g.brightText() : g.text() );
        }
        p->drawLineSegments( a );
    }

    Private::CloseThemeData( htheme );
#endif
}

#endif //QT_NO_STYLE_WINDOWSXP
