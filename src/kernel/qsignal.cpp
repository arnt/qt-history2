/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignal.cpp#9 $
**
** Implementation of QSignal class
**
** Author  : Haavard Nord
** Created : 941201
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qsignal.h"
#include "qmetaobj.h"
#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qsignal.cpp#9 $")

/*! \class QSignal qsignal.h

  \brief The QSignal class is an internal class, used in the
  signal/slot mechanism.

  \internal

  This class is not yet documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */

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
    typedef void (QSignal::*m2_t0)();
    m2_t0 v2_0 = &QSignal::dummy;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = (char *)"x()";			// fake signal x in meta object
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "QSignal", "QObject",
	0, 0,
	signal_tbl, 1 );
}
