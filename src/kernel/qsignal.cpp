/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignal.cpp#28 $
**
** Implementation of QSignal class
**
** Created : 941201
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsignal.h"
#include "qmetaobject.h"
#include <ctype.h>

/*!
  \class QSignal qsignal.h
  \brief The QSignal class can be used to send signals without parameters.

  \ingroup kernel

  QSignal is a simple extension of QObject that can send plain signals
  without parameters.  If you want to send signals from a class that does
  not inherit QObject, you can create an internal QSignal object to emit
  the signal. You must also provide a function that connects the signal to
  an outside object slot.  This is how we have implemented signals in the
  QMenuData class, which is not a QObject.

  In general, we recommend inheriting QObject instead.	QObject provides
  much more functionality.

  Note that QObject is a \e private base class of QSignal, i.e. you cannot
  call any QObject member functions from a QSignal object.

  Example:
  \code
    #include <qsignal.h>

    class MyClass
    {
    public:
	MyClass();
       ~MyClass();

	void doSomething();

	void connect( QObject *receiver, const char *member );

    private:
	QSignal *sig;
    };

    MyClass::MyClass()
    {
	sig = new QSignal;
    }

    MyClass::~MyClass()
    {
	delete sig;
    }

    void MyClass::doSomething()
    {
	// ... does something
	sig->activate();	// activates the signal
    }

    void MyClass::connect( QObject *receiver, const char *member )
    {
	sig->connect( receiver, member );
    }
  \endcode
*/


QMetaObject *QSignal::metaObj = 0;


/*!
  Constructs a signal object with the parent object \e parent and a \e name.
  These arguments are passed directly to QObject.
*/

QSignal::QSignal( QObject *parent, const char *name )
    : QObject( parent, name )
{
    if ( !metaObj )				// will create object dict
	initMetaObject();
    isSignal = TRUE;
}


/*!
  \fn const char *QSignal::name() const
  Returns the name of this signal object.

  Since QObject is a private base class, we have added this function, which
  calls QObject::name().
*/

/*!
  \fn void QSignal::setName( const char *name )
  Sets the name of this signal object to \e name.

  Since QObject is a private base class, we have added this function, which
  calls QObject::setName().
*/

/* NOTE: should not be documented */

const char *QSignal::className() const
{
    return "QSignal";
}


/*!
  Connects the signal to \e member in object \e receiver.
  \sa disconnect(), QObject::connect()
*/

bool QSignal::connect( const QObject *receiver, const char *member )
{
    return QObject::connect( (QObject *)this, SIGNAL(x()),
			     receiver, member );
}

/*!
  Disonnects the signal from \e member in object \e receiver.
  \sa connect(), QObject::disconnect()
*/

bool QSignal::disconnect( const QObject *receiver, const char *member )
{
    return QObject::disconnect( (QObject *)this, SIGNAL(x()),
				receiver, member );
}


/*!
  \fn bool QSignal::isBlocked() const
  Returns TRUE if the signal is blocked, or FALSE if it is not blocked.

  The signal is not blocked by default.

  \sa block(), QObject::signalsBlocked()
*/

/*!
  \fn void QSignal::block( bool b )
  Blocks the signal if \e b is TRUE, or unblocks the signal if \e b is FALSE.

  An activated signal disappears into hyperspace if it is blocked.

  \sa isBlocked(), activate(), QObject::blockSignals()
*/


/*!
  \fn void QSignal::activate()
  Emits the signal.
  \sa isBlock()
*/


void QSignal::dummy()				// just for the meta object
{						//   should never be called
#if defined(CHECK_STATE)
    qWarning( "QSignal: Internal error" );
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
    signal_tbl[0].name = (char *)"x()";		// fake signal x in meta object
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "QSignal", "QObject",
	0, 0,
	signal_tbl, 1 );
}
