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

#include "qcoreapplication.h"
#include "qcoreevent.h"
#include "qsignal.h"
#include "qthread.h"
#include "qmetaobject.h"
#include "qvarlengtharray.h"
#include "private/qobject_p.h"

/*!
    \class QSignal
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
       0,    0, // classinfo
       1,   10, // signals
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

       0        // eod
};

const QMetaObject *QSignalEmitter::metaObject() const
{
    return &staticMetaObject;
}

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

void *QSignalEmitter::qt_metacast(const char *clname)
{
    if (!clname) return 0;
    if (!strcmp(clname, staticMetaObject.className()))
        return (void*)this;
    return 0;
}

int QSignalEmitter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    int _id_global = _id;
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMember) {
        if (_id < 1)
            QMetaObject::activate(this, _id_global, _a);
        _id -= 1;
    }
    return _id;
}

void QSignalEmitter::activate(const void *_t1)
{
    void *_a[] = { 0, const_cast<void *>(_t1) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

bool QSignalEmitter::connect(const QObject *receiver, const char *member, Qt::ConnectionType type)
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
bool qInvokeMetaMember(QObject *obj, const char *member, Qt::ConnectionType type,
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
    if (!obj)
        return false;

    QVarLengthArray<char, 512> sig;
    int len = qstrlen(member);
    if (len <= 0)
        return false;
    sig.append(member, len);
    sig.append('(');

    enum { ParamCount = 11 };
    const char *typeNames[] = {ret.name(), val0.name(), val1.name(), val2.name(), val3.name(),
                               val4.name(), val5.name(), val6.name(), val7.name(), val8.name(),
                               val9.name()};

    int i;
    for (i = 1; i < ParamCount; ++i) {
        len = qstrlen(typeNames[i]);
        if (len <= 0)
            break;
        sig.append(typeNames[i], len);
        sig.append(',');
    }
    if (i == 1)
        sig.append(')'); // no parameters
    else
        sig[sig.size() - 1] = ')';
    sig.append('\0');

    int idx = obj->metaObject()->indexOfMember(sig.constData());
    if (idx < 0) {
        QByteArray norm = QMetaObject::normalizedSignature(sig.constData());
        idx = obj->metaObject()->indexOfMember(norm.constData());
    }
    if (idx < 0)
        return false;

    // check return type
    if (ret.data()) {
        const char *retType = obj->metaObject()->member(idx).typeName();
        if (qstrcmp(ret.name(), retType) != 0)
            return false;
    }
    void *param[] = {ret.data(), val0.data(), val1.data(), val2.data(), val3.data(), val4.data(),
                     val5.data(), val6.data(), val7.data(), val8.data(), val9.data()};
    if (type == Qt::AutoConnection) {
        type = QThread::currentThread() == obj->thread()
               ? Qt::DirectConnection
               : Qt::QueuedConnection;
    }

    if (type != Qt::QueuedConnection) {
        return obj->qt_metacall(QMetaObject::InvokeMetaMember, idx, param) < 0;
    } else {
        if (ret.data()) {
            qWarning("qInvokeMetaMember: Unable to invoke methods with return values in queued "
                     "connections.");
            return false;
        }
        int nargs = 1; // include return type
        void **args = (void **) qMalloc(ParamCount * sizeof(void *));
        int *types = (int *) qMalloc(ParamCount * sizeof(int));
        types[0] = 0; // return type
        args[0] = 0;
        for (i = 1; i < ParamCount; ++i) {
            types[i] = QMetaType::type(typeNames[i]);
            if (types[i]) {
                args[i] = QMetaType::construct(types[i], param[i]);
                ++nargs;
            } else if (param[i]) {
                qWarning("qInvokeMetaMember: Unable to handle unregistered datatype '%s'",
                         typeNames[i]);
                return false;
            }
        }

        QCoreApplication::postEvent(obj, new QMetaCallEvent(idx, nargs, types, args));
    }
    return true;
}
