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

#ifndef QT_X11_P_H
#define QT_X11_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qwindowdefs.h"
#include "QtCore/qlist.h"

// the following is necessary to work around breakage in many versions
// of XFree86's Xlib.h still in use
// ### which versions?
#if defined(_XLIB_H_) // crude hack, but...
#error "cannot include <X11/Xlib.h> before this file"
#endif
#define XRegisterIMInstantiateCallback qt_XRegisterIMInstantiateCallback
#define XUnregisterIMInstantiateCallback qt_XUnregisterIMInstantiateCallback
#define XSetIMValues qt_XSetIMValues
#include <X11/Xlib.h>
#undef XRegisterIMInstantiateCallback
#undef XUnregisterIMInstantiateCallback
#undef XSetIMValues

#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

//#define QT_NO_SHAPE
#ifdef QT_NO_SHAPE
#  define XShapeCombineRegion(a,b,c,d,e,f,g)
#  define XShapeCombineMask(a,b,c,d,e,f,g)
#else
#  include <X11/extensions/shape.h>
#endif // QT_NO_SHAPE


#if !defined (QT_NO_TABLET)
#  include <X11/extensions/XInput.h>
#if defined (Q_OS_IRIX)
#  include <X11/extensions/SGIMisc.h>
#  include <wacom.h>
#endif
#endif // QT_NO_TABLET


// #define QT_NO_XINERAMA
#ifndef QT_NO_XINERAMA
#  if 0 // ### Xsun, but how to detect it?
// Xinerama is only supported in Solaris 7 with patches 107648/108376 and
// Solaris 8 or above which introduce the X11R6.4 Xserver.
// To switch the Xinerama functionality on, you need to add the "+xinerama"
// argument to the Xsun start line.
// At least Solaris 7 and 8 are missing Xinerama system headers and function
// declarations (bug 4284701).
// The Xinerama API is not documented. In theory it could change but it
// probably won't because Sun are using it in at least dtlogin (bug 4221829).
extern "C" Bool XPanoramiXQueryExtension(
    Display*,
    int*,
    int*
);
extern "C" Status XPanoramiXQueryVersion(
    Display*,
    int*,
    int*
);
extern "C" Status XPanoramiXGetState(
    Display*,
    Drawable,
    XPanoramiXInfo*
);
extern "C" Status XPanoramiXGetScreenCount(
    Display *,
    Drawable,
    XPanoramiXInfo*
);
extern "C" Status XPanoramiXGetScreenSize(
    Display*,
    Drawable,
    int,
    XPanoramiXInfo*
);
#  else // XFree86
// XFree86 does not C++ify Xinerama (at least up to XFree86 4.0.3).
extern "C" {
#    include <X11/extensions/Xinerama.h>
}
#  endif
#endif // QT_NO_XINERAMA

// #define QT_NO_XRANDR
#ifndef QT_NO_XRANDR
#  include <X11/extensions/Xrandr.h>
#endif // QT_NO_XRANDR

// #define QT_NO_XRENDER
#ifndef QT_NO_XRENDER
#  include <X11/extensions/Xrender.h>
#if defined(RENDER_MAJOR) && RENDER_MAJOR == 0
# if RENDER_MINOR < 5
#  define QT_NO_XRENDER
# endif
#endif
#endif // QT_NO_XRENDER

// #define QT_NO_XKB
#ifndef QT_NO_XKB
#  include <X11/XKBlib.h>
#endif // QT_NO_XKB


#if !defined(XlibSpecificationRelease)
#  define X11R4
typedef char *XPointer;
#else
#  undef X11R4
#endif

// #define QT_NO_XIM
#if defined(X11R4)
// X11R4 does not have XIM
#define QT_NO_XIM
#elif defined(Q_OS_OSF) && (XlibSpecificationRelease < 6)
// broken in Xlib up to OSF/1 3.2
#define QT_NO_XIM
#elif defined(Q_OS_AIX)
// broken in Xlib up to what version of AIX?
#define QT_NO_XIM
#elif defined(QT_NO_DEBUG) && defined(Q_OS_IRIX)
// XmbLookupString broken on IRIX
// XCreateIC broken when compiling -64 on IRIX 6.5.2
#define QT_NO_XIM
#elif defined(Q_OS_HPUX) && defined(__LP64__)
// XCreateIC broken when compiling 64-bit ELF on HP-UX 11.0
#define QT_NO_XIM
#elif defined(Q_OS_SCO)
// ### suggested by user...
// ### #define QT_NO_XIM
#endif // QT_NO_XIM


/*
 * Solaris patch 108652-47 and higher fixes crases in
 * XRegisterIMInstantiateCallback, but the function doesn't seem to
 * work.
 *
 * Instead, we disabled R6 input, and open the input method
 * immediately at application start.
 */
#if !defined(QT_NO_XIM) && (XlibSpecificationRelease >= 6) && \
    !defined(Q_OS_SOLARIS)
#define USE_X11R6_XIM

//######### XFree86 has wrong declarations for XRegisterIMInstantiateCallback
//######### and XUnregisterIMInstantiateCallback in at least version 3.3.2.
//######### Many old X11R6 header files lack XSetIMValues.
//######### Therefore, we have to declare these functions ourselves.

extern "C" Bool XRegisterIMInstantiateCallback(
    Display*,
    struct _XrmHashBucketRec*,
    char*,
    char*,
    XIMProc, //XFree86 has XIDProc, which has to be wrong
    XPointer
);

extern "C" Bool XUnregisterIMInstantiateCallback(
    Display*,
    struct _XrmHashBucketRec*,
    char*,
    char*,
    XIMProc, //XFree86 has XIDProc, which has to be wrong
    XPointer
);

extern "C" char *XSetIMValues(XIM /* im */, ...);

#endif

#ifndef QT_NO_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#ifndef QT_NO_XIM
// some platforms (eg. Solaris 2.51) don't have these defines in Xlib.h
#ifndef XNResetState
#define XNResetState "resetState"
#endif
#ifndef XIMPreserveState
#define XIMPreserveState (1L<<1)
#endif
#endif


#ifndef X11R4
#  include <X11/Xlocale.h>
#endif // X11R4


#ifdef QT_MITSHM
#  include <X11/extensions/XShm.h>
#endif // QT_MITSHM

class QWidget;


struct QX11InfoData {
    uint ref;
    int screen;
    int dpiX;
    int dpiY;
    int depth;
    int cells;
    Colormap colormap;
    Visual *visual;
    bool defaultColormap;
    bool defaultVisual;
    int subpixel;
};



struct QX11Data;
extern QX11Data *qt_x11Data;

struct QX11Data
{
    Window findClientWindow(Window, Atom, bool);

    // from qclipboard_x11.cpp
    bool clipboardWaitForEvent(Window win, int type, XEvent *event, int timeout);
    bool clipboardReadProperty(Window win, Atom property, bool deleteProperty,
                            QByteArray *buffer, int *size, Atom *type, int *format, bool nullterm);
    QByteArray clipboardReadIncrementalProperty(Window win, Atom property, int nbytes, bool nullterm);

    // from qdnd_x11.cpp
    bool dndEnable(QWidget* w, bool on);
    static void xdndSetup();
    void xdndHandleEnter(QWidget *, const XEvent *, bool);
    void xdndHandlePosition(QWidget *, const XEvent *, bool);
    void xdndHandleStatus(QWidget *, const XEvent *, bool);
    void xdndHandleLeave(QWidget *, const XEvent *, bool);
    void xdndHandleDrop(QWidget *, const XEvent *, bool);
    void xdndHandleFinished(QWidget *, const XEvent *, bool);
    void xdndHandleSelectionRequest(const XSelectionRequestEvent *);
    static bool xdndHandleBadwindow();
    QByteArray xdndAtomToString(Atom a);
    Atom xdndStringToAtom(const char *);

    // from qmotifdnd_x11.cpp
    void motifdndHandle(QWidget *, const XEvent *, bool);
    void motifdndEnable(QWidget *, bool);
    QByteArray motifdndObtainData(const char *format);
    QByteArray motifdndFormat(int n);
    bool motifdnd_active;

    Display *display;
    char *displayName;
    bool foreignDisplay;
    // current focus model
    enum {
        FM_Unknown = -1,
        FM_Other = 0,
        FM_PointerRoot = 1
    };
    int focus_model;

    // true if Qt is compiled w/ RANDR support and RANDR is supported on the connected Display
    bool use_xrandr;
    int xrandr_major;
    int xrandr_eventbase;
    int xrandr_errorbase;

    // true if Qt is compiled w/ RENDER support and RENDER is supported on the connected Display
    bool use_xrender;
    int xrender_major;
    int xrender_eventbase;
    int xrender_errorbase;

    // true if Qt is compiled w/ Tablet support and we have a tablet.
    bool use_xinput;
    int xinput_major;
    int xinput_eventbase;
    int xinput_errorbase;

    QList<QWidget *> deferred_map;
    struct ScrollInProgress {
        long id;
        QWidget* scrolled_widget;
        int dx, dy;
    };
    long sip_serial;
    QList<ScrollInProgress> sip_list;

    // window managers list of supported "stuff"
    Atom *net_supported_list;
    // list of virtual root windows
    Window *net_virtual_root_list;
    // client leader window
    Window wm_client_leader;

    QX11InfoData *screens;
    int screenCount;
    int defaultScreen;

    Time time;
    Time userTime;

    QString default_im;

    // starts to ignore bad window errors from X
    static inline void ignoreBadwindow() {
        qt_x11Data->ignore_badwindow = true;
        qt_x11Data->seen_badwindow = false;
    }

    // ends ignoring bad window errors and returns whether an error had happened.
    static inline bool badwindow() {
        qt_x11Data->ignore_badwindow = false;
        return qt_x11Data->seen_badwindow;
    }

    bool ignore_badwindow;
    bool seen_badwindow;

    // options
    int visual_class;
    int visual_id;
    int color_count;
    bool custom_cmap;

    // outside visual/colormap
    Visual *visual;
    Colormap colormap;

#ifndef QT_NO_XRENDER
    enum { solid_fill_count = 16 };
    struct SolidFills {
        XRenderColor color;
        int screen;
        Picture picture;
    } solid_fills[solid_fill_count];
    enum { pattern_fill_count = 16 };
    struct PatternFills {
        XRenderColor color;
        XRenderColor bg_color;
        int screen;
        int style;
        bool opaque;
        Picture picture;
    } pattern_fills[pattern_fill_count];
    Picture getSolidFill(int screen, const QColor &c);
    XRenderColor preMultiply(const QColor &c);
#endif

    bool has_fontconfig;
    qreal fc_scale;
    bool fc_antialias;

    char *startupId;

    /* Warning: if you modify this list, modify the names of atoms in qapplication_x11.cpp as well! */
    enum X11Atom {
        // window-manager <-> client protocols
        WM_PROTOCOLS,
        WM_DELETE_WINDOW,
        WM_TAKE_FOCUS,
        _NET_WM_PING,
        _NET_WM_CONTEXT_HELP,

        // ICCCM window state
        WM_STATE,
        WM_CHANGE_STATE,

        // Session management
        WM_CLIENT_LEADER,
        WM_WINDOW_ROLE,
        SM_CLIENT_ID,

        // Clipboard
        CLIPBOARD,
        INCR,
        TARGETS,
        MULTIPLE,
        TIMESTAMP,
        CLIP_TEMPORARY,
        _QT_SELECTION,
        _QT_CLIPBOARD_SENTINEL,
        _QT_SELECTION_SENTINEL,

        RESOURCE_MANAGER,

        _XSETROOT_ID,

        _QT_SCROLL_DONE,
        _QT_INPUT_ENCODING,
        _QT_SIZEGRIP,

        _MOTIF_WM_HINTS,

        DTWM_IS_RUNNING,
        KWIN_RUNNING,
        KWM_RUNNING,
        GNOME_BACKGROUND_PROPERTIES,
        ENLIGHTENMENT_DESKTOP,
        _SGI_DESKS_MANAGER,

        // EWMH (aka NETWM)
        _NET_SUPPORTED,
        _NET_VIRTUAL_ROOTS,
        _NET_WORKAREA,

        _NET_WM_NAME,
        _NET_WM_ICON_NAME,
        _NET_WM_ICON,

        _NET_WM_PID,

        _NET_WM_WINDOW_OPACITY,

        _NET_WM_STATE,
        _NET_WM_STATE_ABOVE,
        _NET_WM_STATE_FULLSCREEN,
        _NET_WM_STATE_MAXIMIZED_HORZ,
        _NET_WM_STATE_MAXIMIZED_VERT,
        _NET_WM_STATE_MODAL,
        _NET_WM_STATE_STAYS_ON_TOP,

        _NET_WM_USER_TIME,

        _NET_WM_WINDOW_TYPE,
        _NET_WM_WINDOW_TYPE_DIALOG,
        _NET_WM_WINDOW_TYPE_MENU,
        _NET_WM_WINDOW_TYPE_NORMAL,
        _KDE_NET_WM_WINDOW_TYPE_OVERRIDE,
        _NET_WM_WINDOW_TYPE_SPLASH,
        _NET_WM_WINDOW_TYPE_TOOLBAR,
        _NET_WM_WINDOW_TYPE_UTILITY,

        _KDE_NET_WM_FRAME_STRUT,

        _NET_STARTUP_INFO,
        _NET_STARTUP_INFO_BEGIN,

        // Property formats
        COMPOUND_TEXT,
        TEXT,
        UTF8_STRING,

        // Xdnd
        XdndEnter,
        XdndPosition,
        XdndStatus,
        XdndLeave,
        XdndDrop,
        XdndFinished,
        XdndTypelist,
        XdndActionList,

        XdndSelection,

        XdndAware,
        XdndProxy,

        XdndActionCopy,
        XdndActionLink,
        XdndActionMove,
        XdndActionPrivate,

        // Motif DND
        _MOTIF_DRAG_AND_DROP_MESSAGE,
        _MOTIF_DRAG_INITIATOR_INFO,
        _MOTIF_DRAG_RECEIVER_INFO,
        _MOTIF_DRAG_WINDOW,
        _MOTIF_DRAG_TARGETS,

        XmTRANSFER_SUCCESS,
        XmTRANSFER_FAILURE,

        NPredefinedAtoms,

        _QT_SETTINGS_TIMESTAMP = NPredefinedAtoms,
        NAtoms
    };
    Atom atoms[NAtoms];
};

extern QX11Data *qt_x11Data;
#define ATOM(x) qt_x11Data->atoms[QX11Data::x]
#define X11 qt_x11Data

// rename a couple of X defines to get rid of name clashes
// resolve the conflict between X11's FocusIn and QEvent::FocusIn
enum {
    XFocusOut = FocusOut,
    XFocusIn = FocusIn,
    XKeyPress = KeyPress,
    XKeyRelease = KeyRelease,
    XNone = None,
    XRevertToParent = RevertToParent,
    XGrayScale = GrayScale,
    XCursorShape = CursorShape
};
#undef FocusOut
#undef FocusIn
#undef KeyPress
#undef KeyRelease
#undef None
#undef RevertToParent
#undef GrayScale
#undef CursorShape

#ifdef FontChange
#undef FontChange
#endif

Q_DECLARE_TYPEINFO(XPoint, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(XRectangle, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(XChar2b, Q_PRIMITIVE_TYPE);
#ifndef QT_NO_XRENDER
Q_DECLARE_TYPEINFO(XGlyphElt32, Q_PRIMITIVE_TYPE);
#endif

#endif // QT_X11_P_H
