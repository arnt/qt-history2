/****************************************************************************
**
** Implementation of QSignal class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsignal.h"
#include "qmetaobject.h"

/*!
    \class QSignal qsignal.h
    \brief The QSignal class can be used to send signals for classes
    that don't inherit QObject.

    \ingroup io
    \ingroup misc

    If you want to send signals from a class that does not inherit
    QObject, you can create an internal QSignal object to emit the
    signal. You must also provide a function that connects the signal
    to an outside object slot.  This is how we have implemented
    signals in the QMenuData class, which is not a QObject.

    In general, we recommend inheriting QObject instead. QObject
    provides much more functionality.

    You can set a single QVariant parameter for the signal with
    setValue().

    Note that QObject is a \e private base class of QSignal, i.e. you
    cannot call any QObject member functions from a QSignal object.

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

QSignal::QSignal( QObject *parent, const char *name )
    : QObject( parent, name )
{
    isSignal = TRUE;
#ifndef QT_NO_VARIANT
    val = 0;
#endif
}

/*!
    Destroys the signal. All connections are removed, as is the case
    with all QObjects.
*/
QSignal::~QSignal()
{
}
#ifndef QT_NO_VARIANT
// Returns TRUE if it matches ".+(.*int.*"
static inline bool intSignature( const char *member )
{
    QByteArray s( member );
    int p = s.find( '(' );
    return p > 0 && p < s.findRev( "int" );
}
#endif
/*!
    Connects the signal to \a member in object \a receiver.

    \sa disconnect(), QObject::connect()
*/

bool QSignal::connect( const QObject *receiver, const char *member )
{
#ifndef QT_NO_VARIANT
    if ( intSignature( member ) )
#endif
	return QObject::connect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
#ifndef QT_NO_VARIANT
    return QObject::connect( (QObject *)this, SIGNAL(signal(const QKernelVariant&)),
			     receiver, member );
#endif
}

/*!
    Disonnects the signal from \a member in object \a receiver.

    \sa connect(), QObject::disconnect()
*/

bool QSignal::disconnect( const QObject *receiver, const char *member )
{
#ifndef QT_NO_VARIANT
    if ( intSignature( member ) )
#endif
	return QObject::disconnect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
#ifndef QT_NO_VARIANT
    return QObject::disconnect( (QObject *)this, SIGNAL(signal(const QKernelVariant&)),
				receiver, member );
#endif
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
  Blocks the signal if \a b is TRUE, or unblocks the signal if \a b is FALSE.

  An activated signal disappears into hyperspace if it is blocked.

  \sa isBlocked(), activate(), QObject::blockSignals()
*/


/*!
    \fn void QSignal::activate()

    Emits the signal. If the platform supports QVariant and a
    parameter has been set with setValue(), this value is passed in
    the signal.
*/
void  QSignal::activate()
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
void QSignal::setValue( const QKernelVariant &value )
{
    val = value;
}

/*!
    Returns the signal's parameter
*/
QKernelVariant QSignal::value() const
{
    return val;
}
/*! \fn void QSignal::signal( const QKernelVariant & )
    \internal
*/
/*! \fn void QSignal::intSignal( int )
    \internal
*/

#ifndef QT_NO_COMPAT
/*! \obsolete */
void QSignal::setParameter( int value )
{
    val = value;
}

/*! \obsolete */
int QSignal::parameter() const
{
    return val.toInt();
}
#endif
#endif //QT_NO_VARIANT
