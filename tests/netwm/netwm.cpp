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

#include "netwm.h"

#include <string.h>
#include <stdio.h>

#include <X11/Xlibint.h>


// root window properties
static Atom net_supported            = 0;
static Atom net_client_list          = 0;
static Atom net_client_list_stacking = 0;
static Atom net_desktop_geometry     = 0;
static Atom net_desktop_viewport     = 0;
static Atom net_current_desktop      = 0;
static Atom net_desktop_names        = 0;
static Atom net_number_of_desktops   = 0;
static Atom net_active_window        = 0;
static Atom net_workarea             = 0;
static Atom net_supporting_wm_check  = 0;
static Atom net_virtual_roots        = 0;
static Atom net_kde_docking_windows  = 0;

// root window messages
static Atom net_close_window         = 0;
static Atom net_wm_moveresize        = 0;

// application window properties
static Atom net_properties           = 0;
static Atom net_wm_name              = 0;
static Atom net_wm_desktop           = 0;
static Atom net_wm_window_type       = 0;
static Atom net_wm_state             = 0;
static Atom net_wm_strut             = 0;
static Atom net_wm_icon_geometry     = 0;
static Atom net_wm_icon              = 0;
static Atom net_wm_pid               = 0;
static Atom net_wm_handled_icons     = 0;
static Atom net_wm_kde_docking_window_for = 0;

// application protocols
static Atom net_wm_ping              = 0;

// used to determine whether application window is managed or not
static Atom xa_wm_state              = 0;

static Bool atoms_created            = False;


static char *nstrdup(const char *s1) {
    if (! s1) return (char *) 0;

    int l = strlen(s1) + 1;
    char *s2 = new char[l];
    strncpy(s2, s1, l);
    return s2;
}


static char *nstrndup(const char *s1, int l) {
    if (! s1) return (char *) 0;

    char *s2 = new char[l];
    strncpy(s2, s1, l);
    return s2;
}


static Window *nwindup(Window *w1, int n) {
    if (! w1) return (Window *) 0;

    Window *w2 = new Window[n];
    while (n--) w2[n] = w1[n];
    return w2;
}


static void refdec_nri(NETRootInfoPrivate *p) {
#ifdef    DEBUG
    fprintf(stderr, "decrementing NETRootInfoPrivate::ref (%d)\n", p->ref - 1);
#endif

    if (! --p->ref) {
#ifdef    DEBUG
	fprintf(stderr, "  no more references, deleting\n");
#endif

	if (p->name) delete [] p->name;
	if (p->stacking) delete [] p->stacking;
	if (p->clients) delete [] p->clients;
	if (p->virtual_roots) delete [] p->virtual_roots;

	int i;
	for (i = 0; i < p->desktop_names.size(); i++)
	    if (p->desktop_names[i]) delete [] p->desktop_names[i];
    }
}


static void refdec_nwi(NETWinInfoPrivate *p) {
#ifdef    DEBUG
    fprintf(stderr, "decrementing NETWinInfoPrivate::ref (%d)\n", p->ref - 1);
#endif

    if (! --p->ref) {
	if (p->name) delete [] p->name;

	int i;
	for (i = 0; i < p->icons.size(); i++)
	    if (p->icons[i].data) delete [] p->icons[i].data;
    }
}


static void windowSort(Window *wins, int num) {
    int i, j, swaps;

    // bubble sort:
    for (i = num; i >= 1; i--) {
	swaps = 0;

	for (j = 2; j <= i; j++)
	    if (wins[j - 1] > wins[j]) {
		Window s = wins[j - 1];
		wins[j - 1] = wins[j];
		wins[j] = s;
		swaps++;
	    }

	if (swaps == 0) break;
    }
}


static void create_atoms(Display *d) {
    Atom atoms[28], *atomsp[28];
    const char *names[28];

    names[0]  = "_NET_SUPPORTED";
    atoms[0]   = 0;
    atomsp[0] = &net_supported;

    names[1]  = "_NET_SUPPORTING_WM_CHECK";
    atoms[1]   = 0;
    atomsp[1] = &net_supporting_wm_check;

    names[2]  = "_NET_CLIENT_LIST";
    atoms[2]  = 0;
    atomsp[2] = &net_client_list;

    names[3]  = "_NET_CLIENT_LIST_STACKING";
    atoms[3]  = 0;
    atomsp[3] = &net_client_list_stacking;

    names[4]  = "_NET_NUMBER_OF_DESKTOPS";
    atoms[4]  = 0;
    atomsp[4] = &net_number_of_desktops;

    names[5]  = "_NET_DESKTOP_GEOMETRY";
    atoms[5]  = 0;
    atomsp[5] = &net_desktop_geometry;

    names[6] = "_NET_DESKTOP_VIEWPORT";
    atoms[6] = 0;
    atomsp[6] = &net_desktop_viewport;

    names[7]  = "_NET_CURRENT_DESKTOP";
    atoms[7]  = 0;
    atomsp[7] = &net_current_desktop;

    names[8]  = "_NET_DESKTOP_NAMES";
    atoms[8]  = 0;
    atomsp[8] = &net_desktop_names;

    names[9]  = "_NET_ACTIVE_WINDOW";
    atoms[9]  = 0;
    atomsp[9] = &net_active_window;

    names[10]  = "_NET_WORKAREA";
    atoms[10]  = 0;
    atomsp[10] = &net_workarea;

    names[11]  = "_NET_VIRTUAL_ROOTS";
    atoms[11]  = 0;
    atomsp[11] = &net_virtual_roots;

    names[12]  = "_NET_CLOSE_WINDOW";
    atoms[12]  = 0;
    atomsp[12] = &net_close_window;

    names[13]  = "_NET_WM_MOVERESIZE";
    atoms[13]  = 0;
    atomsp[13] = &net_wm_moveresize;

    names[14]  = "_NET_PROPERTIES";
    atoms[14]  = 0;
    atomsp[14] = &net_properties;

    names[15]  = "_NET_WM_NAME";
    atoms[15]  = 0;
    atomsp[15] = &net_wm_name;

    names[16]  = "_NET_WM_DESKTOP";
    atoms[16]  = 0;
    atomsp[16] = &net_wm_desktop;

    names[17]  = "_NET_WM_WINDOW_TYPE";
    atoms[17]  = 0;
    atomsp[17] = &net_wm_window_type;

    names[18]  = "_NET_WM_STATE";
    atoms[18]  = 0;
    atomsp[18] = &net_wm_state;

    names[19]  = "_NET_WM_STRUT";
    atoms[19]  = 0;
    atomsp[19] = &net_wm_strut;

    names[20]  = "_NET_WM_ICON_GEOMETRY";
    atoms[20]  = 0;
    atomsp[20] = &net_wm_icon_geometry;

    names[21]  = "_NET_WM_ICON";
    atoms[21]  = 0;
    atomsp[21] = &net_wm_icon;

    names[22]  = "_NET_WM_PID";
    atoms[22]  = 0;
    atomsp[22] = &net_wm_pid;

    names[23]  = "_NET_WM_HANDLED_ICONS";
    atoms[23]  = 0;
    atomsp[23] = &net_wm_handled_icons;

    names[24]  = "_NET_WM_PING";
    atoms[24]  = 0;
    atomsp[24] = &net_wm_ping;

    names[25]  = "_NET_KDE_DOCKING_WINDOWS";
    atoms[25]  = 0;
    atomsp[25] = &net_kde_docking_windows;

    names[26]  = "_NET_WM_KDE_DOCKING_WINDOW_FOR";
    atoms[26]  = 0;
    atomsp[26] = &net_wm_kde_docking_window_for;

    names[27] = "WM_STATE";
    atoms[27] = 0;
    atomsp[27] = &xa_wm_state;

    XInternAtoms(d, (char **) names, 28, False, atoms);

    int i = 28;
    while (i--)
	*atomsp[i] = atoms[i];

    atoms_created = True;
}


static void readIcon(NETWinInfoPrivate *p) {
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret, after_ret;
    unsigned char *data_ret;

    int ret =
	XGetWindowProperty(p->display, p->window, net_wm_icon, 0l, 1l, False,
			   XA_CARDINAL, &type_ret, &format_ret, &nitems_ret,
			   &after_ret, &data_ret);

    if (data_ret) XFree(data_ret);

    if (ret != Success || nitems_ret < 3 || type_ret != XA_CARDINAL ||
	format_ret != 32)
	// either we didn't get the property, or the property has less than
	// 3 elements in it
	// NOTE: 3 is the ABSOLUTE minimum:
	//     width = 1, height = 1, length(data) = 1 (width * height)
	return;

    // allocate space after_ret (bytes remaining in property) + 4
    // (the single 32bit quantity we just read)
    unsigned long proplen = after_ret + 4;
    unsigned char *buffer = new unsigned char[proplen];
    unsigned long offset = 0, buffer_offset = 0;

    while (after_ret >0) {
	XGetWindowProperty(p->display, p->window, net_wm_icon, offset,
			   (long) BUFSIZE, False, XA_CARDINAL, &type_ret,
			   &format_ret, &nitems_ret, &after_ret, &data_ret);
	memcpy((buffer + buffer_offset), data_ret, nitems_ret * 4);
	buffer_offset += nitems_ret * 4;
 	offset += nitems_ret;
	XFree(data_ret);
    }

    unsigned long i, j;
    CARD32 *d = (CARD32 *) buffer;
    for (i = 0, j = 0; i < proplen - 3; i++) {
	p->icons[j].size.width = *d++;
	p->icons[j].size.height = *d++;

	unsigned long s = (p->icons[j].size.width *
			   p->icons[j].size.height * 4);
	if (p->icons[j].data) delete [] p->icons[j].data;
	memcpy(p->icons[j].data, d, s);
	d += s;
	j++;
    }
}


template <class Z>
RArray<Z>::RArray() {
  sz = 0;
  d = 0;
}


template <class Z>
RArray<Z>::~RArray() {
  if (d) delete [] d;
}


template <class Z>
Z &RArray<Z>::operator[](int index) {
    if (! d) d = new Z[index + 1];
    else if (index >= sz) {
	// allocate space for the new data
	Z *newdata = new Z[index + 1];

	// move the old data into the new array
	int i;
	for (i = 0; i < sz; i++)
	    newdata[i] = d[i];
	memset((newdata + i), 0, sizeof(Z));
	
	sz = index + 1;

	// delete old data and reassign
	delete d;
	d = newdata;
    }

    return d[index];
}


// Construct a new NETRootInfo object.

NETRootInfo::NETRootInfo(Display *dp, Window sw, const char *nm,
			 unsigned long pr, int sc)
{
#ifdef    DEBUG
    fprintf(stderr, "NETRootInfo::NETRootInfo: using window manager constructor\n");
#endif

    p = new NETRootInfoPrivate;
    p->ref = 1;

    p->display = dp;
    p->name = nstrdup(nm);

    if (sc != -1)
	p->screen = sc;
    else
	p->screen = DefaultScreen(p->display);

    p->root = RootWindow(p->display, p->screen);
    p->supportwindow = sw;
    p->protocols = pr;
    p->number_of_desktops = p->current_desktop = 0;
    p->active = None;
    p->clients = p->stacking = p->virtual_roots = (Window *) 0;
    p->clients_count = p->stacking_count = p->virtual_roots_count = 0;

    role = WindowManager;
    
    if (! atoms_created) create_atoms(p->display);
}


NETRootInfo::NETRootInfo(Display *d, unsigned long pr, int s) {
    p = new NETRootInfoPrivate;
    p->ref = 1;

    p->display = d;

    if (s != -1)
	p->screen = s;
    else
	p->screen = DefaultScreen(p->display);

    p->root = RootWindow(p->display, p->screen);
    p->supportwindow = None;
    p->protocols = pr;
    p->number_of_desktops = p->current_desktop = 0;
    p->active = None;
    p->clients = p->stacking = p->virtual_roots = (Window *) 0;
    p->clients_count = p->stacking_count = p->virtual_roots_count = 0;

    role = Client;
    
    if (! atoms_created) create_atoms(p->display);
}


// Copy an existing NETRootInfo object.

NETRootInfo::NETRootInfo(const NETRootInfo &nri) {
    p = nri.p;
    p->ref++;
}


// Be gone with our NETRootInfo.

NETRootInfo::~NETRootInfo() {
    refdec_nri(p);

    if (! p->ref) delete p;
}


void NETRootInfo::activate() {
    if (! atoms_created) create_atoms(p->display);

    if (role == WindowManager)
	// force support for Supported and SupportingWMCheck for window managers
	setSupported(p->protocols | Supported | SupportingWMCheck);
    else
	update(p->protocols);
}


void NETRootInfo::setClientList(Window *wins, unsigned int num) {
    if (role != WindowManager) return;

    p->clients_count = num;

    if (p->clients) delete [] p->clients;
    p->clients = nwindup(wins, num);

    XChangeProperty(p->display, p->root, net_client_list, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) p->clients,
		    p->clients_count);
}


void NETRootInfo::setClientListStacking(Window *wins, unsigned int num) {
    if (role != WindowManager) return;

    p->stacking_count = num;
    if (p->stacking) delete [] p->stacking;
    p->stacking = nwindup(wins, num);

    XChangeProperty(p->display, p->root, net_client_list_stacking, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) p->stacking,
		    p->stacking_count);
}


void NETRootInfo::setKDEDockingWindows(Window *wins, unsigned int num) {
    if (role != WindowManager) return;

    p->kde_docking_windows_count = num;
    if (p->kde_docking_windows) delete [] p->kde_docking_windows;
    p->kde_docking_windows = nwindup(wins, num);

    XChangeProperty(p->display, p->root, net_kde_docking_windows, XA_WINDOW, 32,
		    PropModeReplace,
		    (unsigned char *) p->kde_docking_windows,
		    p->kde_docking_windows_count);
}


void NETRootInfo::setNumberOfDesktops(CARD32 num) {
    if (role == WindowManager) {
	p->number_of_desktops = num;

	XChangeProperty(p->display, p->root, net_number_of_desktops, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &p->number_of_desktops, 1);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_number_of_desktops;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = num;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, NoEventMask, &e);
    }
}


void NETRootInfo::setCurrentDesktop(CARD32 desk) {
  if (role == WindowManager) {
    p->current_desktop = desk;
    XChangeProperty(p->display, p->root, net_current_desktop, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &(p->current_desktop), 1);
  } else {
    XEvent e;

    e.xclient.type = ClientMessage;
    e.xclient.message_type = net_current_desktop;
    e.xclient.display = p->display;
    e.xclient.window = p->root;
    e.xclient.format = 32;
    e.xclient.data.l[0] = desk;
    e.xclient.data.l[1] = 0l;
    e.xclient.data.l[2] = 0l;
    e.xclient.data.l[3] = 0l;
    e.xclient.data.l[4] = 0l;

    XSendEvent(p->display, p->root, False, NoEventMask, &e);
  }
}


void NETRootInfo::setDesktopName(CARD32 desk, const char *name) {
    // return immediately if the requested desk is out of range
    if (desk >= p->number_of_desktops) return;

    if (p->desktop_names[desk]) delete p->desktop_names[desk];
    p->desktop_names[desk] = nstrdup(name);

    unsigned int i, proplen,
	num = ((p->number_of_desktops < (unsigned) p->desktop_names.size()) ?
	       p->number_of_desktops : (unsigned) p->desktop_names.size());
    for (i = 0, proplen = 0; i < num; i++)
	proplen += strlen(p->desktop_names[i]) + 1;

    char *prop = new char[proplen], *propp = prop;

    for (i = 0; i < num; i++)
	if (p->desktop_names[i]) {
	    strcpy(propp, p->desktop_names[i]);
	    propp += strlen(p->desktop_names[i]) + 1;
	} else	   
	    *propp++ = '\0';

#ifdef    DEBUG
    fprintf(stderr,
	    "NETRootInfo::setDesktopName(%ld, '%s')\n"
	    "desktop_names (atom %ld:\n",
	    desk, name, net_desktop_names);

    for (i = 0; i < num; i++)
	fprintf(stderr, "\t'%s'\n", p->desktop_names[i]);
    
    for (i = 0; i < proplen; i++)
	if (prop[i] == '\0') fprintf(stderr, ".");
	else fprintf(stderr, "%c", prop[i]);
    fprintf(stderr, "\n");
#endif
    
    XChangeProperty(p->display, p->root, net_desktop_names, XA_STRING, 8,
		    PropModeReplace, (unsigned char *) prop, proplen);
    
    delete [] prop;
}


void NETRootInfo::setDesktopGeometry(const NETSize &sz) {
    if (role == WindowManager) {
	CARD32 data[2];
	data[0] = sz.width;
	data[1] = sz.height;

	p->geometry = sz;

	XChangeProperty(p->display, p->root, net_desktop_geometry, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, 2);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_desktop_geometry;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = sz.width;
	e.xclient.data.l[1] = sz.height;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, NoEventMask, &e);
    }
}


void NETRootInfo::setDesktopViewport(const NETPoint &pt) {
    if (role == WindowManager) {
	CARD32 data[2];
	data[0] = pt.x;
	data[1] = pt.y;

	p->viewport = pt;

	XChangeProperty(p->display, p->root, net_desktop_viewport, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) data, 2);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_desktop_viewport;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = pt.x;
	e.xclient.data.l[1] = pt.y;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, NoEventMask, &e);
    }
}


void NETRootInfo::setSupported(unsigned long pr) {
    p->protocols = pr;

    if (role != WindowManager) {
#ifdef    DEBUG
	fprintf(stderr, "NETRootInfo::setSupported - role != WindowManager\n");
#endif

	return;
    }

    Atom atoms[27];
    int pnum = 2;

    atoms[0] = net_supported;
    atoms[1] = net_supporting_wm_check;

    if (p->protocols & ClientList)
	atoms[pnum++] = net_client_list;

    if (p->protocols & ClientListStacking)
	atoms[pnum++] = net_client_list_stacking;

    if (p->protocols & NumberOfDesktops)
	atoms[pnum++] = net_number_of_desktops;

    if (p->protocols & DesktopGeometry)
	atoms[pnum++] = net_desktop_geometry;

    if (p->protocols & DesktopViewport)
	atoms[pnum++] = net_desktop_viewport;

    if (p->protocols & CurrentDesktop)
	atoms[pnum++] = net_current_desktop;

    if (p->protocols & DesktopNames)
	atoms[pnum++] = net_desktop_names;

    if (p->protocols & ActiveWindow)
	atoms[pnum++] = net_active_window;

    if (p->protocols & WorkArea)
	atoms[pnum++] = net_workarea;

    if (p->protocols & VirtualRoots)
	atoms[pnum++] = net_virtual_roots;

    if (p->protocols & KDEDockingWindows)
	atoms[pnum++] = net_kde_docking_windows;

    if (p->protocols & CloseWindow)
	atoms[pnum++] = net_close_window;

    if (p->protocols & WMMoveResize)
	atoms[pnum++] = net_wm_moveresize;

    if (p->protocols & Properties)
	atoms[pnum++] = net_properties;

    if (p->protocols & WMName)
	atoms[pnum++] = net_wm_name;

    if (p->protocols & WMDesktop)
	atoms[pnum++] = net_wm_desktop;

    if (p->protocols & WMWindowType)
	atoms[pnum++] = net_wm_window_type;

    if (p->protocols & WMState)
	atoms[pnum++] = net_wm_state;

    if (p->protocols & WMStrut)
	atoms[pnum++] = net_wm_strut;

    if (p->protocols & WMIconGeometry)
	atoms[pnum++] = net_wm_icon_geometry;

    if (p->protocols & WMIcon)
	atoms[pnum++] = net_wm_icon;

    if (p->protocols & WMPid)
	atoms[pnum++] = net_wm_pid;

    if (p->protocols & WMHandledIcons)
	atoms[pnum++] = net_wm_handled_icons;

    if (p->protocols & WMPing)
	atoms[pnum++] = net_wm_ping;

    if (p->protocols & WMKDEDockWinFor)
	atoms[pnum++] = net_wm_kde_docking_window_for;

    XChangeProperty(p->display, p->root, net_supported, XA_ATOM, 32,
		    PropModeReplace, (unsigned char *) atoms, pnum);
    XChangeProperty(p->display, p->root, net_supporting_wm_check, XA_WINDOW, 32,
	 	    PropModeReplace, (unsigned char *) &(p->supportwindow), 1);

#ifdef    DEBUG
    fprintf(stderr,
	    "NETRootInfo::setSupported: _NET_SUPPORTING_WM_CHECK = 0x%lx on 0x%lx\n"
	    "                         : _NET_WM_NAME = '%s' on 0x%lx\n",
	    p->supportwindow, p->supportwindow, p->name, p->supportwindow);
#endif

    XChangeProperty(p->display, p->supportwindow, net_supporting_wm_check,
		    XA_WINDOW, 32, PropModeReplace,
		    (unsigned char *) &(p->supportwindow), 1);
    XChangeProperty(p->display, p->supportwindow, net_wm_name, XA_STRING, 8,
		    PropModeReplace, (unsigned char *) p->name,
		    strlen(p->name) + 1);
}


void NETRootInfo::setActiveWindow(Window win) {
    if (role == WindowManager) {
	p->active = win;
	XChangeProperty(p->display, p->root, net_active_window, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *) &(p->active), 1);
    } else {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_active_window;
	e.xclient.display = p->display;
	e.xclient.window = p->root;
	e.xclient.format = 32;
	e.xclient.data.l[0] = win;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, NoEventMask, &e);
    }
}


void NETRootInfo::setWorkArea(CARD32 desk, const NETRect &rect) {
    if (role != WindowManager) return;

    p->workarea[desk] = rect;

    CARD32 *wa = new CARD32[p->number_of_desktops * 4];
    unsigned int i, o;
    for (i = 0, o = 0; i < p->number_of_desktops; i++) {
	wa[o++] = p->workarea[i].pos.x;
	wa[o++] = p->workarea[i].pos.y;
	wa[o++] = p->workarea[i].size.width;
	wa[o++] = p->workarea[i].size.height;
    }

    XChangeProperty(p->display, p->root, net_workarea, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) wa,
		    p->number_of_desktops * 4);
}


void NETRootInfo::setVirtualRoots(Window *wins, int num) {
    if (role != WindowManager) return;

    p->virtual_roots_count = num;
    p->virtual_roots = wins;

    XChangeProperty(p->display, p->root, net_virtual_roots, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) p->virtual_roots,
		    p->virtual_roots_count);
}


// assignment operator

const NETRootInfo &NETRootInfo::operator=(const NETRootInfo &nri) {
    if (p != nri.p) {
	refdec_nri(p);

	if (! p->ref) delete p;
    }

    p = nri.p;
    p->ref++;

    return *this;
}


unsigned long NETRootInfo::event(XEvent *e) {
    unsigned long dirty = 0;

    if (role == WindowManager) {
	if (e->type == ClientMessage && e->xclient.format == 32) {
	    if (e->xclient.message_type == net_number_of_desktops) {
		dirty = NumberOfDesktops;

		changeNumberOfDesktops(e->xclient.data.l[0]);
	    } else if (e->xclient.message_type == net_desktop_geometry) {
		dirty = DesktopGeometry;

		NETSize sz;
		sz.width = e->xclient.data.l[0];
		sz.height = e->xclient.data.l[1];
		changeDesktopGeometry(sz);
	    } else if (e->xclient.message_type == net_desktop_viewport) {
		dirty = DesktopViewport;

		NETPoint pt;
		pt.x = e->xclient.data.l[0];
		pt.y = e->xclient.data.l[1];
		changeDesktopViewport(pt);
	    } else if (e->xclient.message_type == net_current_desktop) {
		dirty = CurrentDesktop;

		changeCurrentDesktop(e->xclient.data.l[0]);
	    } else if (e->xclient.message_type == net_active_window) {
		dirty = ActiveWindow;

		changeActiveWindow(e->xclient.window);
	    } else if (e->xclient.message_type == net_wm_moveresize) {
		moveResize(e->xclient.window,
			   e->xclient.data.l[0],
			   e->xclient.data.l[1],
			   e->xclient.data.l[2]);
	    } else if (e->xclient.message_type == net_close_window) {
		closeWindow(e->xclient.window);
	    }
	} else if (e->type == PropertyNotify &&
		   e->xproperty.atom == net_desktop_names) {
	    dirty = DesktopNames;
	    update(dirty);
	}
    } else if (e->type == PropertyNotify) {
	XEvent pe = *e;

	Bool done = False;
	while (! done) {
	    if (pe.xproperty.atom == net_client_list)
		dirty |= ClientList;
	    else if (pe.xproperty.atom == net_client_list_stacking)
		dirty |= ClientListStacking;
	    else if (pe.xproperty.atom == net_kde_docking_windows)
		dirty |= KDEDockingWindows;
	    else if (pe.xproperty.atom == net_desktop_names)
		dirty |= DesktopNames;
	    else if (pe.xproperty.atom == net_workarea)
		dirty |= WorkArea;
	    else if (pe.xproperty.atom == net_number_of_desktops)
		dirty |= NumberOfDesktops;
	    else if (pe.xproperty.atom == net_desktop_geometry)
		dirty |= DesktopGeometry;
	    else if (pe.xproperty.atom == net_desktop_viewport)
		dirty |= DesktopViewport;
	    else if (pe.xproperty.atom == net_current_desktop)
		dirty |= CurrentDesktop;
	    else if (pe.xproperty.atom == net_active_window)
		dirty |= ActiveWindow;
	    else {
		XPutBackEvent(p->display, &pe);
		done = True;
	    }

	    if (! XCheckTypedWindowEvent(p->display, p->root, PropertyNotify, &pe))
		done = True;
	}

	update(dirty);
    }

    return dirty;
}


// private functions to update the data we keep

void NETRootInfo::update(unsigned long dirty) {
    Atom type_ret;
    int format_ret;
    unsigned char *data_ret;
    unsigned long nitems_ret, unused;

    if (dirty & ClientList)
	if (XGetWindowProperty(p->display, p->root, net_client_list,
			       0l, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_WINDOW && format_ret == 32) {
		    Window *wins = (Window *) data_ret;

		    windowSort(wins, nitems_ret);

		    if (p->clients) {
			if (role == Client) {
			    unsigned long n, o;

			    for (n = o = 0;
				 n < nitems_ret && o < p->clients_count; ) {
				if (o == p->clients_count)
				    addClient(wins[n++]);
				else if (n == nitems_ret)
				    removeClient(p->clients[o++]);
				else {
				    if (p->clients[o] < wins[n])
					removeClient(p->clients[o++]);
				    else if (p->clients[o] > wins[n])
					addClient(wins[n++]);
				    else {
					n++;
					o++;
				    }
				}
			    }
			}

			delete [] p->clients;
		    }

		    p->clients_count = nitems_ret;
		    p->clients = nwindup(wins, p->clients_count);
		}

		XFree(data_ret);
	    }

    if (dirty & KDEDockingWindows)
	if (XGetWindowProperty(p->display, p->root, net_kde_docking_windows,
			       0l, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_WINDOW && format_ret == 32) {
		    Window *wins = (Window *) data_ret;

		    windowSort(wins, nitems_ret);

		    if (p->kde_docking_windows) {
			if (role == Client) {
			    unsigned long n, o;

			    for (n = o = 0;
				 n < nitems_ret &&
				     o < p->kde_docking_windows_count; ) {
				if (o == p->kde_docking_windows_count)
				    addDockWin(wins[n++]);
				else if (n == nitems_ret)
				    removeDockWin(p->kde_docking_windows[o++]);
				else {
				    if (p->kde_docking_windows[o] < wins[n])
					removeDockWin(p->kde_docking_windows[o++]);
				    else if (p->kde_docking_windows[o] > wins[n])
					addDockWin(wins[n++]);
				    else {
					n++;
					o++;
				    }
				}
			    }
			}

			delete [] p->kde_docking_windows;
		    }

		    p->kde_docking_windows_count = nitems_ret;
		    p->kde_docking_windows =
			nwindup(wins, p->kde_docking_windows_count);
		}

		XFree(data_ret);
	    }

    if (dirty & ClientListStacking)
	if (XGetWindowProperty(p->display, p->root, net_client_list_stacking,
			       0, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_WINDOW && format_ret == 32) {
		    Window *wins = (Window *) data_ret;

		    if (p->stacking)
			delete [] p->stacking;

		    p->stacking_count = nitems_ret;
		    p->stacking = nwindup(wins, p->stacking_count);
		}

		XFree(data_ret);
	    }

    if (dirty & NumberOfDesktops)
	if (XGetWindowProperty(p->display, p->root, net_number_of_desktops,
			       0l, 1l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1)
		    p->number_of_desktops = *((CARD32 *) data_ret);

		XFree(data_ret);
	    }

    if (dirty & DesktopGeometry)
	if (XGetWindowProperty(p->display, p->root, net_desktop_geometry,
			       0l, 2l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 &&
		    nitems_ret == 2) {
		    CARD32 *d = (CARD32 *) data_ret;
		    p->geometry.width  = d[0];
		    p->geometry.height = d[1];
		}

		XFree(data_ret);
	    }

    if (dirty & DesktopViewport)
	if (XGetWindowProperty(p->display, p->root, net_desktop_viewport,
			       0l, 2l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 &&
		    nitems_ret == 2) {
		    CARD32 *d = (CARD32 *) data_ret;
		    p->viewport.x = d[0];
		    p->viewport.y = d[1];
		}

		XFree(data_ret);
	    }

    if (dirty & CurrentDesktop)
	if (XGetWindowProperty(p->display, p->root, net_current_desktop,
			       0l, 1l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 && nitems_ret == 1)
		    p->current_desktop = *((CARD32 *) data_ret);

		XFree(data_ret);
	    }

    if (dirty & DesktopNames)
	if (XGetWindowProperty(p->display, p->root, net_current_desktop,
			       0l, (long) BUFSIZE, False, XA_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_STRING && format_ret == 8) {
		    // force the last element in the data array to be NUL
		    data_ret[nitems_ret - 1] = '\0';

		    const char *d = (const char *) data_ret;
		    unsigned int s, n, index;

		    for (s = 0, n = 0, index = 0; n < nitems_ret; n++)
			if (d[n] == '\0') {
			    p->desktop_names[index++] = nstrndup((d + s), n - s);
			    s = n + 1;
			}

		}

		XFree(data_ret);
	    }

    if (dirty & ActiveWindow)
	if (XGetWindowProperty(p->display, p->root, net_active_window, 0l, 1l,
			       False, XA_WINDOW, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_WINDOW && format_ret == 32 && nitems_ret == 1)
		    p->active = *((Window *) data_ret);

		XFree(data_ret);
	    }

    if (dirty & WorkArea)
	if (XGetWindowProperty(p->display, p->root, net_active_window, 0l,
			       (p->number_of_desktops * 4), False, XA_CARDINAL,
			       &type_ret, &format_ret, &nitems_ret, &unused,
			       &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 &&
		    nitems_ret == (p->number_of_desktops * 4)) {
		    CARD32 *d = (CARD32 *) data_ret;
		    CARD32 i, j;
		    for (i = 0, j = 0; i < p->number_of_desktops; i++) {
			p->workarea[i].pos.x       = d[j++];
			p->workarea[i].pos.y       = d[j++];
			p->workarea[i].size.width  = d[j++];
			p->workarea[i].size.height = d[j++];
		    }
		}

		XFree(data_ret);
	    }

    if (dirty & SupportingWMCheck)
	if (XGetWindowProperty(p->display, p->root, net_supporting_wm_check,
			       0l, 1l, False, XA_WINDOW, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_WINDOW && format_ret == 32 && nitems_ret == 1) {
		    p->supportwindow = *((Window *) data_ret);

		    unsigned char *name_ret;
		    if (XGetWindowProperty(p->display, p->supportwindow,
					   net_wm_name, 0l, (long) BUFSIZE, False,
					   XA_STRING, &type_ret, &format_ret,
					   &nitems_ret, &unused, &name_ret))
			if (name_ret) {
			    if (type_ret == XA_STRING && format_ret == 8)
				p->name = nstrdup((const char *) name_ret);

			    XFree(name_ret);
			}
		}

		XFree(data_ret);
	    }

    if (dirty & VirtualRoots)
	if (XGetWindowProperty(p->display, p->root, net_virtual_roots,
			       0, (long) BUFSIZE, False, XA_WINDOW, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_WINDOW && format_ret == 32) {
		    Window *wins = (Window *) data_ret;

		    if (p->virtual_roots)
			delete [] p->virtual_roots;

		    p->virtual_roots_count = nitems_ret;
		    p->virtual_roots = nwindup(wins, p->virtual_roots_count);
		}

		XFree(data_ret);
	    }
}


// NETWinInfo stuffs

NETWinInfo::NETWinInfo(Display *d, Window win, Window rwin,
		       unsigned long pr, Role rl)
{
    p = new NETWinInfoPrivate;
    p->ref = 1;

    p->display = d;
    p->window = win;
    p->root = rwin;
    p->state = 0;
    p->type = Normal;
    p->name = (char *) 0;
    p->desktop = p->pid = p->handled_icons = 0;
    p->managed = False;
    p->properties = pr;
    p->icon_count = 0;

    role = rl;

    update(p->properties);

    if (! atoms_created) create_atoms(p->display);
}


NETWinInfo::NETWinInfo(const NETWinInfo &nwi) {
    p = nwi.p;
    p->ref++;
}


NETWinInfo::~NETWinInfo() {
    refdec_nwi(p);

    if (! p->ref) delete p;
}


void NETWinInfo::setIcon(NETIcon icon, Bool replace) {
    if (role != Client) return;

    if (replace) {
	int i;
	for (i = 0; i < p->icons.size(); i++) {
	    if (p->icons[i].data) delete [] p->icons[i].data;
	    p->icons[i].data = (CARD32 *) 0;
	    p->icons[i].size.width = 0;
	    p->icons[i].size.height = 0;
	}

	p->icon_count = 0;
    }

    p->icons[p->icon_count++] = icon;

    int proplen, i;
    for (i = 0, proplen = 0; i < p->icon_count; i++)
	proplen += 2 + (p->icons[i].size.width *
			p->icons[i].size.height);

    CARD32 *prop = new CARD32[proplen], *pprop = prop;
    int sz;
    for (i = 0; i < p->icon_count; i++) {
       	*pprop++ = p->icons[i].size.width;
	*pprop++ = p->icons[i].size.height;
	sz = (p->icons[i].size.width *
	      p->icons[i].size.height * 4);
	(void) memcpy(pprop, p->icons[i].data, sz);
	pprop += sz;
    }
}


void NETWinInfo::setIconGeometry(NETRect rect) {
    if (role != Client) return;

    p->icon_geom = rect;

    CARD32 data[4];
    data[0] = rect.pos.x;
    data[1] = rect.pos.y;
    data[2] = rect.size.width;
    data[3] = rect.size.height;

    XChangeProperty(p->display, p->window, net_wm_icon_geometry, XA_CARDINAL,
		    32, PropModeReplace, (unsigned char *) data, 4);
}


void NETWinInfo::setStrut(NETStrut strut) {
    if (role != Client) return;

    p->strut = strut;

    CARD32 data[4];
    data[0] = strut.left;
    data[1] = strut.right;
    data[2] = strut.top;
    data[3] = strut.bottom;

    XChangeProperty(p->display, p->window, net_wm_strut, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) data, 4);
}


void NETWinInfo::setState(unsigned long st, unsigned long msk) {
    if (role == Client && p->managed) {
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_wm_state;
	e.xclient.display = p->display;
	e.xclient.window = p->window;
	e.xclient.format = 32;
	e.xclient.data.l[0] = st;
	e.xclient.data.l[1] = msk;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, NoEventMask, &e);
    } else {
	p->state = st;
	XChangeProperty(p->display, p->window, net_wm_state, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &st, 1);
    }
}


void NETWinInfo::setWindowType(WindowType type) {
    if (role != Client) return;

    CARD32 data= type;
    XChangeProperty(p->display, p->window, net_wm_window_type, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &data, 1);
}


void NETWinInfo::setName(const char *name) {
    if (role != Client) return;

    if (p->name) delete [] p->name;
    p->name = nstrdup(name);
    XChangeProperty(p->display, p->window, net_wm_name, XA_STRING, 8,
		    PropModeReplace, (unsigned char *) p->name,
		    strlen(p->name) + 1);
}


void NETWinInfo::setDesktop(CARD32 desk) {
    if (role == Client && p->managed) {
	// we only send a ClientMessage if we are 1) a client and 2) managed
	XEvent e;

	e.xclient.type = ClientMessage;
	e.xclient.message_type = net_wm_desktop;
	e.xclient.display = p->display;
	e.xclient.window = p->window;
	e.xclient.format = 32;
	e.xclient.data.l[0] = desk;
	e.xclient.data.l[1] = 0l;
	e.xclient.data.l[2] = 0l;
	e.xclient.data.l[3] = 0l;
	e.xclient.data.l[4] = 0l;

	XSendEvent(p->display, p->root, False, NoEventMask, &e);
    } else {
	// otherwise we just set the property directly
	p->desktop = desk;
	XChangeProperty(p->display, p->window, net_wm_desktop, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &(p->desktop), 1);
    }
}


void NETWinInfo::setPid(CARD32 pd) {
    if (role != Client) return;

    p->pid = pd;
    XChangeProperty(p->display, p->window, net_wm_pid, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &(p->pid), 1);
}


void NETWinInfo::setHandledIcons(Bool handled) {
    if (role != Client) return;

    p->handled_icons = handled;
    CARD32 d = handled;
    XChangeProperty(p->display, p->window, net_wm_handled_icons, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &d, 1);
}


void NETWinInfo::setKDEDockWinFor(Window win) {
    if (role != Client) return;

    p->kde_dockwin_for = win;
    XChangeProperty(p->display, p->window, net_wm_kde_docking_window_for,
		    XA_CARDINAL, 32, PropModeReplace,
		    (unsigned char *) &(p->kde_dockwin_for), 1);
}


NETIcon NETWinInfo::icon(int w, int h) const {
    // find the icon that's closest in size to w x h...
    // return the first icon if w and h are -1
    if (w == h && h == -1) return p->icons[0];

    int i;
    for (i = 0; i < p->icons.size(); i++)
	if (p->icons[i].size.width >= (unsigned) w &&
	    p->icons[i].size.height >= (unsigned) h)
	    return p->icons[i];

    return p->icons[p->icons.size() - 1];
}


unsigned long NETWinInfo::event(XEvent *e) {
    unsigned long dirty = 0;

    if (role == WindowManager) {
	if (e->type == ClientMessage && e->xclient.format == 32) {
	    if (e->xclient.message_type == net_wm_state) {
		dirty = WMState;

		changeState(e->xclient.data.l[0]);
	    } else if (e->xclient.message_type == net_wm_desktop) {
		dirty = WMDesktop;

		changeDesktop(e->xclient.data.l[0]);
	    }
	} else if (e->type == PropertyNotify) {
	    XEvent pe = *e;

	    Bool done = False;
	    while (! done) {
		if (pe.xproperty.atom == net_wm_name)
		    dirty |= WMName;
		else if (pe.xproperty.atom == net_wm_window_type)
		    dirty |=WMWindowType;
		else if (pe.xproperty.atom == net_wm_strut)
		    dirty |= WMStrut;
		else if (pe.xproperty.atom == net_wm_icon_geometry)
		    dirty |= WMIconGeometry;
		else if (pe.xproperty.atom == net_wm_icon)
		    dirty |= WMIcon;
		else {
		    XPutBackEvent(p->display, &pe);
		    done = True;
		}

		if (! XCheckTypedWindowEvent(p->display, p->window,
					     PropertyNotify, &pe))
		    done = True;
	    }

	    update(dirty);
	}
    } else if (e->type == PropertyNotify) {
	XEvent pe = *e;

	Bool done = False;
	while (! done) {
	    if (pe.xproperty.atom == xa_wm_state)
		dirty |= XAWMState;
	    else if (pe.xproperty.atom == net_wm_state)
		dirty |= WMState;
	    else if (pe.xproperty.atom == net_wm_desktop)
		dirty |= WMDesktop;
	    else {
		XPutBackEvent(p->display, &pe);
		done = True;
	    }

	    if (! XCheckTypedWindowEvent(p->display, p->window,
					 PropertyNotify, &pe))
		done = True;
	}

	update(dirty);
    }

    return dirty;
}


void NETWinInfo::update(unsigned long dirty) {
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret, unused;
    unsigned char *data_ret;

    if (dirty & XAWMState)
	if (XGetWindowProperty(p->display, p->window, xa_wm_state, 0l, 1l,
			       False, xa_wm_state, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == xa_wm_state && format_ret == 32 &&
		    nitems_ret == 2) {
		    CARD32 *state = (CARD32 *) data_ret;
		    if (*state != IconicState) p->managed = True;
		}

		XFree(data_ret);
	    }

    if (dirty & WMState)
	if (XGetWindowProperty(p->display, p->window, net_wm_state, 0l, 1l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == net_wm_state && format_ret == 32 &&
		    nitems_ret == 1)
		    p->state = *((CARD32 *) data_ret);

		XFree(data_ret);
	    }

    if (dirty & WMDesktop)
	if (XGetWindowProperty(p->display, p->window, net_wm_desktop, 0l, 1l,
			       False, XA_CARDINAL, &type_ret,
			       &format_ret, &nitems_ret,
			       &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == net_wm_desktop && format_ret == 32 &&
		    nitems_ret == 1)
		    p->desktop = *((CARD32 *) data_ret);

		XFree(data_ret);
	    }

    if (dirty & WMName)
	if (XGetWindowProperty(p->display, p->window, net_wm_name, 0l,
			       (long) BUFSIZE, False, XA_STRING, &type_ret,
			       &format_ret, &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_STRING && format_ret == 8 &&
		    nitems_ret > 0) {
		    if (p->name) delete [] p->name;
		    p->name = nstrndup((const char *) data_ret, nitems_ret);
		}

		XFree(data_ret);
	    }

    if (dirty & WMWindowType)
	if (XGetWindowProperty(p->display, p->window, net_wm_window_type, 0l, 1l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 &&
		    nitems_ret == 1)
		    p->type = (WindowType) *((CARD32 *) data_ret);

		XFree(data_ret);
	    }

    if (dirty & WMStrut)
	if (XGetWindowProperty(p->display, p->window, net_wm_strut, 0l, 4l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 &&
		    nitems_ret == 4) {
		    CARD32 *d = (CARD32 *) data_ret;
		    p->strut.left   = d[0];
		    p->strut.right  = d[1];
		    p->strut.top    = d[2];
		    p->strut.bottom = d[3];
		}

		XFree(data_ret);
	    }

    if (dirty & WMIconGeometry)
	if (XGetWindowProperty(p->display, p->window, net_wm_icon_geometry, 0l, 4l,
			       False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 &&
		    nitems_ret == 4) {
		    CARD32 *d = (CARD32 *) data_ret;
		    p->icon_geom.pos.x       = d[0];
		    p->icon_geom.pos.y       = d[1];
		    p->icon_geom.size.width  = d[2];
		    p->icon_geom.size.height = d[3];
		}

		XFree(data_ret);
	    }

    if (dirty & WMIcon)
	readIcon(p);

    if (dirty & WMKDEDockWinFor)
	if (XGetWindowProperty(p->display, p->window, net_wm_kde_docking_window_for,
			       0l, 1l, False, XA_CARDINAL, &type_ret, &format_ret,
			       &nitems_ret, &unused, &data_ret))
	    if (data_ret) {
		if (type_ret == XA_CARDINAL && format_ret == 32 &&
		    nitems_ret == 1)
		    p->kde_dockwin_for = *((CARD32 *) data_ret);

		XFree(data_ret);
	    }
}
