/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.cpp#15 $
**
** Implementation of QApplication class
**
** Author  : Haavard Nord
** Created : 931107
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qobjcoll.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpalette.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapplication.cpp#15 $";
#endif


QApplication *qApp = 0;				// global application object
QWidget *QApplication::main_widget = 0;		// main application widget
QCursor *QApplication::appCursor   = 0;		// default application cursor
QPalette *QApplication::appPal	   = 0;		// default application palette
QFont   *QApplication::appFont     = 0;		// default application font
bool	 QApplication::starting_up = TRUE;	// app starting up
bool	 QApplication::closing_down  = FALSE;	// app closing down


#if defined(_WS_MAC_)
GUIStyle QApplication::appStyle = MacStyle;	// default style for Mac
#elif defined(_WS_WIN_)
GUIStyle QApplication::appStyle = WindowsStyle;	// default style for Windows
#elif defined(_WS_PM_)
GUIStyle QApplication::appStyle = PMStyle;	// default style for OS/2 PM
#elif defined(_WS_X11_)
GUIStyle QApplication::appStyle = MotifStyle;	// default style for X Windows
#endif


static QPalette *motifPalette = 0;

static void create_palettes()			// creates default palettes
{
    QColor lightBlue = blue.light();
    QColorGroup motif_nor( black, lightGray, lightGray.light(),
			   lightGray.dark(), gray, black, white );
    QColorGroup motif_dis( darkGray, blue, darkGray, lightGray, gray,
			   darkGray, white );
    QColorGroup motif_act( black, blue, darkBlue, lightBlue, blue,
			   black, white );
    motifPalette = new QPalette( motif_nor, motif_dis, motif_act );
}


QApplication::QApplication()
{
#if defined(CHECK_STATE)
    if ( qApp )
	warning( "QApplication: There should be only one application object" );
#endif
    quit_now = FALSE;
    quit_code = 0;
    qApp = this;
    create_palettes();
    appPal = new QPalette( *motifPalette );
    QWidget::createMapper();			// create widget mapper
    starting_up = FALSE;			// no longer starting up
}

QApplication::~QApplication()
{
    if ( appFont )
	delete appFont;
    if ( appCursor )
	delete appCursor;
    qApp = 0;
    closing_down = TRUE;
}


void QApplication::cleanup()			// cleanup application
{
    if ( !qApp ) {				// only if qApp deleted
	if ( main_widget )
	    delete main_widget;
	QWidget::destroyMapper();		// destroy widget mapper
	delete objectDict;			// delete object dictionary
    }
}


void QApplication::setStyle( GUIStyle s )	// set application GUI style
{
    appStyle = s;
}


QPalette *QApplication::palette()		// get application palette
{
    return appPal;
}

void QApplication::setPalette( const QPalette &p, bool forceAllWidgets )
{						// set application palette
    delete appPal;
    appPal = new QPalette( p.copy() );
    CHECK_PTR( appPal );
    if ( forceAllWidgets ) {			// update all widgets
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    w->setPalette( *appPal );
	    ++it;
	}
    }
}


void QApplication::quit( int retcode )		// quit application
{
    if ( !qApp )				// no global app object
	return;
    if ( qApp->quit_now )			// don't overwrite quit code
	return;
    qApp->quit_now = TRUE;
    qApp->quit_code = retcode;
}


bool QApplication::notify( QObject *receiver, QEvent *event )
{						// send event to object
#if defined(CHECK_NULL)
    if ( receiver == 0 )			// fatal error
	warning( "QApplication::notify: Unexpected NULL receiver" );
#endif
    return receiver->event( event );
}


bool QApplication::startingUp()			// is application starting up?
{
    return starting_up;
}

bool QApplication::closingDown()		// is application closing down?
{
    return closing_down;
}


#if !defined(_WS_X11_)

// The X implementation of these functions is in qapp_x11.cpp

void QApplication::flushX()	{}		// do nothing

void QApplication::syncX()	{}		// do nothing

#endif
