/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclb_x11.cpp#1 $
**
** Implementation of QClipboard class for X11
**
** Author  : Haavard Nord
** Created : 960430
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qclipbrd.h"
#include "qapp.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qclb_x11.cpp#1 $")


extern Time qt_x_clipboardtime;			// def. in qapp_x11.cpp


/*----------------------------------------------------------------------------
  Clears the clipboard contents.
 ----------------------------------------------------------------------------*/

void QClipboard::clear()
{
}


void *QClipboard::data( const char *format ) const
{
    bool text = FALSE;
    if ( strcmp(format,"TEXT") == 0 ) {
	text = TRUE;
    } else if ( strcmp(format,"PIXMAP") ) {
#if defined(CHECK_RANGE)
	warning( "QClipboard::data: PIXMAP format not supported" );
#endif
	return 0;
    } else {
#if defined(CHECK_RANGE)
	warning( "QClipboard::data: Unknown format: %s", format );
#endif
	return 0;
    }
    
    Display *dpy = qt_xdisplay();
    if ( XGetSelectionOwner(dpy,XA_PRIMARY) == None ) {
	
    }
}

void QClipboard::setData( const char *format, void *data )
{
    bool text = FALSE;
#if defined(DEBUG)
    ASSERT( data != 0 );
#endif
    if ( strcmp(format,"TEXT") == 0 ) {
	text = TRUE;
    } else if ( strcmp(format,"PIXMAP") ) {
#if defined(CHECK_RANGE)
	warning( "QClipboard::setData: PIXMAP format not supported" );
#endif
	return;
    } else {
#if defined(CHECK_RANGE)
	warning( "QClipboard::setData: Unknown format: %s", format );
#endif
	return;
    }

    Display *dpy = qt_xdisplay();
    QWidget *owner = qApp->mainWidget();
    if ( !owner ) {
#if defined(CHECK_NULL)
	warning( "QClipboard::setData: A main widget is required" );
#endif
	return;
    }

    WId wid = owner->id();
    XSetSelectionOwner( dpy, XA_PRIMARY, wid, qt_x_clipboardtime );
    if ( XGetSelectionOwner(dpy,XA_PRIMARY) != wid ) {
#if defined(DEBUG)
	warning( "QClipboard::setData: Cannot set X11 selection owner" );
#endif
	return;
    }

    
}


/*----------------------------------------------------------------------------
  Handles event specifically for the clipboard.
 ----------------------------------------------------------------------------*/

bool QClipboard::event( QEvent *e )
{
    return TRUE;
}
