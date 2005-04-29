/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3signal.h"
#include "qmetaobject.h"
#include "qpointer.h"
#include "q3cstring.h"

/*!
    \class Q3Signal q3signal.h
    \brief The Q3Signal class can be used to send signals for classes
    that don't inherit QObject.

    \compat

    If you want to send signals from a class that does not inherit
    QObject, you can create an internal Q3Signal object to emit the
    signal. You must also provide a function that connects the signal
    to an outside object slot.  This is how we used to implement
    signals in Qt 3's QMenuData class, which was not a QObject. In Qt
    4, menus contain actions, which are QObjects.

    In general, we recommend inheriting QObject instead. QObject
    provides much more functionality.

    You can set a single QVariant parameter for the signal with
    setValue().

    Note that QObject is a \e private base class of Q3Signal, i.e. you
    cannot call any QObject member functions from a Q3Signal object.

    Example:
    \code
	#include <q3signal.h>

	class MyClass
	{
	public:
	    MyClass();
	    ~MyClass();

	    void doSomething();

	    void connect(QObject *receiver, const char *member);

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

	void MyClass::connect(QObject *receiver, const char *member)
	{
	    sig->connect(receiver, member);
	}
    \endcode
*/

/*!
    Constructs a signal object called \a name, with the parent object
    \a parent. These arguments are passed directly to QObject.
*/

Q3Signal::Q3Signal(QObject *parent, const char *name)
    : QObject(parent, name)
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
// Returns true if it matches ".+(.*int.*"
static inline bool intSignature(const char *member)
{
    Q3CString s(member);
    int p = s.find('(');
    return p > 0 && p < s.findRev("int");
}
#endif
/*!
    Connects the signal to \a member in object \a receiver.

    \sa disconnect(), QObject::connect()
*/

bool Q3Signal::connect(const QObject *receiver, const char *member)
{
#ifndef QT_NO_VARIANT
    if (intSignature(member))
#endif
	return QObject::connect((QObject *)this, SIGNAL(intSignal(int)), receiver, member);
#ifndef QT_NO_VARIANT
    return QObject::connect((QObject *)this, SIGNAL(signal(QVariant)),
			     receiver, member);
#endif
}

/*!
    Disonnects the signal from \a member in object \a receiver.

    \sa connect(), QObject::disconnect()
*/

bool Q3Signal::disconnect(const QObject *receiver, const char *member)
{
    if (!member)
	return QObject::disconnect((QObject *)this, 0, receiver, member);
#ifndef QT_NO_VARIANT
    if (intSignature(member))
#endif
	return QObject::disconnect((QObject *)this, SIGNAL(intSignal(int)), receiver, member);
#ifndef QT_NO_VARIANT
    return QObject::disconnect((QObject *)this, SIGNAL(signal(QVariant)),
				receiver, member);
#endif
}


/*!
  \fn bool Q3Signal::isBlocked() const
  \obsolete
  Returns true if the signal is blocked, or false if it is not blocked.

  The signal is not blocked by default.

  \sa block(), QObject::signalsBlocked()
*/

/*!
  \fn void Q3Signal::block(bool b)
  \obsolete
  Blocks the signal if \a b is true, or unblocks the signal if \a b is false.

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
    /* Create this Q3GuardedPtr on this, if we get destroyed after the intSignal (but before the variant signal)
       we cannot just emit the signal (because val has been destroyed already) */
    QPointer<Q3Signal> me = this;
    if(me)
	emit intSignal(val.toInt());
    if(me)
	emit signal(val);
#else
    emit intSignal(0);
#endif
}

#ifndef QT_NO_VARIANT
/*!
    Sets the signal's parameter to \a value
*/
void Q3Signal::setValue(const QVariant &value)
{
    val = value;
}

/*!
    Returns the signal's parameter
*/
QVariant Q3Signal::value() const
{
    return val;
}
/*! \fn void Q3Signal::signal(const QVariant &)
    \internal
*/
/*! \fn void Q3Signal::intSignal(int)
    \internal
*/

/*! \obsolete */
void Q3Signal::setParameter(int value)
{
    val = value;
}

/*! \obsolete */
int Q3Signal::parameter() const
{
    return val.toInt();
}
#endif //QT_NO_VARIANT
