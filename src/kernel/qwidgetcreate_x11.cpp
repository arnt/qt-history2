/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidgetcreate_x11.cpp#8 $
**
** Implementation of Qt calls to X11
**
** Created : 970529
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qwidget.h"
#include "qt_x11.h"


/*
  Internal Qt functions to create X windows.  We have put them in
  separate functions to allow the programmer to reimplement them by
  custom versions.
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
