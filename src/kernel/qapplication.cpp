/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.cpp#1 $
**
** Implementation of QApplication class
**
** Author  : Haavard Nord
** Created : 931107
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qobjcoll.h"
#include "qwidget.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapplication.cpp#1 $";
#endif


QApplication *qApp = 0;				// global application object
QWidget *QApplication::main_widget = 0;		// main application widget

QApplication::QApplication()
{
#if defined(CHECK_STATE)
    if ( qApp )
	warning( "QApplication: There should be only one application object" );
#endif
    quit_now = FALSE;
    quit_code = 0;
    qApp = this;
    QWidget::createMapper();			// create widget mapper
}

QApplication::~QApplication()
{
    qApp = 0;
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


void QApplication::quit( int retcode )		// quit application
{
    if ( !qApp )				// no global app object
	return;
    if ( qApp->quit_now )			// don't overwrite quit code
	return;
    qApp->quit_now = TRUE;
    qApp->quit_code = retcode;
}


bool QApplication::notify( QObject *obj, QEvent *evt )
{						// send event to object
    return obj->event( evt );
}
