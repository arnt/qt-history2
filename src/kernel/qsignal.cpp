/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignal.cpp#34 $
**
** Implementation of QSignal class
**
** Created : 941201
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsignal.h"
#include "qmetaobject.h"
#include <ctype.h>
#include "qregexp.h"

/*!
  \class QSignal qsignal.h
  \brief The QSignal class can be used to send signals without parameters.

  \ingroup misc

  QSignal is a simple extension of QObject that can send plain signals
  without parameters.  If you want to send signals from a class that does
  not inherit QObject, you can create an internal QSignal object to emit
  the signal. You must also provide a function that connects the signal to
  an outside object slot.  This is how we have implemented signals in the
  QMenuData class, which is not a QObject.

  In general, we recommend inheriting QObject instead.	QObject provides
  much more functionality.

  Note that QObject is a \e private base class of QSignal, i.e., you cannot
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

/*!
  Constructs a signal object with the parent object \e parent and a \e name.
  These arguments are passed directly to QObject.
*/

QSignal::QSignal( QObject *parent, const char *name )
    : QObject( parent, name )
{
    isSignal = TRUE;
    d = 0;
    val = 0;
}

/*!
  Destructs the signal.  All connections are removed, as is the case
  with all QObjects.
*/
QSignal::~QSignal()
{
}

/*!
  Connects the signal to \e member in object \e receiver.
  \sa disconnect(), QObject::connect()
*/

bool QSignal::connect( const QObject *receiver, const char *member )
{
    QRegExp regexp( "*(*[int]*)", TRUE, TRUE );
    if ( regexp.match( member ) )
	return QObject::connect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
    return QObject::connect( (QObject *)this, SIGNAL(signal(const QVariant&)),
			     receiver, member );
}

/*!
  Disonnects the signal from \e member in object \e receiver.
  \sa connect(), QObject::disconnect()
*/

bool QSignal::disconnect( const QObject *receiver, const char *member )
{
    QRegExp regexp( "*(*[int]*)", TRUE, TRUE );
    if ( regexp.match( member ) )
	return QObject::disconnect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
    return QObject::disconnect( (QObject *)this, SIGNAL(signal(const QVariant&)),
				receiver, member );
}


/*!
  \fn bool QSignal::isBlocked() const
  \obsolete
  Returns TRUE if the signal is blocked, or FALSE if it is not blocked.

  The signal is not blocked by default.

  \sa block(), QObject::signalsBlocked()
*/

/*!
  \fn void QSignal::block( bool b )
  \obsolete
  Blocks the signal if \e b is TRUE, or unblocks the signal if \e b is FALSE.

  An activated signal disappears into hyperspace if it is blocked.

  \sa isBlocked(), activate(), QObject::blockSignals()
*/


/*!
  \fn void QSignal::activate()
  Emits the signal.
*/
void  QSignal::activate()
{
    emit intSignal( val.toInt() );
    emit signal( val );
}


/*!
  \fn void  QSignal::activate()
  \obsolete

  Sets the signal's parameter to \a value.
*/

/*!
  \fn int QSignal::parameter() const
  \obsolete

  Returns the signal's parameter.
*/

/*!
  Sets the signal's parameter to \a value
*/
void QSignal::setValue( const QVariant &value )
{
    val = value;
}

/*!
  Returns the signal's parameter
*/
QVariant QSignal::value() const
{
    return val;
}
