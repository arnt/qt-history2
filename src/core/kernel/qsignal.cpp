/****************************************************************************
**
** Implementation of QSignal class.
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

#include "qcoreapplication.h"
#include "qcoreevent.h"
#include "qsignal.h"
#include "qthread.h"
#include "qmetaobject.h"
#include "private/qobject_p.h"

/*!
    \class QSignal qsignal.h
    \brief The QSignal class can be used to send signals for classes
    that do not inherit QObject.

    \ingroup io
    \ingroup misc

    If you want to send signals from a class that does not inherit
    QObject, you can create an internal \c{QSignal<T>} object to emit
    the signal. \c T is the type of the signal's argument, and it can
    be \c void. You must also provide a function that connects the
    signal to an outside object slot.

    In general, we recommend inheriting QObject instead, since QObject
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

            void connect(QObject *receiver, const char *member);

        private:
            QSignal<void> sig;
        };

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
    \fn QSignal::QSignal()

    Constructs a signal object
*/


/*!
    \fn QSignal::~QSignal()

    Destroys the signal. All connections are removed.
*/

/*!
    \fn bool QSignal::connect(const QObject *receiver, const char *member, Qt::ConnectionType type)

    Connects the signal to \a member in object \a receiver using the
    given connection \a type.

    \sa disconnect()
*/

/*!
    \fn bool QSignal::disconnect(const QObject *receiver, const char *member)

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
{
    stringdata = QByteArray("QSignalEmitter\0\0activated(", 26);
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
    QByteArray signal = stringdata.constData() + 16;
    signal.prepend('0' + QSIGNAL_CODE);
    return QObject::connect(this, signal, receiver, member, type);
}

bool QSignalEmitter::disconnect(const QObject *receiver, const char *member)
{
    QByteArray signal = stringdata.constData() + 16;
    signal.prepend('0' + QSIGNAL_CODE);
    return QObject::disconnect(this, signal, receiver, member);
}

/*! internal
 */
bool qInvokeSlot(QObject *obj, const char *slotName, Qt::ConnectionType type,
                 QGenericReturnArgument ret,
                 QGenericArgument val0,
                 QGenericArgument val1,
                 QGenericArgument val2,
                 QGenericArgument val3,
                 QGenericArgument val4,
                 QGenericArgument val5,
                 QGenericArgument val6,
                 QGenericArgument val7,
                 QGenericArgument val8,
                 QGenericArgument val9)
{
    if (!obj || !slotName)
        return false;

    QByteArray sig;
    sig.reserve(512);
    sig.append(slotName).append('(');
    if (val0.name()
        && sig.append(val0.name()).append(',').size()
        && val1.name()
        && sig.append(val1.name()).append(',').size()
        && val2.name()
        && sig.append(val2.name()).append(',').size()
        && val3.name()
        && sig.append(val3.name()).append(',').size()
        && val4.name()
        && sig.append(val4.name()).append(',').size()
        && val5.name()
        && sig.append(val5.name()).append(',').size()
        && val6.name()
        && sig.append(val6.name()).append(',').size()
        && val7.name()
        && sig.append(val7.name()).append(',').size()
        && val8.name()
        && sig.append(val8.name()).append(',').size()
        && val9.name())
    {
        sig.append(val9.name()).append(',');
    }
    if (sig.endsWith(','))
        sig.truncate(sig.length() - 1);
    sig.append(')');

    int idx = obj->metaObject()->indexOfSlot(sig.constData());
    if (idx < 0)
        idx = obj->metaObject()->indexOfSlot(
                QMetaObject::normalizedSignature(sig.constData()).constData());
    if (idx < 0)
        return false;

    // check return type
    if (ret.data() && qstrcmp(ret.name(), obj->metaObject()->slot(idx).type()) != 0)
        return false;

    void *param[] = {ret.data(), val0.data(), val1.data(), val2.data(), val3.data(), val4.data(),
                     val5.data(), val6.data(), val7.data(), val8.data(), val9.data()};
    if (type == Qt::AutoConnection)
        type = QThread::currentThread() == obj->thread() ? Qt::DirectConnection
                                                         : Qt::QueuedConnection;

    if (type != Qt::QueuedConnection) {
        return obj->qt_metacall(QMetaObject::InvokeSlot, idx, param) < 0;
    } else {
        int nargs = 1; // include return type
        void **args = (void **) qMalloc(11 * sizeof(void *));
        int *types = (int *) qMalloc(11 * sizeof(int));
        const char *typeNames[] = {ret.name(), val0.name(), val1.name(), val2.name(), val3.name(),
                             val4.name(), val5.name(), val6.name(), val7.name(), val8.name(),
                             val9.name()};
        types[0] = 0; // return type
        args[0] = 0;
        for (int i = 1; i < 11; ++i) {
            types[i] = QMetaType::type(typeNames[i]);
            if (types[i]) {
                args[i] = QMetaType::copy(types[i], param[i]);
                ++nargs;
            } else if (param[i]) {
                qWarning("Unable to handle unregistered datatype '%s'", typeNames[i]);
                return false;
            }
        }

        QCoreApplication::postEvent(obj,
               new QMetaCallEvent(QEvent::InvokeSlot, idx, obj, nargs, types, args));
    }
    return true;
}

