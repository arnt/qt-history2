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
    that do not inherit QObject.

    \ingroup io
    \ingroup misc

    If you want to send signals from a class that does not inherit
    QObject, you can create an internal \c{QSignal<T> object to emit
    the signal. \c T is the type of the signal's argument, it can be
    \c void. You must also provide a function that connects the signal
    to an outside object slot.

    In general, we recommend inheriting QObject instead. QObject
    provides much more functionality.

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
	    QSignal<void> sig;
	};

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
  \fn QSignal::QSignal()

  Constructs a signal object
*/


/*!
  \fn QSignal::~QSignal()

  Destroys the signal. All connections are removed.
*/

/*!
  \fn bool QSignal::connect( const QObject *receiver, const char *member, Qt::ConnectionType type)

  Connects the signal to \a member in object \a receiver using the
  given connection \a type.

  \sa disconnect()
*/

/*!
  \fn bool QSignal::disconnect( const QObject *receiver, const char *member )

  Disconnects the signal from \a member in object \a receiver.

  \sa connect()
*/

/*!
  \fn void QSignal::activate(const T& t)

  Emits the signal with value \a t.
*/



static const uint qt_meta_data_QSignalEmitter[] = {

 // content:
       1,       // revision
       0,       // classname
       0,   12, // classinfo
       1,   12, // signals
       0,   17, // slots
       0,   17, // properties
       0,   17, // enums/sets

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0,

       0        // eod
};

QSignalEmitter::QSignalEmitter(const char *type)
    :stringdata("QSignalEmitter\0\0activated(", 26)
{
    if (type)
	stringdata += type;
    stringdata += ')';
    staticMetaObject.d.superdata = &QObject::staticMetaObject;
    staticMetaObject.d.data = qt_meta_data_QSignalEmitter;
    staticMetaObject.d.stringdata = stringdata;
}

QSignalEmitter::~QSignalEmitter()
{
}

void *QSignalEmitter::qt_metacast(const char *clname) const
{
    if (!clname) return 0;
    if (!strcmp(clname, staticMetaObject.className()))
	return (void*)this;
    return 0;
}

void QSignalEmitter::activate(void *_t1)
{
    void *_a[] = { 0, _t1 };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

bool QSignalEmitter::connect(const QObject *receiver, const char *member, ConnectionType type)
{
    QByteArray signal = stringdata.data() + 16;
    signal.prepend('0' + SIGNAL_CODE);
    return QObject::connect(this, signal, receiver, member, type);
}

bool QSignalEmitter::disconnect(const QObject *receiver, const char *member)
{
    QByteArray signal = stringdata.data() + 16;
    signal.prepend('0' + SIGNAL_CODE);
    return QObject::disconnect(this, signal, receiver, member);
}
