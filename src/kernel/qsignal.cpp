/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignal.cpp#2 $
**
** Implementation of QSignal class
**
** Author  : Haavard Nord
** Created : 941201
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qsignal.h"
#include "qmetaobj.h"
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qsignal.cpp#2 $";
#endif


QMetaObject *QSignal::metaObj = 0;


QSignal::QSignal( QObject *parent, const char *name ) : QObject( parent, name )
{
    if ( !metaObj )				// will create object dict
	initMetaObject();
    isSignal = TRUE;
}


const char *QSignal::className() const		// get class name
{
    return "QSignal";
}


bool QSignal::connect( const QObject *receiver, const char *member )
{						// connect signal to method
    return QObject::connect( (QObject *)this, SIGNAL(x()),
			     receiver, member );
}


bool QSignal::disconnect( const QObject *receiver, const char *member )
{
    return QObject::disconnect( (QObject *)this, SIGNAL(x()),
 				receiver, member );
}


void QSignal::dummy()				// just for the meta object
{						//   should never be called
#if defined(CHECK_STATE)
    debug( "QSignal: Internal error" );
#endif
}


void QSignal::initMetaObject()			// initialize meta object
{
    if ( metaObj )
	return;
    if ( !QObject::metaObject() )
	QObject::initMetaObject();
    typedef void (QSignal::*m3_t0)();
    m3_t0 v3_0 = &QSignal::dummy;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "x()";			// fake signal x in meta object
    signal_tbl[0].ptr = *((QMember*)&v3_0);
    metaObj = new QMetaObject( "QSignal", "QObject",
	0, 0,
	0, 0,
	signal_tbl, 1 );
}
