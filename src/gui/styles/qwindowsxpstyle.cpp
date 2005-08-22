/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qwindowsxpstyle.h"

#if !defined(QT_NO_STYLE_WINDOWSXP) || defined(QT_PLUGIN)

#include <private/qobject_p.h>
#include <private/qpaintengine_raster_p.h>
#include <qlibrary.h>
#include <qmap.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qwidget.h>
#include <qapplication.h>
#include <qpixmapcache.h>

#include <qdesktopwidget.h>
#include <qtoolbutton.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qheaderview.h>
#include <qspinbox.h>
#include <qstackedwidget.h>
#include <qpushbutton.h>
#include <qtoolbar.h>
#include <qdockwidget.h>

#include <qt_windows.h>
// Uncomment define below to build debug assisting code, and output
//#define DEBUG_XP_STYLE
#include <qdebug.h>

#ifdef Q_CC_GNU
#  include <w32api.h>
#  if (__W32API_MAJOR_VERSION >= 3 || (__W32API_MAJOR_VERSION == 2 && __W32API_MINOR_VERSION >= 5))
#    ifdef _WIN32_WINNT
#      undef _WIN32_WINNT
#    endif
#    define _WIN32_WINNT 0x0501
#    include <commctrl.h>
#  endif
#endif

#include <uxtheme.h>
#include <tmschema.h>
#include <limits.h>

// Undefined for some compile environments
#ifndef TMT_TEXTCOLOR
#  define TMT_TEXTCOLOR 3803
#endif
#ifndef TMT_BORDERCOLORHINT
#  define TMT_BORDERCOLORHINT 3822
#endif
#ifndef TMT_BORDERSIZE
#  define TMT_BORDERSIZE 2403
#endif
#ifndef TMT_BORDERONLY
#  define TMT_BORDERONLY 2203
#endif
#ifndef TMT_TRANSPARENTCOLOR
#  define TMT_TRANSPARENTCOLOR 3809
#endif
#ifndef TMT_CAPTIONMARGINS
#  define TMT_CAPTIONMARGINS 3603
#endif
#ifndef TMT_SIZINGMARGINS
#  define TMT_SIZINGMARGINS 3601
#endif
#ifndef GT_NONE
#  define GT_NONE 0
#endif
#ifndef GT_IMAGEGLYPH
#  define GT_IMAGEGLYPH 1
#endif
#ifndef TMT_GLYPHTYPE
#  define TMT_GLYPHTYPE 2
#endif

// Older Platform SDKs do not have the extended DrawThemeBackgroundEx
// function. We add the needed parts here, and use the extended
// function dynamically, if available in uxtheme.dll. Else, we revert
// back to using the DrawThemeBackground function.
#ifndef DTBG_OMITBORDER
#  ifndef DTBG_CLIPRECT
#   define DTBG_CLIPRECT        0x00000001
#  endif
#  ifndef DTBG_DRAWSOLID
#   define DTBG_DRAWSOLID       0x00000002
#  endif
#  ifndef DTBG_OMITBORDER
#   define DTBG_OMITBORDER      0x00000004
#  endif
#  ifndef DTBG_OMITCONTENT
#   define DTBG_OMITCONTENT     0x00000008
#  endif
#  ifndef DTBG_COMPUTINGREGION
#   define DTBG_COMPUTINGREGION 0x00000010
#  endif
#  ifndef DTBG_MIRRORDC
#   define DTBG_MIRRORDC        0x00000020
#  endif
    typedef struct _DTBGOPTS
    {
	DWORD dwSize;
	DWORD dwFlags;
	RECT rcClip;
    } DTBGOPTS, *PDTBGOPTS;
#endif // _DTBGOPTS

// These defines are missing from the tmschema, but still exist as
// states for their parts
#ifndef MINBS_INACTIVE
#define MINBS_INACTIVE 5
#endif
#ifndef MAXBS_INACTIVE
#define MAXBS_INACTIVE 5
#endif
#ifndef RBS_INACTIVE
#define RBS_INACTIVE 5
#endif
#ifndef HBS_INACTIVE
#define HBS_INACTIVE 5
#endif
#ifndef CBS_INACTIVE
#define CBS_INACTIVE 5
#endif

// Runtime resolved theme engine function calls
typedef bool (WINAPI *PtrIsAppThemed)();
typedef bool (WINAPI *PtrIsThemeActive)();
typedef HRESULT (WINAPI *PtrGetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);
typedef HTHEME (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (WINAPI *PtrCloseThemeData)(HTHEME hTheme);
typedef HRESULT (WINAPI *PtrDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
typedef HRESULT (WINAPI *PtrDrawThemeBackgroundEx)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions);
typedef HRESULT (WINAPI *PtrGetCurrentThemeName)(OUT LPWSTR pszThemeFileName, int cchMaxNameChars, OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars, OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);
typedef HRESULT (WINAPI *PtrGetThemeDocumentationProperty)(LPCWSTR pszThemeName, LPCWSTR pszPropertyName, OUT LPWSTR pszValueBuff, int cchMaxValChars);
typedef HRESULT (WINAPI *PtrGetThemeBool)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT BOOL *pfVal);
typedef HRESULT (WINAPI *PtrGetThemeColor)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT COLORREF *pColor);
typedef HRESULT (WINAPI *PtrGetThemeEnumValue)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT int *piVal);
typedef HRESULT (WINAPI *PtrGetThemeFilename)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);
typedef HRESULT (WINAPI *PtrGetThemeFont)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, OUT LOGFONT *pFont);
typedef HRESULT (WINAPI *PtrGetThemeInt)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT int *piVal);
typedef HRESULT (WINAPI *PtrGetThemeIntList)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT INTLIST *pIntList);
typedef HRESULT (WINAPI *PtrGetThemeMargins)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, OPTIONAL RECT *prc, OUT MARGINS *pMargins);
typedef HRESULT (WINAPI *PtrGetThemeMetric)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId, OUT int *piVal);
typedef HRESULT (WINAPI *PtrGetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);
typedef HRESULT (WINAPI *PtrGetThemePosition)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT POINT *pPoint);
typedef HRESULT (WINAPI *PtrGetThemePropertyOrigin)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT enum PROPERTYORIGIN *pOrigin);
typedef HRESULT (WINAPI *PtrGetThemeRect)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT RECT *pRect);
typedef HRESULT (WINAPI *PtrGetThemeString)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT LPWSTR pszBuff, int cchMaxBuffChars);
typedef HRESULT (WINAPI *PtrGetThemeBackgroundRegion)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion);
typedef BOOL (WINAPI *PtrIsThemeBackgroundPartiallyTransparent)(HTHEME hTheme, int iPartId, int iStateId);
static PtrIsAppThemed pIsAppThemed = 0;
static PtrIsThemeActive pIsThemeActive = 0;
static PtrOpenThemeData pOpenThemeData = 0;
static PtrCloseThemeData pCloseThemeData = 0;
static PtrDrawThemeBackground pDrawThemeBackground = 0;
static PtrDrawThemeBackgroundEx pDrawThemeBackgroundEx = 0;
static PtrGetCurrentThemeName pGetCurrentThemeName = 0;
static PtrGetThemeBool pGetThemeBool = 0;
static PtrGetThemeColor pGetThemeColor = 0;
static PtrGetThemeEnumValue pGetThemeEnumValue = 0;
static PtrGetThemeFilename pGetThemeFilename = 0;
static PtrGetThemeFont pGetThemeFont = 0;
static PtrGetThemeInt pGetThemeInt = 0;
static PtrGetThemeIntList pGetThemeIntList = 0;
static PtrGetThemeMargins pGetThemeMargins = 0;
static PtrGetThemeMetric pGetThemeMetric = 0;
static PtrGetThemePartSize pGetThemePartSize = 0;
static PtrGetThemePosition pGetThemePosition = 0;
static PtrGetThemePropertyOrigin pGetThemePropertyOrigin = 0;
static PtrGetThemeRect pGetThemeRect = 0;
static PtrGetThemeString pGetThemeString = 0;
static PtrGetThemeBackgroundRegion pGetThemeBackgroundRegion = 0;
static PtrGetThemeDocumentationProperty pGetThemeDocumentationProperty = 0;
static PtrIsThemeBackgroundPartiallyTransparent pIsThemeBackgroundPartiallyTransparent = 0;

// General const values
static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsSepHeight        =  7; // separator item height
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  0; // menu item ver text margin
static const int windowsArrowHMargin     =  6; // arrow horizontal margin
static const int windowsCheckMarkHMargin =  0; // horiz. margins of check mark
static const int windowsRightBorder      = 12; // right border on windows

// External function calls
extern Q_GUI_EXPORT HDC qt_win_display_dc();


// Declarations -----------------------------------------------------------------------------------
class XPThemeData
{
public:
    XPThemeData(const QWidget *w = 0, QPainter *p = 0, const QString &theme = QString(),
                int part = 0, int state = 0, const QRect &r = QRect())
        : widget(w), painter(p), name(theme), htheme(0), partId(part), stateId(state), 
          mirrorHorizontally(false), mirrorVertically(false), noBorder(false),
          noContent(false), rotate(0), rect(r)
    {}

    HRGN mask();
    HTHEME handle();

    RECT toRECT(const QRect &qr);
    bool isValid();

    const QWidget *widget;
    QPainter *painter;
    QString name;
    HTHEME htheme;
    int partId;
    int stateId;

    uint mirrorHorizontally : 1;
    uint mirrorVertically : 1;
    uint noBorder : 1;
    uint noContent : 1;
    uint rotate;
    QRect rect;
};

struct ThemeMapKey {
    QString name;
    int partId;
    int stateId;
    bool noBorder;
    bool noContent;

    ThemeMapKey() : partId(-1), stateId(-1) {}
    ThemeMapKey(const XPThemeData &data)
        : name(data.name), partId(data.partId), stateId(data.stateId),
        noBorder(data.noBorder), noContent(data.noContent) {}

};

inline uint qHash(const ThemeMapKey &key)
{ return qHash(key.name) ^ key.partId ^ key.stateId; }

inline bool operator==(const ThemeMapKey &k1, const ThemeMapKey &k2)
{
    return k1.name == k2.name
           && k1.partId == k2.partId
           && k1.stateId == k2.stateId;
}

enum AlphaChannelType {
    UnknownAlpha = -1,          // Alpha of part & state not yet known
    NoAlpha,                    // Totally opaque, no need to touch alpha (RGB)
    MaskAlpha,                  // Alpha channel must be fixed            (ARGB)
    RealAlpha                   // Proper alpha values from Windows       (ARGB_Premultiplied)
};

struct ThemeMapData {
    AlphaChannelType alphaType; // Which type of alpha on part & state

    bool dataValid         : 1; // Only used to detect if hash value is ok
    bool partIsTransparent : 1;
    bool hasAnyData        : 1; // False = part & state has not data, NOP
    bool hasAlphaChannel   : 1; // True =  part & state has real Alpha
    bool wasAlphaSwapped   : 1; // True =  alpha channel needs to be swapped
    bool hadInvalidAlpha   : 1; // True =  alpha channel contained invalid alpha values

    ThemeMapData() : dataValid(false), partIsTransparent(false), hasAnyData(false),
                     hasAlphaChannel(false), wasAlphaSwapped(false), hadInvalidAlpha(false) {}
};

class QWindowsXPStylePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWindowsXPStyle)
public:
    QWindowsXPStylePrivate()
        : QObjectPrivate(), bufferDC(0), bufferBitmap(0), nullBitmap(0),
          bufferPixels(0), bufferW(0), bufferH(0)
    { init(); }

    ~QWindowsXPStylePrivate()
    { cleanup(); }

    static HWND winId(const QWidget *widget);

    void init(bool force = false);
    void cleanup(bool force = false);
    void cleanupHandleMap();
    const QPixmap *tabBody(QWidget *widget);

    HBITMAP buffer(int w = 0, int h = 0);
    HDC bufferHDC()
    { return bufferDC;}

    static bool resolveSymbols();
    static inline bool useXP(bool update = false);

    bool isTransparent(XPThemeData &themeData);
    QRegion region(XPThemeData &themeData);

    void setTransparency(QWidget *widget, XPThemeData &themeData);
    void drawBackground(XPThemeData &themeData);
    void drawBackgroundThruNativeBuffer(XPThemeData &themeData);
    void drawBackgroundDirectly(XPThemeData &themeData);

    bool hasAnyData(const QRect &rect);
    bool hasAlphaChannel(const QRect &rect);
    bool fixAlphaChannel(const QRect &rect);
    bool swapAlphaChannel(const QRect &rect);

    QRgb groupBoxTextColor;
    QRgb groupBoxTextColorDisabled;
    QRgb sliderTickColor;
    static QMap<QString,HTHEME> *handleMap;

private:
#ifdef DEBUG_XP_STYLE
    void dumpNativeDIB(int w, int h);
    void showProperties(XPThemeData &themeData);
#endif

    static QAtomic ref;
    static bool use_xp;
    static QWidget *limboWidget;
    static QPixmap *tabbody;

    QHash<ThemeMapKey, ThemeMapData> alphaCache;
    HDC bufferDC;
    HBITMAP bufferBitmap;
    HBITMAP nullBitmap;
    uchar *bufferPixels;
    int bufferW, bufferH;
    QIcon dockFloat, dockClose;
};


// Theme data helper ------------------------------------------------------------------------------
/* \internal
    Returns true if the themedata is valid for use.
*/
bool XPThemeData::isValid()
{ 
    return QWindowsXPStylePrivate::useXP() && name.size() && handle();
}


/* \internal
    Returns the theme engine handle to the specific class.
    If the handle hasn't been opened before, it opens the data, and
    adds it to a static map, for caching.
*/
HTHEME XPThemeData::handle()
{
    if (!QWindowsXPStylePrivate::useXP())
        return 0;

    if (!htheme && QWindowsXPStylePrivate::handleMap)
        htheme = QWindowsXPStylePrivate::handleMap->operator[](name);

    if (!htheme) {
        htheme = pOpenThemeData(QWindowsXPStylePrivate::winId(widget),
                                (TCHAR*)name.utf16());
        if (htheme) {
            if (!QWindowsXPStylePrivate::handleMap)
                QWindowsXPStylePrivate::handleMap = new QMap<QString, HTHEME>;
            QWindowsXPStylePrivate::handleMap->operator[](name) = htheme;
        }
    }

    return htheme;
}

/* \internal
    Converts a QRect to the native RECT structure.
*/
RECT XPThemeData::toRECT(const QRect &qr)
{
    RECT r;
    r.left = qr.x();
    r.right = qr.x() + qr.width();
    r.top = qr.y();
    r.bottom = qr.y() + qr.height();
    return r;
}

/* \internal
    Returns the native region of a part, if the part is considered
    transparent. The region is scaled to the parts size (rect).
*/
HRGN XPThemeData::mask()
{
    if (!pIsThemeBackgroundPartiallyTransparent(handle(), partId, stateId))
        return 0;

    HRGN hrgn;
    HDC dc = painter == 0 ? 0 : painter->paintEngine()->getDC();
    RECT nativeRect = toRECT(rect);
    pGetThemeBackgroundRegion(handle(), dc, partId, stateId, &nativeRect, &hrgn);
    if (dc)
        painter->paintEngine()->releaseDC(dc);
    return hrgn;
}

// QWindowsXPStylePrivate -------------------------------------------------------------------------
// Static initializations
QWidget *QWindowsXPStylePrivate::limboWidget = 0;
QPixmap *QWindowsXPStylePrivate::tabbody = 0;
QMap<QString,HTHEME> *QWindowsXPStylePrivate::handleMap = 0;
bool QWindowsXPStylePrivate::use_xp = false;
QAtomic QWindowsXPStylePrivate::ref = QAtomic(-1); // -1 based refcounting

/* \internal
    Checks if the theme engine can/should be used, or if we should
    fall back to Windows style.
*/
bool QWindowsXPStylePrivate::useXP(bool update)
{ 
    if (!update)
        return use_xp;
    return (use_xp = resolveSymbols() && pIsThemeActive() && pIsAppThemed());
}

/* \internal
    Handles refcounting, and queries the theme engine for usage.
*/
void QWindowsXPStylePrivate::init(bool force)
{
    if (ref.ref() && !force)
        return;
    if (!force) // -1 based atomic refcounting
        ref.ref();

    useXP(true);
}

/* \internal
    Cleans up all static data.
*/
void QWindowsXPStylePrivate::cleanup(bool force)
{
    if(bufferBitmap)
        DeleteObject(bufferBitmap);
    bufferBitmap = 0;

    if(bufferDC)
        DeleteDC(bufferDC);
    bufferDC = 0;

    if (ref.deref() && !force)
        return;
    if (!force)  // -1 based atomic refcounting
        ref.deref();

    use_xp = false;
    cleanupHandleMap();
    delete limboWidget;
    delete tabbody;
    limboWidget = 0;
    tabbody = 0;
}

/* \internal
    Closes all open theme data handles to ensure that we don't leak
    resources, and that we don't refere to old handles when for
    example the user changes the theme style.
*/
void QWindowsXPStylePrivate::cleanupHandleMap()
{
    if (!handleMap)
        return;

    QMap<QString, HTHEME>::Iterator it;
    for (it = handleMap->begin(); it != handleMap->end(); ++it)
        pCloseThemeData(it.value());
    delete handleMap;
    handleMap = 0;
}

/*! \internal
    This function will always return a valid window handle, and might
    create a limbo widget to do so.
    We often need a window handle to for example open theme data, so
    this function ensures that we get one.
*/
HWND QWindowsXPStylePrivate::winId(const QWidget *widget)
{
    if (widget)
        return widget->winId();

    if (!limboWidget) {
        limboWidget = new QWidget(0);
        limboWidget->setObjectName("xp_limbo_widget");
    }

    return limboWidget->winId();
}

/*! \internal
    Returns the pointer to a tab widgets body pixmap, scaled to the
    height of the screen. This way the theme engine doesn't need to
    scale the body for every time we ask for it. (Speed optimization)
*/
const QPixmap *QWindowsXPStylePrivate::tabBody(QWidget *)
{
    if (!tabbody) {
        SIZE sz;
        XPThemeData theme(0, 0, "TAB", TABP_BODY);
        pGetThemePartSize(theme.handle(), qt_win_display_dc(), TABP_BODY, 0, 0, TS_TRUE, &sz);

        tabbody = new QPixmap(sz.cx, QApplication::desktop()->screenGeometry().height());
        QPainter painter(tabbody);
        theme.rect = QRect(0, 0, sz.cx, sz.cy);
        drawBackground(theme);
        // We fill with the last line of the themedata, that
        // way we don't get a tiled pixmap inside big tabs
        QPixmap temp(sz.cx, 1);
        painter.drawPixmap(0, 0, temp, 0, sz.cy-1, -1, -1);
        painter.drawTiledPixmap(0, sz.cy, sz.cx, tabbody->height()-sz.cy, temp);
    }
    return tabbody;
}

/*! \internal
    Returns true if all the necessary theme engine symbols were
    resolved.
*/
bool QWindowsXPStylePrivate::resolveSymbols()
{
    static bool tried = false;
    if (!tried) {
        tried = true;
        QLibrary themeLib("uxtheme");
        pIsAppThemed = (PtrIsAppThemed)themeLib.resolve("IsAppThemed");
        if (pIsAppThemed) {
            pIsThemeActive          = (PtrIsThemeActive         )themeLib.resolve("IsThemeActive");
            pGetThemePartSize       = (PtrGetThemePartSize      )themeLib.resolve("GetThemePartSize");
            pOpenThemeData          = (PtrOpenThemeData         )themeLib.resolve("OpenThemeData");
            pCloseThemeData         = (PtrCloseThemeData        )themeLib.resolve("CloseThemeData");
            pDrawThemeBackground    = (PtrDrawThemeBackground   )themeLib.resolve("DrawThemeBackground");
            pDrawThemeBackgroundEx  = (PtrDrawThemeBackgroundEx )themeLib.resolve("DrawThemeBackgroundEx");
            pGetCurrentThemeName    = (PtrGetCurrentThemeName   )themeLib.resolve("GetCurrentThemeName");
            pGetThemeBool           = (PtrGetThemeBool          )themeLib.resolve("GetThemeBool");
            pGetThemeColor          = (PtrGetThemeColor         )themeLib.resolve("GetThemeColor");
            pGetThemeEnumValue      = (PtrGetThemeEnumValue     )themeLib.resolve("GetThemeEnumValue");
            pGetThemeFilename       = (PtrGetThemeFilename      )themeLib.resolve("GetThemeFilename");
            pGetThemeFont           = (PtrGetThemeFont          )themeLib.resolve("GetThemeFont");
            pGetThemeInt            = (PtrGetThemeInt           )themeLib.resolve("GetThemeInt");
            pGetThemeIntList        = (PtrGetThemeIntList       )themeLib.resolve("GetThemeIntList");
            pGetThemeMargins        = (PtrGetThemeMargins       )themeLib.resolve("GetThemeMargins");
            pGetThemeMetric         = (PtrGetThemeMetric        )themeLib.resolve("GetThemeMetric");
            pGetThemePartSize       = (PtrGetThemePartSize      )themeLib.resolve("GetThemePartSize");
            pGetThemePosition       = (PtrGetThemePosition      )themeLib.resolve("GetThemePosition");
            pGetThemePropertyOrigin = (PtrGetThemePropertyOrigin)themeLib.resolve("GetThemePropertyOrigin");
            pGetThemeRect           = (PtrGetThemeRect          )themeLib.resolve("GetThemeRect");
            pGetThemeString         = (PtrGetThemeString        )themeLib.resolve("GetThemeString");
            pGetThemeBackgroundRegion              = (PtrGetThemeBackgroundRegion             )themeLib.resolve("GetThemeBackgroundRegion");
            pGetThemeDocumentationProperty         = (PtrGetThemeDocumentationProperty        )themeLib.resolve("GetThemeDocumentationProperty");
            pIsThemeBackgroundPartiallyTransparent = (PtrIsThemeBackgroundPartiallyTransparent)themeLib.resolve("IsThemeBackgroundPartiallyTransparent");
        }
    }

    return pIsAppThemed != 0;
}

/*! \internal
    Returns a native buffer (DIB section) of at least the size of
    ( \a x , \a y ). The buffer has a 32 bit depth, to not loose
    the alpha values on proper alpha-pixmaps.
*/
HBITMAP QWindowsXPStylePrivate::buffer(int w, int h)
{
    // If we already have a HBITMAP which is of adequate size, just return that
    if (bufferBitmap) {
        if (bufferW >= w && bufferH >= h)
            return bufferBitmap;
        // Not big enough, discard the old one
        if (bufferDC && nullBitmap)
            SelectObject(bufferDC, nullBitmap);
        DeleteObject(bufferBitmap);
        bufferBitmap = 0;
    }

    w = qMax(bufferW, w);
    h = qMax(bufferH, h);

    if (!bufferDC)
        bufferDC = CreateCompatibleDC(qt_win_display_dc());

    // Define the header
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = -h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Create the pixmap
    bufferPixels = 0;
    bufferBitmap = CreateDIBSection(bufferDC, &bmi, DIB_RGB_COLORS, (void **) &bufferPixels, 0, 0);
    GdiFlush();
    nullBitmap = (HBITMAP)SelectObject(bufferDC, bufferBitmap);
    
    if (!bufferBitmap) {
        qErrnoWarning("QWindowsXPStylePrivate::buffer(w,h), failed to create dibsection");
        bufferW = 0;
        bufferH = 0;
        return 0;
    }
    if (!bufferPixels) {
        qErrnoWarning("QWindowsXPStylePrivate::buffer(w,h), did not allocate pixel data");
        bufferW = 0;
        bufferH = 0;
        return 0;
    }
    bufferW = w;
    bufferH = h;
#ifdef DEBUG_XP_STYLE
    qDebug("Creating new dib section (%d, %d)", w, h);
#endif
    return bufferBitmap;
}

/*! \internal
    Returns true if the part contains any transparency at all. This does
    not indicate what kind of transparency we're dealing with. It can be
        - Alpha transparency
        - Masked transparency
*/
bool QWindowsXPStylePrivate::isTransparent(XPThemeData &themeData)
{
    return pIsThemeBackgroundPartiallyTransparent(themeData.handle(), themeData.partId,
                                                  themeData.stateId);
}

/*! \internal
    Returns a QRegion of the region of the part
*/
QRegion QWindowsXPStylePrivate::region(XPThemeData &themeData)
{
    HRGN hRgn = 0;
    RECT rect = themeData.toRECT(themeData.rect);
    if (!SUCCEEDED(pGetThemeBackgroundRegion(themeData.handle(), bufferHDC(), themeData.partId, 
                                             themeData.stateId, &rect, &hRgn)))
        return QRegion();
    
    QRegion rgn = QRegion(0,0,1,1);
    CombineRgn(rgn.handle(), hRgn, 0, RGN_COPY);
    DeleteObject(hRgn);
    return rgn;
}

/*! \internal
    Sets the parts region on a window.
*/
void QWindowsXPStylePrivate::setTransparency(QWidget *widget, XPThemeData &themeData)
{
    HRGN hrgn = themeData.mask();
    if (hrgn)
        SetWindowRgn(winId(widget), hrgn, true);
}

/*! \internal
    Returns true if the native doublebuffer contains a pixel which
    has a non-0xFF alpha value. Should only be use when its
    guaranteed that data painted into the buffer wasn't a proper
    alpha pixmap.
*/
bool QWindowsXPStylePrivate::hasAnyData(const QRect &rect)
{
    const int startX = rect.left();
    const int startY = rect.top();
    const int w = rect.width();
    const int h = rect.height();

    for (int y = startY; y < h; ++y) {
        register DWORD *buffer = (DWORD*)bufferPixels + (y * bufferW);
        for (int x = startX; x < w; ++x, ++buffer) {
            int alpha = (*buffer) >> 24;
            if (alpha != 0xFF) // buffer has been touched
                return true;
        }
    }
    return false;
}

/*! \internal
    Returns true if the native doublebuffer contains pixels with
    varying alpha value.
*/
bool QWindowsXPStylePrivate::hasAlphaChannel(const QRect &rect)
{
    const int startX = rect.left();
    const int startY = rect.top();
    const int w = rect.width();
    const int h = rect.height();
    
    int firstAlpha = -1;
    for (int y = startY; y < h/2; ++y) {
        register DWORD *buffer = (DWORD*)bufferPixels + (y * bufferW);
        for (int x = startX; x < w; ++x, ++buffer) {
            int alpha = (*buffer) >> 24;
            if (firstAlpha == -1)
                firstAlpha = alpha;
            else if (alpha != firstAlpha)
                return true;
        }
    }
    return false;
}

/*! \internal
    When the theme engine paints both a true alpha pixmap and a glyph
    into our buffer, the glyph might not contain a proper alpha value.
    The rule of thumb for premultiplied pixmaps is that the color
    values of a pixel can never be higher than the alpha values, so
    we use this to our advantage here, and fix all instances where
    this occures.
*/
bool QWindowsXPStylePrivate::fixAlphaChannel(const QRect &rect)
{
    const int startX = rect.left();
    const int startY = rect.top();
    const int w = rect.width();
    const int h = rect.height();
    bool hasFixedAlphaValue = false;

    for (int y = startY; y < h; ++y) {
        register DWORD *buffer = (DWORD*)bufferPixels + (y * bufferW);
        for (register int x = startX; x < w; ++x, ++buffer) {
            uint pixel = *buffer;
            int alpha = qAlpha(pixel);
            if (qRed(pixel) > alpha || qGreen(pixel) > alpha || qBlue(pixel) > alpha) {
                *buffer |= 0xff000000;
                hasFixedAlphaValue = true;
            }
        }
    }
    return hasFixedAlphaValue;
}

/*! \internal
    Swaps the alpha values on certain pixels:
        0xFF?????? -> 0x00??????
        0x00?????? -> 0xFF??????
    Used to determin the mask of a non-alpha transparent pixmap in
    the native doublebuffer, and swap the alphas so we may paint
    the image as a Premultiplied QImage with drawImage(), and obtain
    the mask transparency.
*/
bool QWindowsXPStylePrivate::swapAlphaChannel(const QRect &rect)
{
    const int startX = rect.left();
    const int startY = rect.top();
    const int w = rect.width();
    const int h = rect.height();
    bool valueChange = false;

    // Flip the alphas, so that 255-alpha pixels are 0, and 0-alpha are 255.
    for (int y = startY; y < h; ++y) {
        register DWORD *buffer = (DWORD*)bufferPixels + (y * bufferW);
        for (register int x = startX; x < w; ++x, ++buffer) {
            register unsigned int alphaValue = (*buffer) & 0xFF000000;
            if (alphaValue == 0xFF000000) {
                *buffer &= 0x00FFFFFF;
                valueChange = true;
            } else if (alphaValue == 0) {
                *buffer |= 0xFF000000;
                valueChange = true;
            }
        }
    }
    return valueChange;
}

/*! \internal
    Main theme drawing function.
    Determines the correct lowlevel drawing method depending on several
    factors.
        Use drawBackgroundThruNativeBuffer() if:
            - Painter does not have an HDC
            - Theme part is flipped (mirrored horizontally)
        else use drawBackgroundDirectly().
*/
void QWindowsXPStylePrivate::drawBackground(XPThemeData &themeData)
{
    if (themeData.rect.isEmpty())
        return;

    QPainter *painter = themeData.painter;
    Q_ASSERT_X(painter != 0, "QWindowsXPStylePrivate::drawBackground()", "Trying to draw a theme part without a painter");
    if (!painter) 
        return;

    painter->save();

    bool useFallback = painter->paintEngine()->getDC() == 0 
		       || themeData.mirrorHorizontally
		       || (themeData.mirrorVertically && pDrawThemeBackgroundEx == 0);
    if (!useFallback)
        drawBackgroundDirectly(themeData);
    else
        drawBackgroundThruNativeBuffer(themeData);

    painter->restore();
}

/*! \internal
    This function draws the theme parts directly to the paintengines HDC.
    Do not use this if you need to perform other transformations on the
    resulting data.
*/
void QWindowsXPStylePrivate::drawBackgroundDirectly(XPThemeData &themeData)
{
    QPainter *painter = themeData.painter;
    HDC dc = painter->paintEngine()->getDC();
    
    QPoint redirectionDelta(int(painter->deviceMatrix().dx() - painter->matrix().dx()),
                            int(painter->deviceMatrix().dy() - painter->matrix().dy()));
    QRect area = themeData.rect.translated(redirectionDelta);

    QRegion sysRgn = area;
    bool addClipping = painter->hasClipping();
    if (addClipping) {
        sysRgn = painter->clipRegion();
        sysRgn.translate(redirectionDelta);
        SelectClipRgn(dc, sysRgn.handle());
    }

#ifdef DEBUG_XP_STYLE
        printf("---[ DIRECT PAINTING ]------------------> Name(%-10s) Part(%d) State(%d)\n",
               qPrintable(themeData.name), themeData.partId, themeData.stateId);
        showProperties(themeData);
#endif

    DTBGOPTS drawOptions;
    drawOptions.dwSize = sizeof(drawOptions);
    drawOptions.rcClip = themeData.toRECT(area);
    drawOptions.dwFlags = DTBG_CLIPRECT
                          | (themeData.noBorder ? DTBG_OMITBORDER : 0)
                          | (themeData.noContent ? DTBG_OMITCONTENT : 0);
    if (pDrawThemeBackgroundEx != 0) {
	pDrawThemeBackgroundEx(themeData.handle(), dc, themeData.partId, themeData.stateId, &(drawOptions.rcClip), &drawOptions);
    } else {
	QRegion extraClip = sysRgn;
	// We are running on a system where the uxtheme.dll does not have
	// the DrawThemeBackgroundEx function, so we need to clip away
	// borders or contents manually. All flips and mirrors uses the
	// fallback implementation

	int borderSize = 0;
	PROPERTYORIGIN origin = PO_NOTFOUND;
	pGetThemePropertyOrigin(themeData.handle(), themeData.partId, themeData.stateId, TMT_BORDERSIZE, &origin);
        pGetThemeInt(themeData.handle(), themeData.partId, themeData.stateId, TMT_BORDERSIZE, &borderSize);

	// Clip away border region
	if ((origin == PO_CLASS || origin == PO_PART || origin == PO_STATE) && borderSize > 0) {
	    if (themeData.noBorder) {
		extraClip &= area;
		drawOptions.rcClip = themeData.toRECT(area.adjusted(-borderSize, -borderSize, borderSize, borderSize));
	    }

	    // Clip away content region
	    if (themeData.noContent) {
		QRegion content = area.adjusted(borderSize, borderSize, -borderSize, -borderSize);
		extraClip ^= content;
	    }

	    // Set the clip region, if used..
	    if (themeData.noBorder || themeData.noContent) {
		addClipping = true;
		SelectClipRgn(dc, extraClip.handle());
	    }
	}

	pDrawThemeBackground(themeData.handle(), dc, themeData.partId, themeData.stateId, &(drawOptions.rcClip), 0);
    }

    if (addClipping)
        SelectClipRgn(dc, 0);
}

/*! \internal
    This function uses a secondary Native doublebuffer for painting parts.
    It should only be used when the painteengine doesn't provide a proper
    HDC for direct painting (e.g. when doing a grabWidget(), painting to
    other pixmaps etc), or when special transformations are needed (e.g.
    flips (horizonal mirroring only, vertical are handled by the theme
    engine).
*/
void QWindowsXPStylePrivate::drawBackgroundThruNativeBuffer(XPThemeData &themeData)
{
    QPainter *painter = themeData.painter;
    QRect rect = themeData.rect;
    int partId = themeData.partId;
    int stateId = themeData.stateId;
    int w = rect.width();
    int h = rect.height();

    // Values initialized later, either from cached values, or from function calls
    AlphaChannelType alphaType = UnknownAlpha;
    bool stateHasData = true; // We assume so;
    bool hasAlpha = false;
    bool partIsTransparent;
    bool inspectData;
    bool potentialInvalidAlpha;

    QString pixmapCacheKey = QString("$qt_xp_%1p%2s%3s%4b%5c%6w%7h").arg(themeData.name)
                             .arg(partId).arg(stateId).arg(!themeData.noBorder).arg(!themeData.noContent)
                             .arg(w).arg(h);
    QPixmap cachedPixmap;
    ThemeMapKey key(themeData);
    ThemeMapData data = alphaCache.value(key);

    bool haveCachedPixmap = false;
    bool isCached = data.dataValid;
    if (isCached) {
        if (!(stateHasData = data.hasAnyData))
            return; // Cached NOOP
        inspectData = data.wasAlphaSwapped;
        partIsTransparent = data.partIsTransparent;
        hasAlpha = data.hasAlphaChannel;
        alphaType = data.alphaType;
        potentialInvalidAlpha = data.hadInvalidAlpha;

        haveCachedPixmap = QPixmapCache::find(pixmapCacheKey, cachedPixmap);

#ifdef DEBUG_XP_STYLE
        char buf[25];
        ::sprintf(buf, "+ Pixmap(%3d, %3d) ]", w, h);
        printf("---[ CACHED %s--------> Name(%-10s) Part(%d) State(%d)\n",
               haveCachedPixmap ? buf : "]-------------------",
               qPrintable(themeData.name), themeData.partId, themeData.stateId);
#endif
    } else {
        // Not cached, so get values from Theme Engine
        BOOL tmt_borderonly = false;
        COLORREF tmt_transparentcolor = 0x0;
        PROPERTYORIGIN proporigin = PO_NOTFOUND;
        pGetThemeBool(themeData.handle(), themeData.partId, themeData.stateId, TMT_BORDERONLY, &tmt_borderonly);
        pGetThemeColor(themeData.handle(), themeData.partId, themeData.stateId, TMT_TRANSPARENTCOLOR, &tmt_transparentcolor);
        pGetThemePropertyOrigin(themeData.handle(), themeData.partId, themeData.stateId, TMT_CAPTIONMARGINS, &proporigin);
        inspectData = (tmt_transparentcolor != 0 || tmt_borderonly || proporigin == PO_PART || proporigin == PO_STATE);
        partIsTransparent = isTransparent(themeData);

        potentialInvalidAlpha = false;
        pGetThemePropertyOrigin(themeData.handle(), themeData.partId, themeData.stateId, TMT_GLYPHTYPE, &proporigin);
        if (proporigin == PO_PART || proporigin == PO_STATE) {
            int tmt_glyphtype = GT_NONE;
            pGetThemeEnumValue(themeData.handle(), themeData.partId, themeData.stateId, TMT_GLYPHTYPE, &tmt_glyphtype);
            potentialInvalidAlpha = partIsTransparent && !inspectData && tmt_glyphtype == GT_IMAGEGLYPH;
        }

#ifdef DEBUG_XP_STYLE
        printf("---[ NOT CACHED ]-----------------------> Name(%-10s) Part(%d) State(%d)\n",
               qPrintable(themeData.name), themeData.partId, themeData.stateId);
        printf("-->partIsTransparen      = %d\n", partIsTransparent);
        printf("-->inspectData           = %d\n", inspectData);
        printf("-->potentialInvalidAlpha = %d\n", potentialInvalidAlpha);
        showProperties(themeData);
#endif
    }
    bool wasAlphaSwapped = false;
    bool wasAlphaFixed = false;

    // OLD PSDK Workaround ------------------------------------------------------------------------
    // See if we need extra clipping for the older PSDK, which does
    // not have a DrawThemeBackgroundEx function for DTGB_OMITBORDER
    // and DTGB_OMITCONTENT
    bool addBorderContentClipping = false;
    QRegion extraClip;
    QRect area = rect;
    if (themeData.noBorder || themeData.noContent) {
	extraClip = area;
	// We are running on a system where the uxtheme.dll does not have
	// the DrawThemeBackgroundEx function, so we need to clip away
	// borders or contents manually. 

	int borderSize = 0;
	PROPERTYORIGIN origin = PO_NOTFOUND;
	pGetThemePropertyOrigin(themeData.handle(), themeData.partId, themeData.stateId, TMT_BORDERSIZE, &origin);
	pGetThemeInt(themeData.handle(), themeData.partId, themeData.stateId, TMT_BORDERSIZE, &borderSize);

	// Clip away border region
	if ((origin == PO_CLASS || origin == PO_PART || origin == PO_STATE) && borderSize > 0) {
	    if (themeData.noBorder) {
		extraClip &= area;
		area = area.adjusted(-borderSize, -borderSize, borderSize, borderSize);
	    }

	    // Clip away content region
	    if (themeData.noContent) {
		QRegion content = area.adjusted(borderSize, borderSize, -borderSize, -borderSize);
		extraClip ^= content;
	    }
	}
	addBorderContentClipping = (themeData.noBorder | themeData.noContent);
    }

    QImage img;
    if (!haveCachedPixmap) { // If the pixmap is not cached, generate it! -------------------------
        buffer(w, h); // Ensure a buffer of at least (w, h) in size
        HDC dc = bufferHDC();

        // Clear the buffer
        if (alphaType != NoAlpha) {
            // Consider have separate "memset" function for small chunks for more speedup
            memset(bufferPixels, inspectData ? 0xFF : 0x00, bufferW * h * 4);
        } 

	// Difference between area and rect
	int dx = area.x() - rect.x();
	int dy = area.y() - rect.y();
	int dr = area.right() - rect.right();
	int db = area.bottom() - rect.bottom();

	// Adjust so painting rect starts from Origo
	rect.moveTo(0,0);
        area.moveTo(dx,dy);
	DTBGOPTS drawOptions;
        drawOptions.dwSize = sizeof(drawOptions);
        drawOptions.rcClip = themeData.toRECT(rect);
        drawOptions.dwFlags = DTBG_CLIPRECT
                            | (themeData.noBorder ? DTBG_OMITBORDER : 0)
                            | (themeData.noContent ? DTBG_OMITCONTENT : 0);

        // Drawing the part into the backing store
	if (pDrawThemeBackgroundEx != 0) {
            pDrawThemeBackgroundEx(themeData.handle(), dc, themeData.partId, themeData.stateId, &(themeData.toRECT(area)), &drawOptions);
	} else {
	    // Set the clip region, if used..
	    if (addBorderContentClipping) {
		SelectClipRgn(dc, extraClip.handle());
		// Compensate for the noBorder area difference (noContent has the same area)
		drawOptions.rcClip = themeData.toRECT(rect.adjusted(dx, dy, dr, db));
	    }

	    pDrawThemeBackground(themeData.handle(), dc, themeData.partId, themeData.stateId, &(drawOptions.rcClip), 0);

	    if (addBorderContentClipping)
		SelectClipRgn(dc, 0);
	}

        // If not cached, analyze the buffer data to figure
        // out alpha type, and if it contains data
        if (!isCached) {
            if (inspectData)
                stateHasData = hasAnyData(rect);
            // SHORTCUT: If the part's state has no data, cache it for NOOP later
            if (!stateHasData) {
                memset(&data, 0, sizeof(data));
                data.dataValid = true;
                alphaCache.insert(key, data);
                return;
            }
            hasAlpha = hasAlphaChannel(rect);
#if defined(DEBUG_XP_STYLE) && 1
            dumpNativeDIB(w, h);
#endif
        }

        // Swap alpha values, if needed
        if (inspectData)
            wasAlphaSwapped = swapAlphaChannel(rect);

        // Fix alpha values, if needed
        if (potentialInvalidAlpha)
            wasAlphaFixed = fixAlphaChannel(rect);

        QImage::Format format;
        if ((partIsTransparent && !wasAlphaSwapped) || (!partIsTransparent && hasAlpha)) {
            format = QImage::Format_ARGB32_Premultiplied;
            alphaType = RealAlpha;
        } else if (wasAlphaSwapped) {
            format = QImage::Format_ARGB32_Premultiplied;
            alphaType = MaskAlpha;
        } else {
            format = QImage::Format_RGB32;
            alphaType = NoAlpha;
        }
#if defined(DEBUG_XP_STYLE) && 1
        printf("Image format is: %s\n", alphaType == RealAlpha ? "Real Alpha" : alphaType == MaskAlpha ? "Masked Alpha" : "No Alpha");
#endif
        img = QImage(bufferPixels, bufferW, bufferH, format);
    } 

    // Blitting backing store
    bool useRegion = partIsTransparent && !hasAlpha && !wasAlphaSwapped;

    QRegion newRegion;
    QRegion oldRegion;
    if (useRegion) {
        newRegion = region(themeData);
        oldRegion = painter->clipRegion();
        painter->setClipRegion(newRegion);
#if defined(DEBUG_XP_STYLE) && 0
        printf("Using region:\n");
        QVector<QRect> rects = newRegion.rects();
        for (int i = 0; i < rects.count(); ++i) {
            const QRect &r = rects.at(i);
            printf("    (%d, %d, %d, %d)\n", r.x(), r.y(), r.right(), r.bottom());
        }
#endif
    }

    if (addBorderContentClipping)
	painter->setClipRegion(extraClip, Qt::IntersectClip);

    if (!themeData.mirrorHorizontally && !themeData.mirrorVertically) {
        if (!haveCachedPixmap)
            painter->drawImage(themeData.rect, img, rect);
        else
            painter->drawPixmap(themeData.rect, cachedPixmap);
    } else {
        // This is _slow_!
        // Make a copy containing only the necessary data, and mirror
        // on all wanted axes. Then draw the copy.
        // If cached, the normal pixmap is cached, instead of caching
        // all possible orientations for each part and state.
        QImage imgCopy;
        if (!haveCachedPixmap)
            imgCopy = img.copy(rect);
        else
            imgCopy = cachedPixmap.toImage();
        painter->drawImage(themeData.rect,
                           imgCopy.mirrored(themeData.mirrorHorizontally, themeData.mirrorVertically));
    }

    if (useRegion || addBorderContentClipping) {
        if (oldRegion.isEmpty())
            painter->setClipping(false);
        else
            painter->setClipRegion(oldRegion);
    }

    // Cache the pixmap to avoid expensive swapAlphaChannel() calls
    if (!haveCachedPixmap && ((w * h) < 2000) && w && h) {
        QPixmap pix = QPixmap::fromImage(img).copy(rect);
        QPixmapCache::insert(pixmapCacheKey, pix);
#ifdef DEBUG_XP_STYLE
        printf("+++Adding pixmap to cache, size(%d, %d), wasAlphaSwapped(%d), wasAlphaFixed(%d), name(%s)\n", 
               w, h, wasAlphaSwapped, wasAlphaFixed, qPrintable(pixmapCacheKey));
#endif
    }

    // Add to theme part cache
    if (!isCached) {
        memset(&data, 0, sizeof(data));
        data.dataValid = true;
        data.partIsTransparent = partIsTransparent;
        data.alphaType = alphaType;
        data.hasAlphaChannel = hasAlpha;
        data.hasAnyData = stateHasData;
        data.wasAlphaSwapped = wasAlphaSwapped;
        data.hadInvalidAlpha = wasAlphaFixed;
        alphaCache.insert(key, data);
    }
}


// ------------------------------------------------------------------------------------------------

/*!
    \class QWindowsXPStyle
    \brief The QWindowsXPStyle class provides a Microsoft WindowsXP-like look and feel.

    \ingroup appearance

    \warning This style is only available on the Windows XP platform
    because it makes use of Windows XP's style engine.

    Most of the functions are documented in the base classes
    QWindowsStyle, QCommonStyle, and QStyle, but the
    QWindowsXPStyle overloads of drawComplexControl(), drawControl(),
    drawControlMask(), drawPrimitive(), subControlRect(), and
    sizeFromContents(), are documented here.

    \img qwindowsxpstyle.png
    \sa QMacStyle, QWindowsStyle, QPlastiqueStyle, QCDEStyle, QMotifStyle
*/

/*!
    Constructs a QWindowsStyle
*/
QWindowsXPStyle::QWindowsXPStyle()
    : QWindowsStyle()
{
    dd = new QWindowsXPStylePrivate;
    dd->q_ptr = this;
}

/*!
    Destroys the style.
*/
QWindowsXPStyle::~QWindowsXPStyle()
{
    delete dd;
}

/*! \reimp */
void QWindowsXPStyle::unpolish(QApplication *app)
{
    QWindowsStyle::unpolish(app);
}

/*! \reimp */
void QWindowsXPStyle::polish(QApplication *app)
{
    QWindowsStyle::polish(app);

    if (!dd->useXP())
        return;

    // Get text color for groupbox labels
    COLORREF cref;
    XPThemeData theme(0, 0, "BUTTON", 0, 0);
    pGetThemeColor(theme.handle(), BP_GROUPBOX, GBS_NORMAL, TMT_TEXTCOLOR, &cref);
    dd->groupBoxTextColor = qRgb(GetRValue(cref), GetGValue(cref), GetBValue(cref));
    pGetThemeColor(theme.handle(), BP_GROUPBOX, GBS_DISABLED, TMT_TEXTCOLOR, &cref);
    dd->groupBoxTextColorDisabled = qRgb(GetRValue(cref), GetGValue(cref), GetBValue(cref));
    // Where does this color come from?
    //pGetThemeColor(theme.handle(), TKP_TICS, TSS_NORMAL, TMT_COLOR, &cref);
    dd->sliderTickColor = qRgb(165, 162, 148);
}

/*! \reimp */
void QWindowsXPStyle::polish(QWidget *widget)
{
    QWindowsStyle::polish(widget);
    if (!dd->useXP())
        return;

    if (qobject_cast<QAbstractButton*>(widget)
        || qobject_cast<QToolButton*>(widget)
        || qobject_cast<QTabBar*>(widget)
        || qobject_cast<QComboBox*>(widget)
        || qobject_cast<QScrollBar*>(widget)
        || qobject_cast<QSlider*>(widget)
        || qobject_cast<QHeaderView*>(widget)
        || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QSpinBox*>(widget)
        || widget->inherits("QWorkspaceChild")
        || widget->inherits("Q3TitleBar"))
        widget->setAttribute(Qt::WA_Hover);

    if (qobject_cast<QStackedWidget*>(widget) &&
               qobject_cast<QTabWidget*>(widget->parent()))
        widget->parentWidget()->setAttribute(Qt::WA_ContentsPropagated);
}

/*! \reimp */
void QWindowsXPStyle::polish(QPalette &pal)
{ 
    QWindowsStyle::polish(pal);
    pal.setBrush(QPalette::AlternateBase, pal.base().color().dark(110));
}

/*! \reimp */
void QWindowsXPStyle::unpolish(QWidget *widget)
{
    // Unpolish of widgets is the first thing that
    // happens when a theme changes, or the theme
    // engine is turned off. So we detect it here.
    bool oldState = dd->useXP();
    bool newState = dd->useXP(true);
    if ((oldState != newState) && newState) {
        dd->cleanup(true);
        dd->init(true);
    } else {
        // Cleanup handle map, if just changing style,
        // or turning it on. In both cases the values
        // already in the map might be old (other style).
        dd->cleanupHandleMap();
    }

    QWindowsStyle::unpolish(widget);
}

/*! \reimp */
QRect QWindowsXPStyle::subElementRect(SubElement sr, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect(option->rect);
    switch(sr) {
    case SE_TabWidgetTabContents:
        if (qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
        {
            rect = QWindowsStyle::subElementRect(sr, option, widget);
            if (sr == SE_TabWidgetTabContents)
                   rect.adjust(0, 0, -2, -2);
        }
        break;

    default:
        rect = QWindowsStyle::subElementRect(sr, option, widget);
    }
    return rect;
}

/*!
    \reimp
*/
void QWindowsXPStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *p,
                                    const QWidget *widget) const
{
    if (!dd->useXP()) {
        QWindowsStyle::drawPrimitive(pe, option, p, widget);
        return;
    }

    QString name;
    int partId = 0;
    int stateId = 0;
    QRect rect = option->rect;
    State flags = option->state;
    bool hMirrored = false;
    bool vMirrored = false;
    bool noBorder = false;
    bool noContent = false;

    switch (pe) {
    case PE_PanelButtonBevel:
        name = "BUTTON";
        partId = BP_PUSHBUTTON;
        if (!(flags & State_Enabled))
            stateId = PBS_DISABLED;
        else if (flags & State_Sunken)
            stateId = PBS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = PBS_HOT;
        //else if (flags & State_ButtonDefault)
        //    stateId = PBS_DEFAULTED;
        else
            stateId = PBS_NORMAL;
        break;

    case PE_PanelButtonTool:
        if (widget && widget->inherits("QDockWidgetTitleButton")) {
            if (const QDockWidget *dw = qobject_cast<const QDockWidget *>(widget->parent()))
                if (dw->isFloating())
                    return;
        }
        name = "TOOLBAR";
        partId = TP_BUTTON;
        if (!flags & State_Enabled)
            stateId = TS_DISABLED;
        else if (flags & State_Sunken)
            stateId = TS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = flags & State_On ? TS_HOTCHECKED : TS_HOT;
        else if (flags & State_On)
            stateId = TS_CHECKED;
        else
            stateId = TS_NORMAL;
        break;

    case PE_IndicatorButtonDropDown:
        name = "TOOLBAR";
        partId = TP_SPLITBUTTONDROPDOWN;
        if (!flags & State_Enabled)
            stateId = TS_DISABLED;
        else if (flags & State_Sunken)
            stateId = TS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = flags & State_On ? TS_HOTCHECKED : TS_HOT;
        else if (flags & State_On)
            stateId = TS_CHECKED;
        else
            stateId = TS_NORMAL;
        break;

    case PE_IndicatorCheckBox:
        name = "BUTTON";
        partId = BP_CHECKBOX;
        if (!(flags & State_Enabled))
            stateId = CBS_UNCHECKEDDISABLED;
        else if (flags & State_Sunken)
            stateId = CBS_UNCHECKEDPRESSED;
        else if (flags & State_MouseOver)
            stateId = CBS_UNCHECKEDHOT;
        else
            stateId = CBS_UNCHECKEDNORMAL;

        if (flags & State_On)
            stateId += CBS_CHECKEDNORMAL-1;
        else if (flags & State_NoChange)
            stateId += CBS_MIXEDNORMAL-1;

        break;

    case PE_IndicatorRadioButton:
        name = "BUTTON";
        partId = BP_RADIOBUTTON;
        if (!(flags & State_Enabled))
            stateId = RBS_UNCHECKEDDISABLED;
        else if (flags & State_Sunken)
            stateId = RBS_UNCHECKEDPRESSED;
        else if (flags & State_MouseOver)
            stateId = RBS_UNCHECKEDHOT;
        else
            stateId = RBS_UNCHECKEDNORMAL;

        if (flags & State_On)
            stateId += RBS_CHECKEDNORMAL-1;
        break;

    case PE_IndicatorDockWidgetResizeHandle:
        return;

    case PE_Frame:
        if (flags & State_Raised)
            return;
        name = "LISTVIEW";
        partId = LVP_LISTGROUP;
        break;

    case PE_FrameLineEdit:
        if (qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            name = "EDIT";
            partId = EP_EDITTEXT;
            noContent = true;
            if (!(flags & State_Enabled))
                stateId = ETS_DISABLED;
            else
                stateId = ETS_NORMAL;
        }
        break;

    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            name = "EDIT";
            partId = EP_EDITTEXT;
            noBorder = true;
            QBrush bg;
            bool usePalette = false;
            uint resolve_mask = panel->palette.resolve();
            if (flags & State_Enabled) {
                stateId = ETS_NORMAL;
                if (resolve_mask & (1 << QPalette::Base)) {
                    // Base color is set for this widget, so use it
                    bg = panel->palette.brush(QPalette::Base);
                    usePalette = true;
                }
            } else {
                stateId = ETS_DISABLED;
                if (resolve_mask & (1 << QPalette::Background)) {
                    // Background color is set for this widget, so use it
                    bg = panel->palette.brush(QPalette::Background);
                    usePalette = true;
                }
            }

            if (usePalette) {
                p->fillRect(panel->rect, bg);
                return;
            }
        }
        break;

    case PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *tab = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
        {
            name = "TAB";
            partId = TABP_PANE;

            if (tab->shape == QTabBar::RoundedNorth)
                break;

            QStyleOptionTabWidgetFrame frameOpt = *tab;
            frameOpt.rect = widget->rect();
            QRect contentsRect = subElementRect(SE_TabWidgetTabContents, &frameOpt, widget);
            QRegion reg = option->rect;
            reg -= contentsRect;
            p->setClipRegion(reg);
            XPThemeData theme(0, p, name, partId, stateId, rect);
            theme.mirrorHorizontally = hMirrored;
            theme.mirrorVertically = vMirrored;
            dd->drawBackground(theme);

            p->setClipRect(contentsRect);
            partId = TABP_BODY;
            switch (tab->shape) {
            case QTabBar::RoundedSouth:
                vMirrored = true;
                break;
            default:
                break;
            }
        }
        break;

    case PE_FrameMenu:
        p->save();
        p->setPen(option->palette.dark().color());
        p->drawRect(rect.adjusted(0, 0, -1, -1));
        p->restore();
        return;

    case PE_PanelMenuBar:
        break;

    case PE_IndicatorHeaderArrow:
        {
#if 0 // XP theme engine doesn't know about this :(
            name = "HEADER";
            partId = HP_HEADERSORTARROW;
            if (flags & State_Down)
                stateId = HSAS_SORTEDDOWN;
            else
                stateId = HSAS_SORTEDUP;
#else
            if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
                p->save();
                p->setPen(option->palette.dark().color());
                p->translate(0, option->rect.height()/2 - 4);
                if (header->sortIndicator & QStyleOptionHeader::SortUp) { // invert logic to follow Windows style guide
                    p->drawLine(option->rect.x(), option->rect.y(), option->rect.x()+8, option->rect.y());
                    p->drawLine(option->rect.x()+1, option->rect.y()+1, option->rect.x()+7, option->rect.y()+1);
                    p->drawLine(option->rect.x()+2, option->rect.y()+2, option->rect.x()+6, option->rect.y()+2);
                    p->drawLine(option->rect.x()+3, option->rect.y()+3, option->rect.x()+5, option->rect.y()+3);
                    p->drawPoint(option->rect.x()+4, option->rect.y()+4);
                } else if(header->sortIndicator & QStyleOptionHeader::SortDown) {
                    p->drawLine(option->rect.x(), option->rect.y()+4, option->rect.x()+8, option->rect.y()+4);
                    p->drawLine(option->rect.x()+1, option->rect.y()+3, option->rect.x()+7, option->rect.y()+3);
                    p->drawLine(option->rect.x()+2, option->rect.y()+2, option->rect.x()+6, option->rect.y()+2);
                    p->drawLine(option->rect.x()+3, option->rect.y()+1, option->rect.x()+5, option->rect.y()+1);
                    p->drawPoint(option->rect.x()+4, option->rect.y());
                }
                p->restore();
                return;
            }
#endif
        }
        break;

    case PE_FrameStatusBar:
        name = "STATUS";
        partId = SP_PANE;
        break;

    case PE_FrameGroupBox:
        name = "BUTTON";
        partId = BP_GROUPBOX;
        if (!(flags & State_Enabled))
            stateId = GBS_DISABLED;
        else
            stateId = GBS_NORMAL;
        break;

    //case PE_SizeGrip:
    //    name = "STATUS";
    //    partId = SP_GRIPPER;
    //    // empiric correction values...
    //    rect.adjust(-4, -8, 0, 0);
    //    mirror = qApp->reverseLayout();
    //    break;

    case PE_IndicatorProgressChunk:
        name = "PROGRESS";
        partId = PP_CHUNK;
        stateId = 1;
        rect = QRect(option->rect.x(), option->rect.y() + 3, option->rect.width(), option->rect.height() - 5);
        break;

    case PE_Q3DockWindowSeparator:
        name = "TOOLBAR";
        if (flags & State_Horizontal)
            partId = TP_SEPARATOR;
        else
            partId = TP_SEPARATORVERT;
        break;

    case PE_FrameDockWidget:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            name = "WINDOW";
            if (flags & State_Active)
                stateId = FS_ACTIVE;
            else
                stateId = FS_INACTIVE;

            int fwidth = pixelMetric(PM_DockWidgetFrameWidth, frm, widget);

            XPThemeData theme(widget, p, name, 0, stateId);
            if (!theme.isValid())
                break;

            theme.rect = QRect(frm->rect.x(), frm->rect.y(), frm->rect.x()+fwidth, frm->rect.height()-fwidth);
            theme.partId = WP_SMALLFRAMELEFT;
            dd->drawBackground(theme);
            theme.rect = QRect(frm->rect.width()-fwidth, frm->rect.y(), fwidth, frm->rect.height()-fwidth);
            theme.partId = WP_SMALLFRAMERIGHT;
            dd->drawBackground(theme);
            theme.rect = QRect(frm->rect.x(), frm->rect.bottom()-fwidth+1, frm->rect.width(), fwidth);
            theme.partId = WP_SMALLFRAMEBOTTOM;
            dd->drawBackground(theme);
            return;
        }
        break;

    case PE_FrameWindow:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            name = "WINDOW";
            if (flags & State_Active)
                stateId = FS_ACTIVE;
            else
                stateId = FS_INACTIVE;

            int fwidth = frm->lineWidth + frm->midLineWidth;

            XPThemeData theme(0, p, name, 0, stateId);
            if (!theme.isValid())
                break;

            theme.rect = QRect(option->rect.x(), option->rect.y()+fwidth, option->rect.x()+fwidth, option->rect.height()-fwidth);
            theme.partId = WP_FRAMELEFT;
            dd->drawBackground(theme);
            theme.rect = QRect(option->rect.width()-fwidth, option->rect.y()+fwidth, fwidth, option->rect.height()-fwidth);
            theme.partId = WP_FRAMERIGHT;
            dd->drawBackground(theme);
            theme.rect = QRect(option->rect.x(), option->rect.height()-fwidth, option->rect.width(), fwidth);
            theme.partId = WP_FRAMEBOTTOM;
            dd->drawBackground(theme);
            theme.rect = QRect(option->rect.x()-5, option->rect.y()-5, option->rect.width()+10, option->rect.y()+fwidth+5);
            theme.partId = WP_CAPTION;
            dd->drawBackground(theme);
            return;
        }
        break;

    case PE_IndicatorBranch:
        {
            static const int decoration_size = 9;
            int mid_h = option->rect.x() + option->rect.width() / 2;
            int mid_v = option->rect.y() + option->rect.height() / 2;
            int bef_h = mid_h;
            int bef_v = mid_v;
            int aft_h = mid_h;
            int aft_v = mid_v;
            QBrush brush(option->palette.dark().color(), Qt::Dense4Pattern);
            if (option->state & State_Item) {
                if (QApplication::isRightToLeft())
                    p->fillRect(option->rect.left(), mid_v, bef_h - option->rect.left(), 1, brush);
                else
                    p->fillRect(aft_h, mid_v, option->rect.right() - aft_h + 1, 1, brush);
            }
            if (option->state & State_Sibling)
                p->fillRect(mid_h, aft_v, 1, option->rect.bottom() - aft_v + 1, brush);
            if (option->state & (State_Open | State_Children | State_Item | State_Sibling))
                p->fillRect(mid_h, option->rect.y(), 1, bef_v - option->rect.y(), brush);
            if (option->state & State_Children) {
                int delta = decoration_size / 2;
                bef_h -= delta;
                bef_v -= delta;
                aft_h += delta;
                aft_v += delta;
                XPThemeData theme(0, p, "TREEVIEW");
                theme.rect = QRect(bef_h, bef_v, decoration_size, decoration_size);
                theme.partId = TVP_GLYPH;
                theme.stateId = flags & QStyle::State_Open ? GLPS_OPENED : GLPS_CLOSED;
                dd->drawBackground(theme);
            }
        }
        return;
    default:
        break;
    }

    XPThemeData theme(0, p, name, partId, stateId, rect);
    if (!theme.isValid()) {
        QWindowsStyle::drawPrimitive(pe, option, p, widget);
        return;
    }
    theme.mirrorHorizontally = hMirrored;
    theme.mirrorVertically = vMirrored;
    theme.noBorder = noBorder;
    theme.noContent = noContent;
    dd->drawBackground(theme);
}

/*!
    \reimp
*/
void QWindowsXPStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *p,
                                  const QWidget *widget) const
{
    if (!dd->useXP()) {
        QWindowsStyle::drawControl(element, option, p, widget);
        return;
    }

    QRect rect(option->rect);
    State flags = option->state;

    int rotate = 0;
    bool hMirrored = false;
    bool vMirrored = false;

    QString name;
    int partId = 0;
    int stateId = 0;
    switch (element) {
    case CE_HeaderSection:
        name = "HEADER";
        partId = HP_HEADERITEM;
        if (flags & State_Sunken)
            stateId = HIS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = HIS_HOT;
        else
            stateId = HIS_NORMAL;
        break;

    case CE_Splitter:
        p->eraseRect(option->rect);
        return; 

    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            name = "BUTTON";
            partId = BP_PUSHBUTTON;
            if (!(flags & State_Enabled) && !(btn->features & QStyleOptionButton::Flat))
                stateId = PBS_DISABLED;
            else if ((btn->features & QStyleOptionButton::Flat) && !(flags & (State_On|State_Sunken)))
                return;
            else if (flags & (State_Sunken | State_On))
                stateId = PBS_PRESSED;
            else if (flags & State_MouseOver)
                stateId = PBS_HOT;
            else if (btn->features & QStyleOptionButton::AutoDefaultButton)
                stateId = PBS_DEFAULTED;
            else
                stateId = PBS_NORMAL;
        }
        break;

    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
        {
            name = "TAB";
            bool isDisabled = !(tab->state & State_Enabled);
            bool hasFocus = tab->state & State_HasFocus;
            bool isHot = tab->state & State_MouseOver;
            bool selected = tab->state & State_Selected;
            bool lastTab = tab->position == QStyleOptionTab::End;
            bool firstTab = tab->position == QStyleOptionTab::Beginning;
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            //bool previousSelected = (tab->selectedPosition == QStyleOptionTab::PreviousIsSelected);
            //bool nextSelected = (tab->selectedPosition == QStyleOptionTab::NextIsSelected);
            bool leftAligned = styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignLeft;
            bool centerAligned = styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignCenter;
            //bool rightAligned = styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignRight;
            int borderThickness = pixelMetric(PM_DefaultFrameWidth, option, widget);
            int tabOverlap = onlyOne ? 0 : pixelMetric(PM_TabBarTabOverlap, option, widget);

            if (isDisabled)
                stateId = TIS_DISABLED;
            else if (selected)
                stateId = TIS_SELECTED;
            else if (hasFocus)
                stateId = TIS_FOCUSED;
            else if (isHot)
                stateId = TIS_HOT;
            else
                stateId = TIS_NORMAL;

            // Selecting proper part depending on position
            if (firstTab || onlyOne) {
                if (leftAligned) {
                    partId = TABP_TABITEMLEFTEDGE;
                } else if (centerAligned) {
                    partId = TABP_TABITEM;
                } else { // rightAligned
                    partId = TABP_TABITEMRIGHTEDGE;
                }
            } else {
                partId = TABP_TABITEM;
            }

            switch (tab->shape) {
            default:
                QCommonStyle::drawControl(element, option, p, widget);
                break;
            case QTabBar::RoundedNorth:
                if (selected)
                    rect.adjust(firstTab ? 0 : -tabOverlap, 0, lastTab ? 0 : tabOverlap, borderThickness);
                else
                    rect.adjust(0, tabOverlap, 0, -borderThickness);
                break;
            case QTabBar::RoundedSouth:
                vMirrored = true;
                if (selected)
                    rect.adjust(firstTab ? 0 : -tabOverlap , -borderThickness, lastTab ? 0 : tabOverlap, 0);
                else
                    rect.adjust(0, borderThickness, 0, -tabOverlap);
                break;
            }



            //switch (tab->shape) {
            //case QTabBar::RoundedNorth:
            //case QTabBar::TriangularNorth:
            //    if ((flags & State_Selected) || (flags & State_HasFocus)) {
            //        rect.adjust(0, 0, 0, 1);
            //    } else {
            //        rect.adjust(0, 1, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::NextIsSelected)
            //            rect.adjust(1, 0, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::PreviousIsSelected)
            //            rect.adjust(0, 0, -1, 0);
            //    }
            //    break;
            //case QTabBar::RoundedSouth:
            //case QTabBar::TriangularSouth:
            //    rotate = 180;
            //    if ((flags & State_Selected) || (flags & State_HasFocus)) {
            //        rect.adjust(0, -1, 0, 0);
            //    } else {
            //        rect.adjust(0, 0, 0, -1);
            //        if (tab->selectedPosition != QStyleOptionTab::NextIsSelected)
            //            rect.adjust(1, 0, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::PreviousIsSelected)
            //            rect.adjust(0, 0, -1, 0);
            //    }
            //    break;
            //case QTabBar::RoundedEast:
            //case QTabBar::TriangularEast:
            //    rotate = 90;
            //    break;
            //case QTabBar::RoundedWest:
            //case QTabBar::TriangularWest:
            //    rotate = 270;
            //    if ((flags & State_Selected) || (flags & State_HasFocus)) {
            //        rect.adjust(0, 0, 1, 0);
            //    } else {
            //        rect.adjust(1, 0, -5, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::NextIsSelected)
            //            rect.adjust(0, 1, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::PreviousIsSelected)
            //            rect.adjust(0, 0, 0, -1);
            //    }
            //    break;
            //}

            //if (!(flags & State_Selected)) {
            //    switch (tab->shape) {
            //    case QTabBar::RoundedNorth:
            //    case QTabBar::TriangularNorth:
            //        rect.adjust(0,0, 0,-1);
            //        break;
            //    case QTabBar::RoundedSouth:
            //    case QTabBar::TriangularSouth:
            //        rect.adjust(0,1, 0,0);
            //        break;
            //    case QTabBar::RoundedEast:
            //    case QTabBar::TriangularEast:
            //        rect.adjust(1,0, 0,0);
            //        break;
            //    case QTabBar::RoundedWest:
            //    case QTabBar::TriangularWest:
            //        rect.adjust(0,0, -1,0);
            //        break;
            //    }
            //}
        }
        break;

    case CE_ProgressBarGroove:
        name = "PROGRESS";
        partId = PP_BAR;
        stateId = 1;
        break;

    case CE_MenuEmptyArea:
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            int tab = menuitem->tabWidth;
            bool dis = !(menuitem->state & State_Enabled);
            bool act = menuitem->state & State_Selected;
            bool checkable = menuitem->menuHasCheckableItems;
            bool checked = checkable ? menuitem->checked : false;

            // windows always has a check column, regardless whether we have an icon or not
            int checkcol = qMax(menuitem->maxIconWidth, 20);

            int x, y, w, h;
            rect.getRect(&x, &y, &w, &h);

            QBrush fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            p->fillRect(rect, fill);

            if (element == CE_MenuEmptyArea)
                break;

            // draw separator -------------------------------------------------
            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                p->setPen(menuitem->palette.dark().color());
                p->drawLine(x, y + h/2, x+w, y + h/2);
                p->setPen(menuitem->palette.light().color());
                p->drawLine(x, y+1 + h/2, x+w, y+1 + h/2);
                return;
            }

            int xpos = x;
            QRect vrect = QRect(xpos, y, checkcol, h);

            // draw icon ------------------------------------------------------
            if (!menuitem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap = checked ?
                                 menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, QIcon::On) :
                                 menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                //if (act && !dis && !checked)
                //    qDrawShadePanel(p, vrect, menuitem->palette, false, 1, &menuitem->palette.brush(QPalette::Button));
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vrect.center());
                p->setPen(menuitem->palette.text().color());
                p->drawPixmap(pmr.topLeft(), pixmap);

                //int xp = xpos + checkcol + 1;
                //fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
                //p->fillRect(QRect(xp, y, w - checkcol - 1, h), fill);

            // draw checkmark -------------------------------------------------
            } else if (checked) {
                QStyleOptionMenuItem newMi = *menuitem;
                newMi.state = State_None;
                if (!dis)
                    newMi.state |= State_Enabled;
                if (act)
                    newMi.state |= State_On;
                newMi.rect = QRect(menuitem->rect.x() + windowsItemFrame, menuitem->rect.y() + windowsItemFrame,
                                   checkcol - 2 * windowsItemFrame, menuitem->rect.height() - 2*windowsItemFrame);
                drawPrimitive(PE_IndicatorMenuCheckMark, &newMi, p, widget);
            }

            QColor textColor = dis ? menuitem->palette.text().color() :
                               act ? menuitem->palette.highlightedText().color() : menuitem->palette.buttonText().color();
            p->setPen(textColor);

            // draw text ------------------------------------------------------
            int xm = windowsItemFrame + checkcol + windowsItemHMargin;
            xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
            QRect vTextRect = textRect;
            QString s = menuitem->text;
            if (!s.isEmpty()) {
                p->save();
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= (QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft);
                // draw tab text ----------------
                if (t >= 0) {
                    QRect vShortcutRect = QRect(textRect.topRight(), menuitem->rect.bottomRight());
                    if (dis && !act) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1,1,1,1), text_flags, s.mid(t + 1));
                        p->setPen(textColor);
                    }
                    p->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                p->setFont(font);
                if (dis && !act) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1,1,1,1), text_flags, s.left(t));
                    p->setPen(textColor);
                }
                p->drawText(vTextRect, text_flags, s);
                p->restore();
            }

            // draw sub menu arrow --------------------------------------------
            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {
                int dim = (h - 2 * windowsItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = QApplication::isRightToLeft() ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                xpos = x + w - windowsArrowHMargin - windowsItemFrame - dim;
                QRect vSubMenuRect = QRect(xpos, y + h / 2 - dim / 2, dim, dim);
                QStyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vSubMenuRect;
                newMI.state = dis ? State_None : State_Enabled;
                if (act)
                    newMI.palette.setColor(QPalette::ButtonText, newMI.palette.highlightedText().color());
                drawPrimitive(arrow, &newMI, p, widget);
            }
        }
        return;

    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            if (mbi->state == QStyleOptionMenuItem::DefaultItem)
                break;

            bool act = mbi->state & State_Selected;
            bool dis = !(mbi->state & State_Enabled);

            QBrush fill = mbi->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            QPalette::ColorRole textRole = dis ? QPalette::Text:
                                           act ? QPalette::HighlightedText : QPalette::ButtonText;
            QPixmap pix = mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal);

            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
            if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment |= Qt::TextHideMnemonic;

            p->fillRect(rect, fill);
            if (!pix.isNull())
                drawItemPixmap(p, mbi->rect, alignment, pix);
            else
                drawItemText(p, mbi->rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
        }
        return;

    case CE_DockWidgetTitle:
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option))
        {
            if (const QDockWidget *dw = qobject_cast<const QDockWidget *>(widget))
                if (!dw->isFloating()) {
                    QWindowsStyle::drawControl(element, option, p, widget);
                    return;
                }

            name = "WINDOW";
            if (dwOpt->state & State_Enabled)
                stateId = CS_ACTIVE;
            //else if (!(flags & QStyle::State_Enabled))
            //    stateId = CS_DISABLED;
            else
                stateId = CS_INACTIVE;

            int fw = pixelMetric(PM_DockWidgetFrameWidth, dwOpt, widget);
            int titleHeight = rect.height() - 2;
            rect = rect.adjusted(-fw, -fw, fw, 0);

            XPThemeData theme(widget, p, name, 0, stateId);
            if (!theme.isValid())
                break;

            // Draw small type title bar
            theme.rect = rect;
            theme.partId = WP_SMALLCAPTION;
            dd->drawBackground(theme);

            // Figure out maximal button space on title bar
            QSize closeSize = dwOpt->closable ? 
                standardPixmap(QStyle::SP_TitleBarCloseButton, dwOpt, widget).size() : QSize(0,0);
            QSize floatSize = dwOpt->floatable ? 
                standardPixmap(QStyle::SP_TitleBarMaxButton, dwOpt, widget).size() : QSize(0,0);

            int iconSize = qMax(closeSize.width(), closeSize.height());
            iconSize = qMax(iconSize, qMax(floatSize.width(), floatSize.height()));

            QIcon ico = widget->windowIcon();
            bool hasIcon = (ico.serialNumber() != qApp->windowIcon().serialNumber());
            int indent = fw;
            if (hasIcon) {
                QPixmap pxIco = ico.pixmap(titleHeight);
                p->drawPixmap(indent, rect.bottom() - titleHeight - 2, pxIco);
                indent += pxIco.width() + 1;
            }

            if (!dwOpt->title.isEmpty()) {
                p->setPen(Qt::white);
                QFont oldFont = p->font();
                QFont titleFont = oldFont;
                titleFont.setBold(true);
                p->setFont(titleFont);
                drawItemText(p, rect.adjusted(indent + 1, rect.bottom() - p->fontMetrics().lineSpacing() - 4,
                                              - (2 * iconSize), -1),
                             Qt::AlignLeft | Qt::AlignVCenter, dwOpt->palette,
                             dwOpt->state & State_Enabled, dwOpt->title);
                p->setFont(oldFont);
            }
            return;
        }
        break;

    default:
        break;
    }

    XPThemeData theme(widget, p, name, partId, stateId, rect);
    if (!theme.isValid()) {
        QWindowsStyle::drawControl(element, option, p, widget);
        return;
    }

    theme.rotate = rotate;
    theme.mirrorHorizontally = hMirrored;
    theme.mirrorVertically = vMirrored;
    dd->drawBackground(theme);
}


/*!
    \reimp
*/
void QWindowsXPStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *option,
                                         QPainter *p, const QWidget *widget) const
{
    if (!dd->useXP()) {
        QWindowsStyle::drawComplexControl(cc, option, p, widget);
        return;
    }

    State flags = option->state;
    SubControls sub = option->subControls;
    QRect r = option->rect;

    int partId = 0;
    int stateId = 0;
    if (widget->testAttribute(Qt::WA_UnderMouse) && widget->isActiveWindow())
        flags |= State_MouseOver;

    switch (cc) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
        {
            XPThemeData theme(widget, p, "SPIN");

            if (sb->frame && (sub & SC_SpinBoxFrame)) {
                partId = EP_EDITTEXT;
                if ((!flags & State_Enabled))
                    stateId = ETS_DISABLED;
                else if (flags & State_HasFocus)
                    stateId = ETS_FOCUSED;
                else
                    stateId = ETS_NORMAL;

                XPThemeData ftheme(widget, p, "EDIT", partId, stateId, r);
                ftheme.noContent = true;
                dd->drawBackground(ftheme);
            }
            if (sub & SC_SpinBoxUp) {
                theme.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);
                partId = SPNP_UP;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) || !(flags & State_Enabled))
                    stateId = UPS_DISABLED;
                else if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_Sunken))
                    stateId = UPS_PRESSED;
                else if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_MouseOver))
                    stateId = UPS_HOT;
                else
                    stateId = UPS_NORMAL;
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
            }
            if (sub & SC_SpinBoxDown) {
                theme.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);
                partId = SPNP_DOWN;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) || !(flags & State_Enabled))
                    stateId = DNS_DISABLED;
                else if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_Sunken))
                    stateId = DNS_PRESSED;
                else if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_MouseOver))
                    stateId = DNS_HOT;
                else
                    stateId = DNS_NORMAL;
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
            }
        }
        break;

    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            if (sub & SC_ComboBoxEditField) {
                partId = EP_EDITTEXT;
                if (!(flags & State_Enabled))
                    stateId = ETS_DISABLED;
                else if (flags & State_HasFocus)
                    stateId = ETS_FOCUSED;
                else
                    stateId = ETS_NORMAL;
                XPThemeData theme(widget, p, "EDIT", partId, stateId, r);

                dd->drawBackground(theme);
                if (!cmb->editable) {
                    QRect re = subControlRect(CC_ComboBox, option, SC_ComboBoxEditField, widget);
                    if (widget->hasFocus()) {
                        p->fillRect(re, option->palette.highlight());
                        p->setPen(option->palette.highlightedText().color());
                        p->setBackground(option->palette.highlight());
                    } else {
                        p->fillRect(re, option->palette.base());
                        p->setPen(option->palette.text().color());
                        p->setBackground(option->palette.base());
                    }
                }
            }

            if (sub & SC_ComboBoxArrow) {
                XPThemeData theme(widget, p, "COMBOBOX");
                theme.rect = subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget);
                partId = CP_DROPDOWNBUTTON;

                if (!(flags & State_Enabled))
                    stateId = CBXS_DISABLED;
                else if (cmb->activeSubControls == SC_ComboBoxArrow && (cmb->state & State_Sunken))
                    stateId = CBXS_PRESSED;
                else if (cmb->activeSubControls == SC_ComboBoxArrow && (cmb->state & State_MouseOver))
                    stateId = CBXS_HOT;
                else
                    stateId = CBXS_NORMAL;
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
            }
        }
        break;

    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            XPThemeData theme(widget, p, "SCROLLBAR");
            QScrollBar *bar = (QScrollBar*)widget;
            bool maxedOut = (bar->maximum() == bar->minimum());
            if (maxedOut)
                flags &= ~State_Enabled;

            bool isHorz = flags & State_Horizontal;
            bool isRTL  = option->direction == Qt::RightToLeft;
            if (sub & SC_ScrollBarAddLine) {
                theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget);
                partId = SBP_ARROWBTN;
                if (!(flags & State_Enabled))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTDISABLED : ABS_RIGHTDISABLED) : ABS_DOWNDISABLED);
                else if (scrollbar->activeSubControls & SC_ScrollBarAddLine && (scrollbar->state & State_Sunken))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTPRESSED : ABS_RIGHTPRESSED) : ABS_DOWNPRESSED);
                else if (scrollbar->activeSubControls & SC_ScrollBarAddLine && (scrollbar->state & State_MouseOver))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTHOT : ABS_RIGHTHOT) : ABS_DOWNHOT);
                else
                    stateId = (isHorz ? (isRTL ? ABS_LEFTNORMAL : ABS_RIGHTNORMAL) : ABS_DOWNNORMAL);
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
            }
            if (sub & SC_ScrollBarSubLine) {
                theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget);
                partId = SBP_ARROWBTN;
                if (!(flags & State_Enabled))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTDISABLED : ABS_LEFTDISABLED) : ABS_UPDISABLED);
                else if (scrollbar->activeSubControls & SC_ScrollBarSubLine && (scrollbar->state & State_Sunken))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTPRESSED : ABS_LEFTPRESSED) : ABS_UPPRESSED);
                else if (scrollbar->activeSubControls & SC_ScrollBarSubLine && (scrollbar->state & State_MouseOver))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTHOT : ABS_LEFTHOT) : ABS_UPHOT);
                else
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTNORMAL : ABS_LEFTNORMAL) : ABS_UPNORMAL);
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
            }
            if (maxedOut) {
                theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
                theme.rect = theme.rect.unite(subControlRect(CC_ScrollBar, option, SC_ScrollBarSubPage, widget));
                theme.rect = theme.rect.unite(subControlRect(CC_ScrollBar, option, SC_ScrollBarAddPage, widget));
                partId = bar->orientation() == Qt::Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                stateId = SCRBS_DISABLED;
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
            } else {
                if (sub & SC_ScrollBarSubPage) {
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSubPage, widget);
                    partId = flags & State_Horizontal ? SBP_UPPERTRACKHORZ : SBP_UPPERTRACKVERT;
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSubPage && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSubPage && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
                if (sub & SC_ScrollBarAddPage) {
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarAddPage, widget);
                    partId = flags & State_Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarAddPage && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarAddPage && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
                if (sub & SC_ScrollBarSlider) {
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSlider && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSlider && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;

                    // Draw handle
                    theme.rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
                    theme.partId = flags & State_Horizontal ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);

                    // Calculate rect of gripper
                    const int swidth = theme.rect.width();
                    const int sheight = theme.rect.height();

                    MARGINS contentsMargin;
                    RECT rect = theme.toRECT(theme.rect);
                    pGetThemeMargins(theme.handle(), 0, theme.partId, theme.stateId, TMT_SIZINGMARGINS, &rect, &contentsMargin);

                    SIZE size;
                    theme.partId = flags & State_Horizontal ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT;
                    pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                    int gw = size.cx, gh = size.cy;


                    QRect gripperBounds;
                    if (flags & State_Horizontal && ((swidth - contentsMargin.cxLeftWidth - contentsMargin.cxRightWidth) > gw)) {
                        gripperBounds.setLeft(theme.rect.left() + swidth/2 - gw/2);
                        gripperBounds.setTop(theme.rect.top() + sheight/2 - gh/2);
                        gripperBounds.setWidth(gw);
                        gripperBounds.setHeight(gh);
                    } else if ((sheight - contentsMargin.cyTopHeight - contentsMargin.cyBottomHeight) > gh) {
                        gripperBounds.setLeft(theme.rect.left() + swidth/2 - gw/2);
                        gripperBounds.setTop(theme.rect.top() + sheight/2 - gh/2);
                        gripperBounds.setWidth(gw);
                        gripperBounds.setHeight(gh);
                    }

                    // Draw gripper if there is enough space
                    if (!gripperBounds.isEmpty()) {
                        p->save();
                        XPThemeData grippBackground = theme;
                        grippBackground.partId = flags & State_Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                        theme.rect = gripperBounds;
                        p->setClipRegion(dd->region(theme));// Only change inside the region of the gripper
                        dd->drawBackground(grippBackground);// The gutter is the grippers background
                        dd->drawBackground(theme);          // Transparent gripper ontop of background
                        p->restore();
                    }
                }
            }
        }
        break;

#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            XPThemeData theme(widget, p, "TRACKBAR");
            QSlider *sl = (QSlider*)widget;
            QRegion tickreg = sl->rect();
            if (sub & SC_SliderGroove) {
                theme.rect = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
                if (slider->orientation == Qt::Horizontal) {
                    partId = TKP_TRACK;
                    stateId = TRS_NORMAL;
                    theme.rect = QRect(0, theme.rect.center().y() - 2, sl->width(), 4);
                } else {
                    partId = TKP_TRACKVERT;
                    stateId = TRVS_NORMAL;
                    theme.rect = QRect(theme.rect.center().x() - 2, 0, 4, sl->height());
                }
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
                tickreg -= theme.rect;
            }
            if (sub & SC_SliderTickmarks) {
                int tickOffset = pixelMetric(PM_SliderTickmarkOffset, slider, widget);
                int ticks = slider->tickPosition;
                int thickness = pixelMetric(PM_SliderControlThickness, slider, widget);
                int len = pixelMetric(PM_SliderLength, slider, widget);
                int available = pixelMetric(PM_SliderSpaceAvailable, slider, widget);
                int interval = slider->tickInterval;
                if (interval <= 0) {
                    interval = slider->singleStep;
                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,
                                                        available)
                        - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          0, available) < 3)
                        interval = slider->pageStep;
                }
                if (!interval)
                    interval = 1;
                int fudge = len / 2;
                int pos;
                int bothOffset = (ticks & QSlider::TicksAbove && ticks & QSlider::TicksBelow) ? 1 : 0;
                p->setPen(dd->sliderTickColor);
                int v = slider->minimum;
                while (v <= slider->maximum + 1) {
                    int tickLength = (v == slider->minimum || v >= slider->maximum) ? 4 : 3;
                    pos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum + 1,
                                                          v, available) + fudge;
                    if (slider->orientation == Qt::Horizontal) {
                        if (ticks & QSlider::TicksAbove)
                            p->drawLine(pos, tickOffset - 1 - bothOffset,
                                        pos, tickOffset - 1 - bothOffset - tickLength);

                        if (ticks & QSlider::TicksBelow)
                            p->drawLine(pos, tickOffset + thickness + bothOffset,
                                        pos, tickOffset + thickness + bothOffset + tickLength);
                    } else {
                        if (ticks & QSlider::TicksAbove)
                            p->drawLine(tickOffset - 1 - bothOffset, pos,
                                        tickOffset - 1 - bothOffset - tickLength, pos);

                        if (ticks & QSlider::TicksBelow)
                            p->drawLine(tickOffset + thickness + bothOffset, pos,
                                        tickOffset + thickness + bothOffset + tickLength, pos);
                    }
                    v += interval;
                }
            }
            if (sub & SC_SliderHandle) {
                theme.rect = subControlRect(CC_Slider, option, SC_SliderHandle, widget);
                p->fillRect(theme.rect, option->palette.background());
                if (slider->orientation == Qt::Horizontal) {
                    if (slider->tickPosition == QSlider::TicksAbove)
                        partId = TKP_THUMBTOP;
                    else if (slider->tickPosition == QSlider::TicksBelow)
                        partId = TKP_THUMBBOTTOM;
                    else
                        partId = TKP_THUMB;

                    if (!widget->isEnabled())
                        stateId = TUS_DISABLED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_Sunken))
                        stateId = TUS_PRESSED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_MouseOver))
                        stateId = TUS_HOT;
                    else if (flags & State_HasFocus)
                        stateId = TUS_FOCUSED;
                    else
                        stateId = TUS_NORMAL;
                } else {
                    if (slider->tickPosition == QSlider::TicksLeft)
                        partId = TKP_THUMBLEFT;
                    else if (slider->tickPosition == QSlider::TicksRight)
                        partId = TKP_THUMBRIGHT;
                    else
                        partId = TKP_THUMBVERT;

                    if (!widget->isEnabled())
                        stateId = TUVS_DISABLED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_Sunken))
                        stateId = TUVS_PRESSED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_MouseOver))
                        stateId = TUVS_HOT;
                    else if (flags & State_HasFocus)
                        stateId = TUVS_FOCUSED;
                    else
                        stateId = TUVS_NORMAL;
                }
                theme.partId = partId;
                theme.stateId = stateId;
                dd->drawBackground(theme);
            }
            //if (flags & State_HasFocus) {
            //    Q3StyleOptionFocusRect option(0);
            //    option.rect = subElementRect(SE_SliderFocusRect, sl);
            //    option.palette = pal;
            //    option.state = State_Default;
            //    drawPrimitive(PE_FrameFocusRect, p, re, pal);
            //}
        }
        break;
#endif

    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(option))
        {
            XPThemeData theme(widget, p, "TOOLBAR");
            QToolButton *tb = (QToolButton*)widget;

            QRect button, menuarea;
            button = subControlRect(cc, toolbutton, SC_ToolButton, widget);
            menuarea = subControlRect(cc, toolbutton, SC_ToolButtonMenu, widget);

            State bflags = flags, mflags = flags;

            if (toolbutton->activeSubControls == SC_ToolButton)
                bflags |= State_Sunken;
            else if (toolbutton->activeSubControls == SC_ToolButtonMenu)
                mflags |= State_Sunken;

            if (sub & SC_ToolButton) {
                theme.rect = subControlRect(CC_ToolButton, option, SC_ToolButton, widget);
                QWidget *pW = static_cast<QWidget *>(tb->parent());

                // ########## CE_ToolButtonLabel
                if (toolbutton->features & QStyleOptionToolButton::Arrow) {
                    Qt::ArrowType type = toolbutton->arrowType;

#define TBL_STATE(prefix) \
                    if (!tb->isEnabled()) \
                        stateId = prefix##_DISABLED; \
                    else if (bflags & (State_Sunken | State_On)) \
                        stateId = prefix##_PRESSED; \
                    else if (bflags & State_MouseOver) \
                        stateId = prefix##_HOT; \
                    else \
                        stateId = prefix##_NORMAL;

                    switch(type)
                    {
                    case Qt::RightArrow:
                        partId = SPNP_UPHORZ;
                        TBL_STATE(UPHZS);
                        break;
                    case Qt::LeftArrow:
                        partId = SPNP_DOWNHORZ;
                        TBL_STATE(DNHZS);
                        break;
                    case Qt::UpArrow:
                        partId = SPNP_UP;
                        TBL_STATE(UPS);
                        break;
                    case Qt::DownArrow:
                    default:
                        partId = SPNP_DOWN;
                        TBL_STATE(DNS);
                        break;
                    }
                    theme.name = "SPIN";
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                } else if (bflags & (State_Sunken | State_On | State_Raised)) {
                    if (sub & SC_ToolButtonMenu) {
                        partId = TP_SPLITBUTTON;
                        if (!flags & State_Enabled)
                            stateId = TS_DISABLED;
                        else if (flags & State_Sunken)
                            stateId = TS_PRESSED;
                        else if (flags & State_MouseOver)
                            stateId = flags & State_On ? TS_HOTCHECKED : TS_HOT;
                        else if (flags & State_On)
                            stateId = TS_CHECKED;
                        else
                            stateId = TS_NORMAL;

                        theme.partId = partId;
                        theme.stateId = stateId;
                        dd->drawBackground(theme);
                    } else {
                        if (!qobject_cast<QToolBar*>(widget->parentWidget()) && !(bflags & State_AutoRaise))
                            drawPrimitive(PE_PanelButtonBevel, option, p, widget);
                        else
                            drawPrimitive(PE_PanelButtonTool, option, p, widget);
                    }
                } else if (pW &&
                           !pW->palette().brush(pW->backgroundRole()).texture().isNull()) {
                    p->drawTiledPixmap(r, pW->palette().brush(pW->backgroundRole()).texture(), tb->pos());
                }
            }
            if (sub & SC_ToolButtonMenu) {
                QStyleOptionToolButton menuOption = *toolbutton;
                menuOption.rect = subControlRect(CC_ToolButton, option, SC_ToolButtonMenu, widget);
                drawPrimitive(PE_IndicatorButtonDropDown, &menuOption, p, widget);
            }

            QStyleOptionToolButton label = *toolbutton;
            int fw = pixelMetric(PM_DefaultFrameWidth, option, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);
            label.features &= ~QStyleOptionToolButton::Arrow;
            drawControl(CE_ToolButtonLabel, &label, p, widget);
            //if (tb->hasFocus() && !tb->focusProxy()) {
            //    Q3StyleOptionFocusRect option(0);
            //    option.rect = tb->rect();
            //    option.rect.adjust(3, 3, -3, -3);
            //    option.palette = pal;
            //    option.state = State_Default;
            //    drawPrimitive(PE_FrameFocusRect, &option, p, tb);
            //}
        }
        break;

    case CC_TitleBar:
        {
            if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
            {
                bool isActive = tb->titleBarState & QStyle::State_Active;
                XPThemeData theme(widget, p, "WINDOW");
                if (sub & SC_TitleBarLabel) {
                    theme.rect = option->rect;
                    partId = WP_CAPTION;
                    //partId = (tb->titleBarState & WindowMinimized)(titlebar->window() && titlebar->window()->isMinimized() ? WP_MINCAPTION : WP_CAPTION);
                    partId = (tb->titleBarState & Qt::WindowMinimized) ? WP_MINCAPTION : WP_CAPTION;
                    if (widget && !widget->isEnabled())
                        stateId = CS_DISABLED;
                    else if (isActive)
                        stateId = CS_ACTIVE;
                    else
                        stateId = CS_INACTIVE;

                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);

                    QRect ir = subControlRect(CC_TitleBar, tb, SC_TitleBarLabel, widget);

                    p->setPen(tb->palette.highlightedText().color());
                    p->drawText(ir.x() + 2, ir.y(), ir.width() - 2, ir.height(),
                                Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, tb->text);
                }

                if (sub & SC_TitleBarSysMenu && tb->titleBarFlags & Qt::WindowSystemMenuHint) {
                    theme.rect = subControlRect(CC_TitleBar, option, SC_TitleBarSysMenu, widget);
                    partId = WP_SYSBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = SBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarSysMenu && (option->state & State_Sunken))
                        stateId = SBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarSysMenu && (option->state & State_MouseOver))
                        stateId = SBS_HOT;
                    else
                        stateId = SBS_NORMAL;
                    if (!tb->icon.isNull()) {
                        tb->icon.paint(p, theme.rect);
                    } else {
                        theme.partId = partId;
                        theme.stateId = stateId;
                        dd->drawBackground(theme);
                    }
                }

                if (sub & SC_TitleBarMinButton && tb->titleBarFlags & Qt::WindowMinimizeButtonHint) {
                    theme.rect = subControlRect(CC_TitleBar, option, SC_TitleBarMinButton, widget);
                    partId = WP_MINBUTTON;
                    if (!widget->isEnabled())
                        stateId = MINBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarMinButton && (option->state & State_Sunken))
                        stateId = MINBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarMinButton && (option->state & State_MouseOver))
                        stateId = MINBS_HOT;
                    else if (!isActive) 
                        stateId = MINBS_INACTIVE;
                    else
                        stateId = MINBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
                if (sub & SC_TitleBarMaxButton && tb->titleBarFlags & Qt::WindowMaximizeButtonHint) {
                    theme.rect = subControlRect(CC_TitleBar, option, SC_TitleBarMaxButton, widget);
                    partId = WP_MAXBUTTON;
                    if (!widget->isEnabled())
                        stateId = MAXBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarMaxButton && (option->state & State_Sunken))
                        stateId = MAXBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarMaxButton && (option->state & State_MouseOver))
                        stateId = MAXBS_HOT;
                    else if (!isActive) 
                        stateId = MAXBS_INACTIVE;
                    else
                        stateId = MAXBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
                if (sub & SC_TitleBarNormalButton && tb->titleBarFlags & Qt::WindowMinimizeButtonHint) {
                    theme.rect = subControlRect(CC_TitleBar, option, SC_TitleBarNormalButton, widget);
                    partId = WP_RESTOREBUTTON;
                    if (!widget->isEnabled())
                        stateId = RBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarNormalButton && (option->state & State_Sunken))
                        stateId = RBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarNormalButton && (option->state & State_MouseOver))
                        stateId = RBS_HOT;
                    else if (!isActive) 
                        stateId = RBS_INACTIVE;
                    else
                        stateId = RBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
                if (sub & SC_TitleBarShadeButton && tb->titleBarFlags & Qt::WindowShadeButtonHint) {
                    theme.rect = subControlRect(CC_TitleBar, option, SC_TitleBarShadeButton, widget);
                    partId = WP_MINBUTTON;
                    if (!widget->isEnabled())
                        stateId = MINBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarShadeButton && (option->state & State_Sunken))
                        stateId = MINBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarShadeButton && (option->state & State_MouseOver))
                        stateId = MINBS_HOT;
                    else if (!isActive) 
                        stateId = MINBS_INACTIVE;
                    else
                        stateId = MINBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
                if (sub & SC_TitleBarUnshadeButton && tb->titleBarFlags & Qt::WindowShadeButtonHint) {
                    theme.rect = subControlRect(CC_TitleBar, option, SC_TitleBarUnshadeButton, widget);
                    partId = WP_RESTOREBUTTON;
                    if (!widget->isEnabled())
                        stateId = RBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarUnshadeButton && (option->state & State_Sunken))
                        stateId = RBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarUnshadeButton && (option->state & State_MouseOver))
                        stateId = RBS_HOT;
                    else if (!isActive) 
                        stateId = RBS_INACTIVE;
                    else
                        stateId = RBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
                if (sub & SC_TitleBarCloseButton) {
                    theme.rect = subControlRect(CC_TitleBar, option, SC_TitleBarCloseButton, widget);
                    //partId = titlebar->testWFlags(Qt::WA_WState_Tool) ? WP_SMALLCLOSEBUTTON : WP_CLOSEBUTTON;
                    partId = WP_CLOSEBUTTON;
                    if (!widget->isEnabled())
                        stateId = CBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarCloseButton && (option->state & State_Sunken))
                        stateId = CBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarCloseButton && (option->state & State_MouseOver))
                        stateId = CBS_HOT;
                    else if (!isActive) 
                        stateId = CBS_INACTIVE;
                    else
                        stateId = CBS_NORMAL;
                    theme.partId = partId;
                    theme.stateId = stateId;
                    dd->drawBackground(theme);
                }
            }
        }
        break;

    default:
        QWindowsStyle::drawComplexControl(cc, option, p, widget);
        break;
    }
}

/*! \reimp */
int QWindowsXPStyle::pixelMetric(PixelMetric pm, const QStyleOption *option, const QWidget *widget) const
{
    if (!dd->useXP())
        return QWindowsStyle::pixelMetric(pm, option, widget);

    int res = 0;
    switch (pm) {
    case PM_DockWidgetFrameWidth:
        {
            XPThemeData theme(widget, 0, "WINDOW", WP_SMALLFRAMERIGHT, FS_ACTIVE);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                res = size.cx;
            }
        }
        break;

    case PM_DockWidgetSeparatorExtent:
        res = 4;
        break;

    case PM_MenuBarPanelWidth:
        res = 0;
        break;

    case PM_MenuPanelWidth:
    case PM_DefaultFrameWidth:
    case PM_SpinBoxFrameWidth:
        res = 1;
        break;

    case PM_TabBarTabOverlap:
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        res = 2;
        break;

    case PM_TabBarBaseOverlap:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                res = 1;
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                res = 3;
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                res = 3;
                break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                res = 1;
                break;
            }
        }
        break;

    case PM_SplitterWidth:
        res = qMax(5, QApplication::globalStrut().width());
        break;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        {
            XPThemeData theme(widget, 0, "BUTTON", BP_CHECKBOX, CBS_UNCHECKEDNORMAL);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                res = (pm == PM_IndicatorWidth ? size.cx+2 : res = size.cy+2);
            }
        }
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        {
            XPThemeData theme(widget, 0, "BUTTON", BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                res = (pm == PM_ExclusiveIndicatorWidth ? size.cx+2 : res = size.cy+2);
            }
        }
        break;

    case PM_ProgressBarChunkWidth:
        {
            XPThemeData theme(widget, 0, "PROGRESS", PP_CHUNK);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                res = size.cx;
            }
        }
        break;

    case PM_ScrollBarExtent:
        {
            XPThemeData theme(widget, 0, "SCROLLBAR", SBP_LOWERTRACKHORZ);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                res = size.cy;
            }
        }
        break;

    case PM_SliderThickness:
        {
            XPThemeData theme(widget, 0, "TRACKBAR", TKP_THUMB);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                res = size.cy;
            }
        }
        break;

    case PM_MenuButtonIndicator:
        {
            XPThemeData theme(widget, 0, "TOOLBAR", TP_SPLITBUTTONDROPDOWN);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                res = size.cx;
            }
        }
        break;

    case PM_TitleBarHeight:
        {
            XPThemeData theme(widget, 0, "WINDOW", WP_CAPTION, CS_ACTIVE);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, WP_CAPTION, CS_ACTIVE, 0, TS_TRUE, &size);
                res = size.cy+1;
            }
        }
        break;

    case PM_MDIFrameWidth:
        {
            XPThemeData theme(widget, 0, "WINDOW", WP_FRAMELEFT, FS_ACTIVE);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), 0, WP_FRAMELEFT, FS_ACTIVE, 0, TS_TRUE, &size);
                res = size.cx-1;
            }
        }
        break;

    case PM_MDIMinimizedWidth:
        res = 160;
        break;

    default:
        res = QWindowsStyle::pixelMetric(pm, option, widget);
    }

    return res;
}

/*!
    \reimp
*/
QRect QWindowsXPStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *option,
                                      SubControl sc, const QWidget *widget) const
{
    if (!dd->useXP())
        return QWindowsStyle::subControlRect(cc, option, sc, widget);

    QRect rect;

    switch (cc) {
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            const bool isToolTitle = false; // widget->testWFlags(Qt::WA_WState_Tool)
            const int height = tb->rect.height();
            const int width = tb->rect.width();
            const int controlTop = 6; //widget->testWFlags(Qt::WA_WState_Tool) ? 4 : 6;
            const int controlHeight = height - controlTop - 3;

            const bool sysmenuHint  = (tb->titleBarFlags & Qt::WindowSystemMenuHint) != 0;
            const bool minimizeHint = (tb->titleBarFlags & Qt::WindowMinimizeButtonHint) != 0;
            const bool maximizeHint = (tb->titleBarFlags & Qt::WindowMaximizeButtonHint) != 0;
            const bool contextHint = (tb->titleBarFlags & Qt::WindowContextHelpButtonHint) != 0;
            const bool shadeHint = (tb->titleBarFlags & Qt::WindowShadeButtonHint) != 0;

            switch (sc) {
            case SC_TitleBarLabel:
                rect = QRect(0, 0, width, height);
                if (isToolTitle) {
                    if (sysmenuHint)
                        rect.adjust(0, 0, -controlHeight-3, 0);
                    if (minimizeHint || maximizeHint)
                        rect.adjust(0, 0, -controlHeight-2, 0);
                } else {
                    if (sysmenuHint)
                        rect.adjust(controlHeight+3, 0, -controlHeight-3, 0);
                    if (minimizeHint)
                        rect.adjust(0, 0, -controlHeight-2, 0);
                    if (maximizeHint)
                        rect.adjust(0, 0, -controlHeight-2, 0);
                    if (contextHint)
                        rect.adjust(0, 0, -controlHeight-2, 0);
                    if (shadeHint)
                        rect.adjust(0, 0, -controlHeight-2, 0);
                }
                break;

            case SC_TitleBarCloseButton:
                rect = QRect(width - (controlHeight + 2) - controlTop, controlTop, 
                             controlHeight, controlHeight);
                break;

            case SC_TitleBarMaxButton:
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                rect = QRect(width - ((controlHeight + 2) * 2) - controlTop, controlTop,
                             controlHeight, controlHeight);
                break;

            case SC_TitleBarMinButton:
            case SC_TitleBarNormalButton:
                {
                    int offset = controlHeight + 2;
                    if (!maximizeHint)
                        offset *= 2;
                    else
                        offset *= 3;
                    rect = QRect(width - offset - controlTop, controlTop,
                                 controlHeight, controlHeight);
                }
                break;

            case SC_TitleBarSysMenu:
                {
                    QSize iconSize = tb->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).size();
                    if (tb->icon.isNull())
                        iconSize = QSize(controlHeight, controlHeight);
                    int hPad = (controlHeight - iconSize.height())/2;
                    int vPad = (controlHeight - iconSize.width())/2;
                    rect = QRect(3 + vPad, controlTop + hPad, iconSize.width(), controlHeight - hPad);
                }
                break;
            }
        }
        break; 

    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            int x = 0, y = 0, wi = cmb->rect.width(), he = cmb->rect.height();
            int xpos = x;
            xpos += wi - 1 - 16;

            switch (sc) {
            case SC_ComboBoxFrame:
                rect = cmb->rect;
                break;

            case SC_ComboBoxArrow:
                rect = QRect(xpos, y+1, 16, he-2);
                break;

            case SC_ComboBoxEditField:
                rect = QRect(x+2, y+2, wi-3-16, he-4);
                break;

            case SC_ComboBoxListBoxPopup:
                rect = cmb->popupRect;
                break;
            }
        }
        break;

    default:
        rect = visualRect(option->direction, option->rect, 
                          QWindowsStyle::subControlRect(cc, option, sc, widget));
        break;
    }
    return visualRect(option->direction, option->rect, rect);
}

/*!
    \reimp
*/
QSize QWindowsXPStyle::sizeFromContents(ContentsType ct, const QStyleOption *option,
                                        const QSize &contentsSize, const QWidget *widget) const
{
    if (!dd->useXP())
        return QWindowsStyle::sizeFromContents(ct, option, contentsSize, widget);

    QSize sz(contentsSize);

    switch (ct) {
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                sz = QSize(10, windowsSepHeight);
                break;
            } else if (menuitem->icon.isNull()) {
                sz = QWindowsStyle::sizeFromContents(ct, option, sz, widget);
                sz.setHeight(sz.height() - 2);
                break;
            }
        }     
        // Otherwise, fall through
    default:
        sz = QWindowsStyle::sizeFromContents(ct, option, sz, widget);
        break;
    }

    return sz;
}


/*! \reimp */
int QWindowsXPStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                               QStyleHintReturn *returnData) const
{
    if (!dd->useXP())
        return QWindowsStyle::styleHint(hint, option, widget, returnData);

    int res = 0;
    switch (hint) {

    case SH_SpinControls_DisableOnBounds:
        res = 0;
        break;

    case SH_TitleBar_NoBorder:
        res = 1;
        break;

    case SH_GroupBox_TextLabelColor:
        if (widget->isEnabled())
            res = dd->groupBoxTextColor;
        else
            res = dd->groupBoxTextColorDisabled;
        break;

    case SH_Table_GridLineColor:
        res = 0xC0C0C0;
        break;

    case SH_LineEdit_PasswordCharacter:
        {
            const QFontMetrics &fm = widget->fontMetrics();
            if (fm.inFont(QChar(0x25CF)))
                res = 0x25CF;
            else if (fm.inFont(QChar(0x2022)))
                res = 0x2022;
            else
                res = '*';
        }
        break;

    case SH_WindowFrame_Mask:
        {
            res = 1;
            QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData);
            const QStyleOptionTitleBar *titlebar = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
            if (mask && titlebar) {
                XPThemeData themeData;
                if (titlebar->titleBarState & Qt::WindowMinimized) {
                    themeData = XPThemeData(widget, 0, "WINDOW", WP_MINCAPTION, CS_ACTIVE, option->rect);
                } else
                    themeData = XPThemeData(widget, 0, "WINDOW", WP_CAPTION, CS_ACTIVE, option->rect);
                mask->region = dd->region(themeData);
            }
        }
        break;

    default:
        res =QWindowsStyle::styleHint(hint, option, widget, returnData);
    }

    return res;
}

/*! \reimp */
QPalette QWindowsXPStyle::standardPalette() const
{
    return QWindowsStyle::standardPalette();
}

QPixmap QWindowsXPStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
                                        const QWidget *widget) const
{
    switch(standardPixmap) {
    case SP_TitleBarMaxButton:
    case SP_TitleBarCloseButton:
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option))
        {
            if (const QDockWidget *dw = qobject_cast<const QDockWidget *>(widget)) {
                if (dw->isFloating()) {
                    XPThemeData theme(widget, 0, "WINDOW", WP_SMALLCLOSEBUTTON, CBS_NORMAL);
                    if (theme.isValid()) {
                        SIZE sz;
                        pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &sz);
                        return QStyle::standardIcon(standardPixmap, option, widget).pixmap(QSize(sz.cx, sz.cy));
                    }
                }
            }
        }
        break;
    }
    return QWindowsStyle::standardPixmap(standardPixmap, option, widget);
}

QIcon QWindowsXPStyle::standardIconSlot(StandardPixmap standardIcon,
                                        const QStyleOption *option,
                                        const QWidget *widget) const
{
    switch(standardIcon) {
    case SP_TitleBarMaxButton:
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option))
        {
            if (dd->dockFloat.isNull()) {
                XPThemeData themeSize(0, 0, "WINDOW", WP_SMALLCLOSEBUTTON, CBS_NORMAL);
                XPThemeData theme(0, 0, "WINDOW", WP_MAXBUTTON, MAXBS_NORMAL);
                if (theme.isValid()) {
                    SIZE size;
                    pGetThemePartSize(themeSize.handle(), 0, themeSize.partId, themeSize.stateId, 0, TS_TRUE, &size);
                    QPixmap pm = QPixmap(size.cx, size.cy);
                    QPainter p(&pm);
                    theme.painter = &p;
                    theme.rect = QRect(0, 0, size.cx, size.cy);
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockFloat.addPixmap(pm, QIcon::Normal, QIcon::Off);    // Normal
                    theme.stateId = MAXBS_PUSHED;
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockFloat.addPixmap(pm, QIcon::Normal, QIcon::On);     // Pressed
                    theme.stateId = MAXBS_HOT;
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockFloat.addPixmap(pm, QIcon::Active, QIcon::Off);    // Hover
                    theme.stateId = MAXBS_DISABLED;
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockFloat.addPixmap(pm, QIcon::Disabled, QIcon::Off);  // Disabled
                }
            }
            if (widget)
                if (const QDockWidget *dw = qobject_cast<const QDockWidget *>(widget))
                    if (dw->isFloating())
                        return dd->dockFloat;

        }
        break;
    case SP_TitleBarCloseButton:
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(option))
        {
            if (dd->dockClose.isNull()) {
                XPThemeData theme(0, 0, "WINDOW", WP_SMALLCLOSEBUTTON, CBS_NORMAL);
                if (theme.isValid()) {
                    SIZE size;
                    pGetThemePartSize(theme.handle(), 0, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                    QPixmap pm = QPixmap(size.cx, size.cy);
                    QPainter p(&pm);
                    theme.painter = &p;
                    theme.partId = WP_CLOSEBUTTON; // ####
                    theme.rect = QRect(0, 0, size.cx, size.cy);
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockClose.addPixmap(pm, QIcon::Normal, QIcon::Off);    // Normal
                    theme.stateId = CBS_PUSHED;
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockClose.addPixmap(pm, QIcon::Normal, QIcon::On);     // Pressed
                    theme.stateId = CBS_HOT;
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockClose.addPixmap(pm, QIcon::Active, QIcon::Off);    // Hover
                    theme.stateId = CBS_DISABLED;
                    p.fillRect(pm.rect(), QColor(0xff, 0xff, 0xff, 0xff));
                    dd->drawBackground(theme);
                    dd->dockClose.addPixmap(pm, QIcon::Disabled, QIcon::Off);  // Disabled
                }
            }
            if (widget)
                if (const QDockWidget *dw = qobject_cast<const QDockWidget *>(widget))
                    if (dw->isFloating())
                        return dd->dockClose;
        }
        break;
    }

    return QWindowsStyle::standardIconSlot(standardIcon, option, widget);
}


// Debugging code ---------------------------------------------------------------------[ START ]---
// The code for this point on is not compiled by default, but only used as assisting
// debugging code when you uncomment the DEBUG_XP_STYLE define at the top of the file.

#ifdef DEBUG_XP_STYLE
// The schema file expects these to be defined by the user.
#define TMT_ENUMDEF 8
#define TMT_ENUMVAL TEXT('A')
#define TMT_ENUM    TEXT('B')
#define SCHEMA_STRINGS // For 2nd pass on schema file
#include <tmschema.h>

// A property's value, type and name combo
struct PropPair {
    int propValue;
    int propType;
    LPCWSTR propName;
};

// Operator for sorting of PropPairs
bool operator<(PropPair a, PropPair b) {
    return wcscmp(a.propName, b.propName) < 0;
}

// Our list of all possible properties
static QList<PropPair> all_props;


/*! \internal
    Dumps a portion of the full native DIB section double buffer.
    The DIB section double buffer is only used when doing special
    transformations to the theme part, or when the real double
    buffer in the paintengine does not have an HDC we may use
    directly.
    Since we cannot rely on the pixel data we get from Microsoft
    when drawing into the DIB section, we use this function to
    see the actual data we got, and can determin the appropriate
    action.
*/
void QWindowsXPStylePrivate::dumpNativeDIB(int w, int h)
{
    if (w && h) {
        static int pCount = 0;
        DWORD *bufPix = (DWORD*)bufferPixels;

        char *bufferDump = new char[bufferH * bufferW * 16];
        char *bufferPos = bufferDump;
     
        memset(bufferDump, 0, sizeof(bufferDump));
        bufferPos += sprintf(bufferPos, "const int pixelBufferW%d = %d;\n", pCount, w);
        bufferPos += sprintf(bufferPos, "const int pixelBufferH%d = %d;\n", pCount, h);
        bufferPos += sprintf(bufferPos, "const unsigned DWORD pixelBuffer%d[] = {", pCount);
        for (int iy = 0; iy < h; ++iy) {
            bufferPos += sprintf(bufferPos, "\n    ");
            bufPix = (DWORD*)(bufferPixels + (iy * bufferW * 4));
            for (int ix = 0; ix < w; ++ix) {
                bufferPos += sprintf(bufferPos, "0x%08x, ", *bufPix);
                ++bufPix;
            }
        }
        bufferPos += sprintf(bufferPos, "\n};\n\n");
        printf(bufferDump);

        delete bufferDump;
        ++pCount;
    }
}

/*! \internal
    Shows the value of a given property for a part.
*/
static void showProperty(XPThemeData &themeData, const PropPair &prop)
{
    PROPERTYORIGIN origin = PO_NOTFOUND;
    pGetThemePropertyOrigin(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &origin);
    const char *originStr;
    switch(origin) {
    case PO_STATE:
        originStr = "State ";
        break;
    case PO_PART:
        originStr = "Part  ";
        break;
    case PO_CLASS:
        originStr = "Class ";
        break;
    case PO_GLOBAL:
        originStr = "Globl ";
        break;
    case PO_NOTFOUND:
    default:
        originStr = "Unkwn ";
        break;
    }

    switch(prop.propType) {
    case TMT_STRING:
        {
            wchar_t buffer[512];
            pGetThemeString(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, buffer, 512);
            printf("  (%sString)  %-20S: %S\n", originStr, prop.propName, buffer);
        }
        break;
    case TMT_ENUM:
        {
            int result = -1;
            pGetThemeEnumValue(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%sEnum)    %-20S: %d\n", originStr, prop.propName, result);
        }
        break;
    case TMT_INT:
        {
            int result = -1;
            pGetThemeInt(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%sint)     %-20S: %d\n", originStr, prop.propName, result);
        }
        break;
    case TMT_BOOL:
        {
            BOOL result = false;
            pGetThemeBool(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%sbool)    %-20S: %d\n", originStr, prop.propName, result);
        }
        break;
    case TMT_COLOR:
        {
            COLORREF result = 0;
            pGetThemeColor(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%scolor)   %-20S: 0x%08X\n", originStr, prop.propName, result);
        }
        break;
    case TMT_MARGINS:
        {
            MARGINS result;
            memset(&result, 0, sizeof(result));
            pGetThemeMargins(themeData.handle(), 0, themeData.partId, themeData.stateId, prop.propValue, 0, &result);
            printf("  (%smargins) %-20S: (%d, %d, %d, %d)\n", originStr, 
                   prop.propName, result.cxLeftWidth, result.cyTopHeight, result.cxRightWidth, result.cyBottomHeight);
        }
        break;
    case TMT_FILENAME:
        {
            wchar_t buffer[512];
            pGetThemeFilename(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, buffer, 512);
            printf("  (%sfilename)%-20S: %S\n", originStr, prop.propName, buffer);
        }
        break;
    case TMT_SIZE:
        {
            SIZE result1;
            SIZE result2;
            SIZE result3;
            memset(&result1, 0, sizeof(result1));
            memset(&result2, 0, sizeof(result2));
            memset(&result3, 0, sizeof(result3));
            pGetThemePartSize(themeData.handle(), 0, themeData.partId, themeData.stateId, 0, TS_MIN,  &result1);
            pGetThemePartSize(themeData.handle(), 0, themeData.partId, themeData.stateId, 0, TS_TRUE, &result2);
            pGetThemePartSize(themeData.handle(), 0, themeData.partId, themeData.stateId, 0, TS_DRAW, &result3);
            printf("  (%ssize)    %-20S: Min (%d, %d),  True(%d, %d),  Draw(%d, %d)\n", originStr, prop.propName,
                   result1.cx, result1.cy, result2.cx, result2.cy, result3.cx, result3.cy);
        }
        break;
    case TMT_POSITION:
        {
            POINT result;
            memset(&result, 0, sizeof(result));
            pGetThemePosition(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%sPosition)%-20S: (%d, %d)\n", originStr, prop.propName, result.x, result.y);
        }
        break;
    case TMT_RECT:
        {
            RECT result;
            memset(&result, 0, sizeof(result));
            pGetThemeRect(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%sRect)    %-20S: (%d, %d, %d, %d)\n", originStr, prop.propName, result.left, result.top, result.right, result.bottom);
        }
        break;
    case TMT_FONT:
        {
            LOGFONT result;
            memset(&result, 0, sizeof(result));
            pGetThemeFont(themeData.handle(), 0, themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%sFont)    %-20S: %S  height(%d)  width(%d)  weight(%d)\n", originStr, prop.propName,
                   result.lfFaceName, result.lfHeight, result.lfWidth, result.lfWeight);
        }
        break;
    case TMT_INTLIST:
        {
            INTLIST result;
            memset(&result, 0, sizeof(result));
            pGetThemeIntList(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &result);
            printf("  (%sInt list)%-20S: { ", originStr, prop.propName);
            for (int i = 0; i < result.iValueCount; ++i)
                printf("%d ", result.iValues[i]);
            printf("}\n");
        }
        break;
    default:
        printf("    %s%S : Unknown property type (%d)!\n", originStr, prop.propName, prop.propType);
    }
}

/*! \internal
    Dump all valid properties for a part.
    If it's the first time this function is called, then the name,
    enum value and documentation of all properties are shown, as
    well as all global properties.
*/
void QWindowsXPStylePrivate::showProperties(XPThemeData &themeData)
{
    if (!all_props.count()) {
        const TMSCHEMAINFO *infoTable = GetSchemaInfo();
        for (int i = 0; i < infoTable->iPropCount; ++i) {
            int propType  = infoTable->pPropTable[i].bPrimVal;
            int propValue = infoTable->pPropTable[i].sEnumVal;
            LPCWSTR propName = infoTable->pPropTable[i].pszName;

            switch(propType) {
            case TMT_ENUMDEF:
            case TMT_ENUMVAL:
                continue;
            default:
                if (propType != propValue) {
                    PropPair prop;
                    prop.propValue = propValue;
                    prop.propName  = propName;
                    prop.propType  = propType;
                    all_props.append(prop);
                }
            }
        }
        qSort(all_props);

        {// List all properties
            printf("part properties count = %d:\n", all_props.count());
            printf("      Enum  Property Name        Description\n");
            printf("-----------------------------------------------------------\n");
            wchar_t themeName[256];
            pGetCurrentThemeName(themeName, 256, 0, 0, 0, 0);
            for (int j = 0; j < all_props.count(); ++j) {
                PropPair prop = all_props.at(j);
                wchar_t buf[500];
                pGetThemeDocumentationProperty(themeName, prop.propName, buf, 500);
                printf("%3d: (%4d) %-20S %S\n", j, prop.propValue, prop.propName, buf);
            }
        }

        {// Show Global values
            printf("Global Properties:\n");
            for (int j = 0; j < all_props.count(); ++j) {
                PropPair prop = all_props.at(j);
                PROPERTYORIGIN origin = PO_NOTFOUND;
                pGetThemePropertyOrigin(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &origin);
                if (origin == PO_GLOBAL) {
                    showProperty(themeData, prop);
                }
            }
        }
    }

    for (int j = 0; j < all_props.count(); ++j) {
        PropPair prop = all_props.at(j);
        PROPERTYORIGIN origin = PO_NOTFOUND;
        pGetThemePropertyOrigin(themeData.handle(), themeData.partId, themeData.stateId, prop.propValue, &origin);
        if (origin != PO_NOTFOUND) 
        {
            showProperty(themeData, prop);
        }
    }
}
#endif
// Debugging code -----------------------------------------------------------------------[ END ]---


#endif //QT_NO_WINDOWSXP
