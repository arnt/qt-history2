/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_X11_H
#define QT_X11_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of q*_x11.cpp.  This header file may change from version to version
// without notice, or even be removed.
//
//


#ifndef QT_H
#include "qwindowdefs.h"
#include "qlist.h"
#endif // QT_H

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
#define XShapeCombineRegion(a,b,c,d,e,f,g)
#define XShapeCombineMask(a,b,c,d,e,f,g)
#else
#include <X11/extensions/shape.h>
#endif // QT_NO_SHAPE


// the wacom tablet (currently just the IRIX version)
#if defined (QT_TABLET_SUPPORT)
#  include <X11/extensions/XInput.h>
#if defined (Q_OS_IRIX)
#  include <wacom.h>  // wacom driver defines for IRIX (quite handy)
#endif
#endif // QT_TABLET_SUPPORT


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
// #define QT_NO_XFTFREETYPE
#  ifndef QT_NO_XFTFREETYPE
#    ifdef QT_USE_XFT2_HEADER
#      include <X11/Xft/Xft2.h>
#    else
#      include <X11/Xft/Xft.h>
#    endif // QT_USE_XFT2_HEADER
#    if defined(XFT_VERSION) && XFT_VERSION >= 20000
#      define QT_XFT2
#    else
#      include <X11/Xft/XftFreetype.h>
#    endif // XFT_VERSION
#  endif // QT_NO_XFTFREETYPE
#else
// make sure QT_NO_XFTFREETYPE is defined if QT_NO_XRENDER is defined
#  ifndef QT_NO_XFTFREETYPE
#    define QT_NO_XFTFREETYPE
#  endif
#endif // QT_NO_XRENDER


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

extern "C" char *XSetIMValues( XIM /* im */, ... );

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

struct QX11Data
{
    Display *display;
    char *displayName;
    bool foreignDisplay;
#if !defined(QT_NO_XIM)
    XIM xim;
    XIMStyle xim_style;
#endif
    // current focus model
    enum {
	FM_Unknown = -1,
	FM_Other = 0,
	FM_PointerRoot = 1
    };
    int focus_model;
    // TRUE if Qt is compiled w/ XRandR support and XRandR exists on the connected Display
    bool use_xrandr;
    int xrandr_eventbase;
    // TRUE if Qt is compiled w/ XRender support and XRender exists on the connected Display
    bool use_xrender;
    bool has_xft;
    bool use_antialiasing;
    bool xftDone;
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


    /* Warning: if you modify this list, modify the names of atoms in qapplication_x11.cpp as well! */
    enum X11Atoms {
	Wm_Protocols,
	Wm_Delete_Window,
	Wm_State,
	Wm_Change_State,
	Wm_Take_Focus,
	Wm_Client_Leader,
	Wm_Window_Role,
	Sm_Client_Id,
	Clipboard,
	Resource_Manager,
	Incr,
	Xsetroot_Id,
	Qt_Selection,
	Qt_Clipboard_Sentinel,
	Qt_Selection_Sentinel,
	Qt_Scroll_Done,
	Qt_Input_Encoding,
	Qt_Sizegrip,
	Net_Wm_Context_Help,
	Net_Wm_Ping,
	Motif_Wm_Hints,
	Dtwm_Is_Running,
	Kwin_Running,
	Kwm_Running,
	Gnome_Background_Properties,
	Net_Supported,
	Net_Virtual_Roots,
	Net_Workarea,
	Net_Wm_State,
	Net_Wm_State_Modal,
	Net_Wm_State_Maximized_Vert,
	Net_Wm_State_Maximized_Horz,
	Net_Wm_State_FullScreen,
	Net_Wm_State_Above,
	Net_Wm_Window_Type,
	Net_Wm_Window_Type_Normal,
	Net_Wm_Window_Type_Dialog,
	Net_Wm_Window_Type_Toolbar,
	Net_Wm_Window_Type_Menu,
	Net_Wm_Window_Type_Utility,
	Net_Wm_Window_Type_Splash,
	Kde_Net_Wm_Window_Type_Override,
	Kde_Net_Wm_Frame_Strut,
	Net_Wm_State_Stays_On_Top,
	Net_Wm_Pid,
	Net_Wm_User_Time,
	Enlightenment_Desktop,
	Net_Wm_Name,
	Net_Wm_Icon_Name,
	Utf8_String,
	Text,
	Compound_Text,
	Targets,
	Multiple,
	Timestamp,
	Clip_Temporary,

	// Xdnd
	Xdnd_Enter,
	Xdnd_Position,
	Xdnd_Status,
	Xdnd_Leave,
	Xdnd_Drop,
	Xdnd_Finished,
	Xdnd_Typelist,

	Xdnd_Selection,

	Xdnd_Aware,
	Xdnd_Proxy,


	Xdnd_Action_Copy,
	Xdnd_Action_Link,
	Xdnd_Action_Move,
	Xdnd_Action_Private,

	// Motif Dnd
	Motif_Drag_And_Drop_Message,
	Motif_Drag_Initiator_Info,
	Motif_Drag_Receiver_Info,
	Motif_Drag_Window,
	Motif_Drag_Targets,

	Xm_Transfer_Success,
	Xm_Transfer_Failure,

	NPredefinedAtoms,

	Qt_Settings_Timestamp = NPredefinedAtoms,
	NAtoms
    };
    Atom atoms[NAtoms];
};

extern QX11Data *qt_x11Data;
#define ATOM(x) qt_x11Data->atoms[QX11Data::x]
#define X11 qt_x11Data

#endif // QT_X11_H
