/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qt_x11.cpp#3 $
**
** Implementation of Qt calls to X11
**
** Created : 970529
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwidget.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

/*
  Internal Qt functions to create X windows.  We have put them in
  separate functions to allow the programmer to override them by custom
  versions.
*/

Window qt_XCreateWindow( const QWidget*, Display *display, Window parent,
			 int x, int y, uint w, uint h,
			 int borderwidth, int depth,
			 uint windowclass, Visual *visual,
			 ulong valuemask, XSetWindowAttributes *attributes )
{
    return XCreateWindow( display, parent, x, y, w, h, borderwidth, depth,
			  windowclass, visual, valuemask, attributes );
}


Window qt_XCreateSimpleWindow( const QWidget*, Display *display, Window parent,
			       int x, int y, uint w, uint h, int borderwidth,
			       ulong border, ulong background )
{
    return XCreateSimpleWindow( display, parent, x, y, w, h, borderwidth,
				border, background );
}


void qt_XDestroyWindow( const QWidget*, Display *display, Window window )
{
    XDestroyWindow( display, window );
}
