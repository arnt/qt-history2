/****************************************************************************
**
** Implementation of Qt calls to X11.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwidget.h"
#include "qt_x11_p.h"


/*
  Internal Qt functions to create X windows.  We have put them in
  separate functions to allow the programmer to reimplement them by
  custom versions.
*/

Window qt_XCreateWindow(const QWidget*, Display *display, Window parent,
                         int x, int y, uint w, uint h,
                         int borderwidth, int depth,
                         uint windowclass, Visual *visual,
                         ulong valuemask, XSetWindowAttributes *attributes)
{
    return XCreateWindow(display, parent, x, y, w, h, borderwidth, depth,
                          windowclass, visual, valuemask, attributes);
}


Window qt_XCreateSimpleWindow(const QWidget*, Display *display, Window parent,
                               int x, int y, uint w, uint h, int borderwidth,
                               ulong border, ulong background)
{
    return XCreateSimpleWindow(display, parent, x, y, w, h, borderwidth,
                                border, background);
}


void qt_XDestroyWindow(const QWidget*, Display *display, Window window)
{
    XDestroyWindow(display, window);
}
