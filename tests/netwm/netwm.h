/*

  Copyright (c) 2000 Troll Tech AS

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

*/


#ifndef   __net_wm_h
#define   __net_wm_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#include "netwm_def.h"
#include "netwm_p.h"


/**
   Common API for root window properties/protocols.

   The NETRootInfo class provides a commom API for clients and window managers
   to set/read/change properties on the root window as defined by the NET Window
   Manager Specification..

   @author Bradley T. Hughes <bhughes@trolltech.com>
   @see NET
 **/

class NETRootInfo : public NET {
public:
    /**
       Window Managers should use this constructor to create a NETRootInfo object,
       which will be used to set/update information stored on the rootWindow.
       The application role is automatically set to WindowManager
       when using this constructor.

       Taken arguments:

       @li display - an X11 Display struct.

       @li supportWindow - the Window id of the supportWindow.  The supportWindow
       must be created by the window manager as a child of the rootWindow.  The
       supportWindow must not be destroyed until the Window Manager exits.

       @li wmName - a string which should be the window manager's name (ie. "KWin"
       or "Blackbox").

       @li properties - an OR'ed list of all properties and protocols the window
       manager supports (see the NET base class documentation for a description
       of all properties and protocols).

       @li screen - for Window Managers that support multiple screen (ie.
       "multiheaded") displays, the screen number may be explicitly defined.  If
       this argument is omitted, the default screen will be used.
    **/
    NETRootInfo(Display *display, Window supportWindow, const char *wmName,
		unsigned long properties, int screen = -1);

    /**
       Clients should use this constructor to create a NETRootInfo object, which
       will be used to query information set on the root window. The application
       role is automatically set to Client when using this constructor.

       Taken arguments:

       @li display - an X11 Display struct.

       @li properties - an OR'ed list of all properties and protocols the client
       supports (see the NET base class documentation for a description of all
       properties and protocols).

       @li screen - for Clients that support multiple screen (ie. "multiheaded")
       displays, the screen number may be explicitly defined. If this argument is
       omitted, the default screen will be used.
    **/
    NETRootInfo(Display *display, unsigned long properties, int screen = -1);

    /**
       Creates a shared copy of the specified NETRootInfo object.
    **/
    NETRootInfo(const NETRootInfo &rootinfo);

    /**
       Destroys the NETRootInfo object.
    **/
    virtual ~NETRootInfo();

    /**
       Returns the X11 Display struct used.
    **/
    inline Display *x11Display() const;

    /**
       Returns the Window id of the rootWindow.
    **/
    inline Window rootWindow() const;

    /**
       Returns the Window id of the supportWindow.
    **/
    inline Window supportWindow() const;

    /**
       Returns the name of the Window Manager.
    **/
    inline const char *wmName() const;

    /**
       Returns the screenNumber.
    **/
    inline int screenNumber() const;

    /**
       Returns an OR'ed list of supported protocols and properties.
    **/
    inline unsigned long supported() const;

    /**
       Returns an array of Window id's, which contain all managed windows.

       @see clientListCount
    **/
    inline const Window *clientList() const;

    /**
       Returns the number of managed windows in clientList array.
    **/
    inline int clientListCount() const;

    /**
       Returns an array of Window id's, which contain all managed windows in
       stacking order.
    **/
    inline const Window *clientListStacking() const;

    /**
       Returns the number of managed windows in the clientListStacking array.
    **/
    inline int clientListStackingCount() const;

    /**
       Returns an array of Window id's, which contain all KDE docking windows.
    **/
    inline const Window *kdeDockingWindows() const;

    /**
       Returns the number of windows in the kdeDockingWindows array.
    **/
    inline int kdeDockingWindowsCount() const;

    /**
       Returns the size of the specified desktop.
    **/
    inline NETSize desktopGeometry(CARD32 desktop) const;

    /**
       Returns the viewport of the specified desktop.
    **/
    inline NETPoint desktopViewport(CARD32 desktop) const;

    /**
       Returns the workArea for the specified desktop.
    **/
    inline NETRect workArea(CARD32 desktop) const;

    /**
       Returns the name for the specified desktop.
    **/
    inline const char *desktopName(CARD32 desktop) const;

    /**
       Returns an array of Window id's, which contain the virtual root windows.
    **/
    inline const Window *virtualRoots( ) const;

    /**
       Returns the number of window in the virtualRoots array.
    **/
    inline int virtualRootsCount() const;

    /**
       Returns the number of desktops.
    **/
    inline CARD32 numberOfDesktops() const;

    /**
       Returns the current desktop.
    **/
    inline CARD32 currentDesktop() const;

    /**
       Returns the active (focused) window.
    **/
    inline Window activeWindow() const;

    /**
       Window Managers must call this after creating the NETRootInfo object, and
       before using any other method in the class.  This method sets initial data
       on the root window and does other post-construction duties.

       Clients must also call this after creating the object to do an initial
       data read/update.
    **/
    void activate();

    /**
       Sets the list of managed windows on the Root/Desktop window.

       Taken arguments:

       @li windows - the array of Window id's

       @li count - the number of windows in the array
    **/
    void setClientList(Window *windows, unsigned int count);

    /**
       Sets the list of managed windows in stacking order on the Root/Desktop
       window.

       Taken arguments:

       @li windows - the array of Window id's

       @li count - the number of windows in the array.
    **/
    void setClientListStacking(Window *windows, unsigned int count);

    /**
       Sets the list of KDE docking windows on the root window.

       Taken arguments:

       @li window - the array of window id's

       @li count - the number of windows in the array.
    **/
    void setKDEDockingWindows(Window *windows, unsigned int count);

    /**
       Sets the current desktop to the specified desktop.
    **/
    void setCurrentDesktop(CARD32 desktop);

    /**
       Sets the specified desktop geometry to the specified geometry.

    **/
    void setDesktopGeometry(CARD32 desktop, const NETSize &geometry);

    /**
       Sets the specified desktop viewport to the specified viweport.
    **/
    void setDesktopViewport(CARD32 desktop, const NETPoint &viewport);

    /**
       Sets the number of desktops the the specified number.
    **/
    void setNumberOfDesktops(CARD32 numberOfDesktops);

    /**
       Sets the name of the specified desktop.
    **/
    void setDesktopName(CARD32 desktop, const char *desktopName);

    /**
       Sets the active (focused) window the specified window.
    **/
    void setActiveWindow(Window window);

    /**
       Sets the workarea for the specified desktop
    **/
    void setWorkArea(CARD32 desktop, const NETRect &workArea);

    /**
       Sets the list of virtual root windows on the root window.

       Taken arguments:

       @li windows - the array of Window id's

       @li count - the number of windows in the array.
    **/
    void setVirtualRoots(Window *windows, unsigned int count);

    /**
       Assignment operator.  Ensures that the shared data reference counts are
       correct.
    **/
    const NETRootInfo &operator=(const NETRootInfo &rootinfo);

    /**
       Clients (such as pagers/taskbars) that wish to close a window should call
       this function.  This will send a request to the Window Manager, which
       usually can usually decide how to react to such requests.
    **/
    void closeWindowRequest(Window window);
    
    /**
       Clients (such as pagers/taskbars) that wish to start a WMMoveResize
       (where the window manager controls the resize/movement) should call
       this function.  This will send a request to the Window Manager.
       
       Taken arguments:
       
       @li window - the client window that whould be resized/moved.
       
       @li x_root - X position of the cursor relative to the root window.
       
       @li y_root - Y position of the cursor relative to the root window.
       
       @li direction - one of NET::Direction (see base class documentation for
       a description of the different directions).
    **/
    void moveResizeRequest(Window window, int x_root, int y_root,
			   Direction direction);
       
    /**
       This function takes the passed XEvent and returns an OR'ed list of
       NETRootInfo properties that have changed.  The new information will be
       read immediately by the class.
    **/
    unsigned long event(XEvent *event);


protected:
    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been added.
    **/
    // virtual void addClient(Window window) { }
    virtual void addClient(Window) { }

    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been removed.
    **/
    // virtual void removeClient(Window window) { }
    virtual void removeClient(Window) { }

    /**
       A Client should subclass NETRootInfo and reimeplement this function when
       it wants to know when a docking window has been added.  This is a KDE 2.0
       extension.
    **/
    // virtual void addDockWin(Window window) { }
    virtual void addDockWin(Window) { }

    /**
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a docking window has been removed.  This is a KDE 2.0
       extension.
    **/
    // virtual void removeDockWin(Window window) { }
    virtual void removeDockWin(Window) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the number
       of desktops.
    **/
    // virtual void changeNumberOfDesktops(CARD32 numberOfDesktops) { }
    virtual void changeNumberOfDesktops(CARD32) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the current
       desktop geometry.
    **/
    // virtual void changeDesktopGeometry(const NETSize &geom) { }
    virtual void changeDesktopGeometry(const NETSize &) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the current
       desktop viewport.
    **/
    // virtual void changeDesktopViewport(const NETPoint &viewport) { }
    virtual void changeDesktopViewport(const NETPoint &) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the current
       desktop.
    **/
    // virtual void changeCurrentDesktop(CARD32 desktop) { }
    virtual void changeCurrentDesktop(CARD32) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the active
       (focused) window.
    **/
    // virtual void changeActiveWindow(Window window) { }
    virtual void changeActiveWindow(Window) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to close a window.
    **/
    // virtual void closeWindow(Window window) { }
    virtual void closeWindow(Window) { }

    /**
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to start a move/resize.

       Taken arguments:

       @li window - the window that wants to move/resize

       @li x_root / y_root - position of the cursor relative to the root window.

       @li direction - one of NET::Direction (see base class documentation for
       a description of the different directions).
    **/
    // virtual void moveResize(Window window, int x_root, int y_root,
    // 			    unsigned long direction) { }
    virtual void moveResize(Window, int, int, unsigned long) { }


private:
    void update(unsigned long);
    void setSupported(unsigned long);

    NETRootInfoPrivate *p;
    Role role;
};


/**
   Common API for application window properties/protocols.

   The NETWinInfo class provides a common API for clients and window managers to
   set/read/change properties on an application window as defined by the NET
   Window Manager Specification.

   @author Bradley T. Hughes <bhughes@trolltech.com>
   @see http://www.freedesktop.org/standards/wm-spec/
 **/

class NETWinInfo : public NET {
public:
    /**
       Create a NETWinInfo object, which will be used to set/read/change
       information stored on an application window.

       Taken arguments:

       @li display - an X11 Display struct.

       @li window - the Window id of the application window.

       @li rootWindow - the Window id of the root window.

       @li properties - an OR'ed list of all properties and protocols the
       client/window manager supports (see the NET base class documentation
       for a description of all properties and protocols).

       @li role - select the application role.  If this argument is omitted,
       the role will default to Client.
    **/
    NETWinInfo(Display *display, Window window,
	       Window rootWindow, unsigned long properties,
	       Role role = Client);

    /**
       Creates a shared copy of the specified NETWinInfo object.
    **/
    NETWinInfo(const NETWinInfo & wininfo);

    /**
       Destroys the NETWinInfo object.
    **/
    virtual ~NETWinInfo();

    /**
       Returns an OR'ed list of supported protocols and properties.
    **/
    inline unsigned long properties() const;

    /**
       Returns the icon geometry.
    **/
    inline NETRect iconGeometry() const;

    /**
       Returns the state of the window (see the NET base class documentation for a
       description of the various states).
    **/
    inline unsigned long state() const;

    /**
       Returns the strut specified by this client.
    **/
    inline NETStrut strut() const;

    /**
       Returns the window type for this client (see the NET base class
       documentation for a description of the various window types).
    **/
    inline WindowType windowType() const;

    /**
       Returns the name of the window in UTF-8 format.
    **/
    inline const char *name() const;

    /**
       Returns the visible name as set by the window manager in UTF-8 format.
    **/
    inline const char *visibleName() const;

    /**
       Returns the desktop where the window is residing.
    **/
    inline CARD32 desktop() const;

    /**
       Returns the process id for the client window.
    **/
    inline CARD32 pid() const;

    /**
       Returns whether or not this client handles icons.
    **/
    inline Bool handledIcons() const;

    /**
       Returns a Window id, telling the window manager which window we are
       representing.
    **/
    inline Window kdeDockWinFor() const;

    /**
       Returns the mapping state for the window (see the NET base class
       documentation for a description of mapping state).
    **/
    inline MappingState mappingState() const;

    /**
       Set icons for the application window.  If replace is True, then
       the specified icon is defined to be the only icon.  If replace is False,
       then the specified icon is added to a list of icons.
    **/
    void setIcon(NETIcon icon, Bool replace = True);

    /**
       Set the icon geometry for the application window.
    **/
    void setIconGeometry(NETRect geometry);

    /**
       Set the strut for the application window.
    **/
    void setStrut(NETStrut strut);

    /**
       Set the state for the application window (see the NET base class documentation
       for a description of window state).
    **/
    void setState(unsigned long state, unsigned long mask);

    /**
       Sets the window type for this client (see the NET base class
       documentation for a description of the various window types).
    **/
    void setWindowType(WindowType type);

    /**
       Sets the name for the application window.
    **/
    void setName(const char *name);

    /**
       For Window Managers only:  set the visible name ( i.e. xterm, xterm <2>,
       xterm <3>, ... )
    **/
    void setVisibleName(const char *visibleName);

    /**
       Set which window the desktop is (should be) on.
    **/
    void setDesktop(CARD32 desktop);

    /**
       Set the application window's process id.
    **/
    void setPid(CARD32 pid);

    /**
       Set whether this application window handles icons.
    **/
    void setHandledIcons(Bool handled);

    /**
       Set which window we are representing as a dock window.
    **/
    void setKDEDockWinFor(Window window);

    /**
       Set the frame decoration strut.  This is a KDE 2.0 extension to aid in
       writing pager applications.
    **/
    void setKDEFrameStrut(NETStrut strut);

    /**
       Returns an icon.  If width and height are passed, the icon returned will be
       the closest it can find (the next biggest).  If width and height are omitted,
       then the first icon in the list is returned.
    **/
    NETIcon icon(int width = -1, int height = -1) const;

    /**
       Places the window frame geometry in frame, and the application window
       geometry in window.  Both geometries are relative to the root window.
    **/
    void kdeGeometry(NETRect &frame, NETRect &window);

    /**
       This function takes the pass XEvent and returns an OR'ed list of NETWinInfo
       properties that have changed.  The new information will be read
       immediately by the class.
    **/
    unsigned long event(XEvent *event);

    /**
       Sentinel value to indicate that the client wishes to be visible on
       all desktops.
    **/
    static const CARD32 OnAllDesktops = (CARD32) -1;
       
       
protected:
    /**
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change desktops (ie. move to
       another desktop).
    **/
    // virtual void changeDesktop(CARD32 desktop) { }
    virtual void changeDesktop(CARD32) { }

    /**
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change state (ie. to
       Shade / Unshade).
    **/
    // virtual void changeState(CARD32 state, CARD32 mask) { }
    virtual void changeState(CARD32, CARD32) { }


private:
    void update(unsigned long);

    NETWinInfoPrivate *p;
    Role role;
};


// NETRootInfo inlines

inline Display *NETRootInfo::x11Display() const { return p->display; }

inline Window NETRootInfo::rootWindow() const { return p->root; }

inline Window NETRootInfo::supportWindow() const { return p->supportwindow; }

inline const char *NETRootInfo::wmName() const { return p->name; }

inline int NETRootInfo::screenNumber() const { return p->screen; }

inline unsigned long NETRootInfo::supported() const  { return p->protocols; }

inline const Window *NETRootInfo::clientList() const { return p->clients; }

inline int NETRootInfo::clientListCount() const { return p->clients_count; }

inline const Window *NETRootInfo::clientListStacking() const {
    return p->stacking;
}

inline int NETRootInfo::clientListStackingCount() const {
    return p->stacking_count;
}

inline const Window *NETRootInfo::kdeDockingWindows() const {
    return p->kde_docking_windows;
}

inline int NETRootInfo::kdeDockingWindowsCount() const {
    return p->kde_docking_windows_count;
}

inline NETSize NETRootInfo::desktopGeometry(CARD32 desktop) const {
    if (desktop < 1 || desktop > p->number_of_desktops) {
	NETSize sz;
	sz.width = sz.height = 0;
	return sz;
    }
    
    return p->geometry[desktop - 1];
}

inline NETPoint NETRootInfo::desktopViewport(CARD32 desktop) const {
    if (desktop < 1 || desktop > p->number_of_desktops) {
	NETPoint pt;
	pt.x = pt.y = 0;
	return pt;
    }
    
    return p->viewport[desktop - 1];
}

inline NETRect NETRootInfo::workArea(CARD32 desktop) const {
    if (desktop < 1 || desktop > p->number_of_desktops) {
	NETRect rt;
	rt.pos.x = rt.pos.y = rt.size.width = rt.size.height = 0;
	return rt;
    }
    
    return p->workarea[desktop - 1];
}

inline const char *NETRootInfo::desktopName(CARD32 desktop) const {
    if (desktop < 1 || desktop > p->number_of_desktops) {
	return 0;
    }
    
    return p->desktop_names[desktop - 1];
}

inline const Window *NETRootInfo::virtualRoots( ) const {
    return p->virtual_roots;
}

inline int NETRootInfo::virtualRootsCount() const {
    return p->virtual_roots_count;
}

inline CARD32 NETRootInfo::numberOfDesktops() const {
    return p->number_of_desktops;
}

inline CARD32 NETRootInfo::currentDesktop() const { return p->current_desktop; }

inline Window NETRootInfo::activeWindow() const { return p->active; }


// NETWinInfo inlines

inline NETRect NETWinInfo::iconGeometry() const { return p->icon_geom; }

inline unsigned long NETWinInfo::state() const { return p->state; }

inline NETStrut NETWinInfo::strut() const { return p->strut; }

inline NET::WindowType NETWinInfo::windowType() const { return p->type; }

inline const char *NETWinInfo::name() const { return p->name; }

inline const char *NETWinInfo::visibleName() const { return p->visible_name; }

inline CARD32 NETWinInfo::desktop() const { return p->desktop; }

inline CARD32 NETWinInfo::pid() const { return p->pid; }

inline Bool NETWinInfo::handledIcons() const { return p->handled_icons; }

inline Window NETWinInfo::kdeDockWinFor() const { return p->kde_dockwin_for; }

inline unsigned long NETWinInfo::properties() const { return p->properties; }

inline NET::MappingState NETWinInfo::mappingState() const {
    return p->mapping_state;
}

#endif // __net_wm_h
