/****************************************************************************
**
** Implementation of Q3Signal class.
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

#include "q3signal.h"
#include "qmetaobject.h"

/*!
    \class Q3Signal q3signal.h

    \obsolete

    \brief The Q3Signal class can be used to send signals for classes
    that don't inherit QObject.

    \ingroup io
    \ingroup misc

    If you want to send signals from a class that does not inherit
    QObject, you can create an internal Q3Signal object to emit the
    signal. You must also provide a function that connects the signal
    to an outside object slot.  This is how we have implemented
    signals in the QMenuData class, which is not a QObject.

    In general, we recommend inheriting QObject instead. QObject
    provides much more functionality.

    You can set a single QVariant parameter for the signal with
    setValue().

    Note that QObject is a \e private base class of Q3Signal, i.e. you
    cannot call any QObject member functions from a Q3Signal object.

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
	    Q3Signal *sig;
	};

	MyClass::MyClass()
	{
	    sig = new Q3Signal;
	}

	MyClass::~MyClass()
	{
	    delete sig;
	}

	void MyClass::doSomething()
	{
	    // ... does something
	    sig->activate(); // emits the signal
	}

	void MyClass::connect( QObject *receiver, const char *member )
	{
	    sig->connect( receiver, member );
	}
    \endcode
*/

/*!
    Constructs a signal object called \a name, with the parent object
    \a parent. These arguments are passed directly to QObject.
*/

Q3Signal::Q3Signal( QObject *parent, const char *name )
    : QObject( parent, name )
{
#ifndef QT_NO_VARIANT
    val = 0;
#endif
}

/*!
    Destroys the signal. All connections are removed, as is the case
    with all QObjects.
*/
Q3Signal::~Q3Signal()
{
}
#ifndef QT_NO_VARIANT
// Returns TRUE if it matches ".+(.*int.*"
static inline bool intSignature( const char *member )
{
    QByteArray s( member );
    int p = s.indexOf( '(' );
    return p > 0 && p < s.lastIndexOf( "int" );
}
#endif
/*!
    Connects the signal to \a member in object \a receiver.

    \sa disconnect(), QObject::connect()
*/

bool Q3Signal::connect( const QObject *receiver, const char *member )
{
#ifndef QT_NO_VARIANT
    if ( intSignature( member ) )
#endif
	return QObject::connect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
#ifndef QT_NO_VARIANT
    return QObject::connect( (QObject *)this, SIGNAL(signal(const QCoreVariant&)),
			     receiver, member );
#endif
}

/*!
    Disonnects the signal from \a member in object \a receiver.

    \sa connect(), QObject::disconnect()
*/

bool Q3Signal::disconnect( const QObject *receiver, const char *member )
{
    if (!member)
	return QObject::disconnect( (QObject *)this, 0, receiver, member);
#ifndef QT_NO_VARIANT
    if ( intSignature( member ) )
#endif
	return QObject::disconnect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
#ifndef QT_NO_VARIANT
    return QObject::disconnect( (QObject *)this, SIGNAL(signal(const QCoreVariant&)),
				receiver, member );
#endif
}


/*!
  \fn bool Q3Signal::isBlocked() const
  \obsolete
  Returns TRUE if the signal is blocked, or FALSE if it is not blocked.

  The signal is not blocked by default.

  \sa block(), QObject::signalsBlocked()
*/

/*!
  \fn void Q3Signal::block( bool b )
  \obsolete
  Blocks the signal if \a b is TRUE, or unblocks the signal if \a b is FALSE.

  An activated signal disappears into hyperspace if it is blocked.

  \sa isBlocked(), activate(), QObject::blockSignals()
*/


/*!
    \fn void Q3Signal::activate()

    Emits the signal. If the platform supports QVariant and a
    parameter has been set with setValue(), this value is passed in
    the signal.
*/
void  Q3Signal::activate()
{
#ifndef QT_NO_VARIANT
    emit intSignal( val.toInt() );
    emit signal( val );
#else
    emit intSignal(0);
#endif
}

#ifndef QT_NO_VARIANT
/*!
    Sets the signal's parameter to \a value
*/
void Q3Signal::setValue( const QCoreVariant &value )
{
    val = value;
}

/*!
    Returns the signal's parameter
*/
QCoreVariant Q3Signal::value() const
{
    return val;
}
/*! \fn void Q3Signal::signal( const QCoreVariant & )
    \internal
*/
/*! \fn void Q3Signal::intSignal( int )
    \internal
*/

#ifdef QT_COMPAT
/*! \obsolete */
void Q3Signal::setParameter( int value )
{
    val = value;
}

/*! \obsolete */
int Q3Signal::parameter() const
{
    return val.toInt();
}
#endif
#endif //QT_NO_VARIANT
