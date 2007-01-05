/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWINDOWSXPSTYLE_P_H
#define QWINDOWSXPSTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qwindowsxpstyle.h"
#include "qwindowsstyle_p.h"
#include <qmap.h>
#include <qt_windows.h>

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


// Uncomment define below to build debug assisting code, and output
// #define DEBUG_XP_STYLE

#if !defined(QT_NO_STYLE_WINDOWSXP)

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

class QWindowsXPStylePrivate : public QWindowsStylePrivate
{
    Q_DECLARE_PUBLIC(QWindowsXPStyle)
public:
    QWindowsXPStylePrivate()
        : QWindowsStylePrivate(), hasInitColors(false), bufferDC(0), bufferBitmap(0), nullBitmap(0),
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
    static bool useXP(bool update = false);

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
    bool hasInitColors;

    static QMap<QString,HTHEME> *handleMap;

    QIcon dockFloat, dockClose;

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
};

#endif // QT_NO_STYLE_WINDOWS
#endif //QWINDOWSXPSTYLE_P_H
