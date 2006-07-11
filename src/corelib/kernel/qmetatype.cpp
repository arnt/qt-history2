/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmetatype.h"
#include "qobjectdefs.h"
#include "qbytearray.h"
#include "qreadwritelock.h"
#include "qstring.h"
#include "qvector.h"

/*!
    \macro Q_DECLARE_METATYPE(Type)
    \relates QMetaType

    This macro makes the type \a Type known to QMetaType. It is
    needed to use the type \a Type as a custom type in QVariant.

    Ideally, this macro should be placed below the declaration of
    the class or struct. If that is not possible, it can be put in
    a private header file which has to be included every time that
    type is used in a QVariant.

    Adding a Q_DECLARE_METATYPE() makes the type known to all template
    based functions, including QVariant. In addition to declaring the type,
    you must call qInitMetaType() if you intend to use the type in
    queued signal and slot connections.

    This example shows a typical use case of Q_DECLARE_METATYPE():

    \code
        struct MyStruct
        {
            int i;
            ...
        };

        Q_DECLARE_METATYPE(MyStruct)
    \endcode

    If \c MyStruct is in a namespace, the Q_DECLARE_METATYPE() macro
    has to be outside the namespace:

    \code
        namespace MyNamespace
        {
            ...
        }

        Q_DECLARE_METATYPE(MyNamespace::MyStruct)
    \endcode

    Since \c{MyStruct} is now known to QMetaType, it can be used in QVariant:

    \code
        MyStruct s;
        QVariant var;
        var.setValue(s); // copy s into the variant

        ...

        // retrieve the value
        MyStruct s2 = var.value<MyStruct>();
    \endcode

    \sa qInitMetaType(), Q_DECLARE_METATYPE()
*/

/*!
    \enum QMetaType::Type

    These are the built-in types supported by QMetaType:

    \value Void \c void
    \value Bool \c bool
    \value Int \c int
    \value UInt \c{unsigned int}
    \value Double \c double
    \value QChar QChar
    \value QString QString
    \value QByteArray QByteArray

    \value VoidStar \c{void *}
    \value Long \c{long}
    \value Short \c{short}
    \value Char \c{char}
    \value ULong \c{unsigned long}
    \value UShort \c{unsigned short}
    \value UChar \c{unsigned char}
    \value Float \c float
    \value QObjectStar QObject *
    \value QWidgetStar QWidget *

    \value User  Base value for user types

    Additional types can be registered using Q_DECLARE_METATYPE().

    \sa type(), typeName()
*/

/*!
    \class QMetaType
    \brief The QMetaType class manages named types in the meta-object system.

    \ingroup objectmodel
    \threadsafe

    The class is used as a helper to marshall types in QVariant and
    in queued signals and slots connections. It associates a type
    name to a type so that it can be created and destructed
    dynamically at run-time. Declare new types with Q_DECLARE_METATYPE()
    to make them available to QVariant and other template-based functions.
    Call qInitMetaType() to make type available to non-template based
    functions, such as the queued signal and slot connections.

    Any class or struct that has a public default
    constructor, a public copy constructor, and a public destructor
    can be registered.

    The following code allocates and destructs an instance of
    \c{MyClass}:

    \code
        int id = QMetaType::type("MyClass");
        if (id != -1) {
            void *myClassPtr = QMetaType::construct(id);
            ...
            QMetaType::destroy(id, myClassPtr);
            myClassPtr = 0;
        }
    \endcode

    If we want the stream operators \c operator<<() and \c
    operator>>() to work on QVariant objects that store custom types,
    the custom type must provide \c operator<<() and \c operator>>()
    operators.

    \sa Q_DECLARE_METATYPE(), QVariant::setValue(), QVariant::value(), QVariant::fromValue()
*/

static const struct { const char * typeName; int type; } types[] = {
    {"void*", QMetaType::VoidStar},
    {"long", QMetaType::Long},
    {"int", QMetaType::Int},
    {"short", QMetaType::Short},
    {"char", QMetaType::Char},
    {"ulong", QMetaType::ULong},
    {"unsigned long", QMetaType::ULong},
    {"uint", QMetaType::UInt},
    {"unsigned int", QMetaType::UInt},
    {"ushort", QMetaType::UShort},
    {"unsigned short", QMetaType::UShort},
    {"uchar", QMetaType::UChar},
    {"unsigned char", QMetaType::UChar},
    {"bool", QMetaType::Bool},
    {"float", QMetaType::Float},
    {"double", QMetaType::Double},
    {"QChar", QMetaType::QChar},
    {"QByteArray", QMetaType::QByteArray},
    {"QString", QMetaType::QString},
    {"QObject*", QMetaType::QObjectStar},
    {"QWidget*", QMetaType::QWidgetStar},
    {"void", QMetaType::Void},
    {"", QMetaType::Void},
    {0, QMetaType::Void}
};

class QCustomTypeInfo
{
public:
    QCustomTypeInfo() : typeName(0, '\0'), constr(0), destr(0)
#ifndef QT_NO_DATASTREAM
    , saveOp(0), loadOp(0)
#endif
    {}

    QByteArray typeName;
    QMetaType::Constructor constr;
    QMetaType::Destructor destr;
#ifndef QT_NO_DATASTREAM
    QMetaType::SaveOperator saveOp;
    QMetaType::LoadOperator loadOp;
#endif
};

Q_GLOBAL_STATIC(QVector<QCustomTypeInfo>, customTypes)
Q_GLOBAL_STATIC(QReadWriteLock, customTypesLock)

#ifndef QT_NO_DATASTREAM
/*! \internal
*/
void QMetaType::registerStreamOperators(const char *typeName, SaveOperator saveOp,
                                        LoadOperator loadOp)
{
    int idx = type(typeName);
    if (!idx)
        return;

    QVector<QCustomTypeInfo> *ct = customTypes();
    if (!ct)
        return;
    QWriteLocker locker(customTypesLock());
    QCustomTypeInfo &inf = (*ct)[idx - User];
    inf.saveOp = saveOp;
    inf.loadOp = loadOp;
}
#endif

/*!
    Returns the type name associated with the given \a type, or 0 if no
    matching type was found. The returned pointer must not be deleted.

    \sa type(), isRegistered(), Type
*/
const char *QMetaType::typeName(int type)
{
    if (type >= User) {
        if (!isRegistered(type))
            return 0;

        const QVector<QCustomTypeInfo> * const ct = customTypes();
        if (!ct)
            return 0;
        QReadLocker locker(customTypesLock());
        return ct->at(type - User).typeName.constData();
    }
    int i = 0;
    while (types[i].typeName) {
        if (types[i].type == type)
            return types[i].typeName;
        ++i;
    }
    return 0;
}

/*! \internal
    Same as QMetaType::type(), but doesn't lock.
*/
static int qMetaTypeType_unlocked(const char *typeName)
{
    if (!typeName)
        return 0;
    int i = 0;
    while (types[i].typeName && strcmp(typeName, types[i].typeName))
        ++i;
    if (!types[i].type) {
        const QVector<QCustomTypeInfo> * const ct = customTypes();
        if (!ct)
            return 0;

        for (int v = 0; v < ct->count(); ++v) {
            if (strcmp(ct->at(v).typeName, typeName) == 0)
                return v + QMetaType::User;
        }
    }
    return types[i].type;
}

/*! \internal

    Registers a user type for marshalling, with \a typeName, a \a
    destructor, and a \a constructor. Returns the type's handle,
    or -1 if the type could not be registered.
 */
int QMetaType::registerType(const char *typeName, Destructor destructor,
                            Constructor constructor)
{
    QVector<QCustomTypeInfo> *ct = customTypes();
    if (!ct || !typeName || !destructor || !constructor)
        return -1;

#ifdef QT_NO_QOBJECT
    ::QByteArray normalizedTypeName = typeName;
#else
    ::QByteArray normalizedTypeName = QMetaObject::normalizedType(typeName);
#endif

    QWriteLocker locker(customTypesLock());
    static int currentIdx = User;
    int idx = qMetaTypeType_unlocked(normalizedTypeName.constData());

    if (idx) {
        if (idx < User) {
            qWarning("QMetaType: Cannot re-register basic type '%s'",
                    normalizedTypeName.constData());
            return -1;
        }
        QCustomTypeInfo &inf = (*ct)[idx - User];
        inf.constr = constructor;
        inf.destr = destructor;
    } else {
        idx = currentIdx++;
        ct->resize(ct->count() + 1);
        QCustomTypeInfo &inf = (*ct)[idx - User];
        inf.typeName = normalizedTypeName;
        inf.constr = constructor;
        inf.destr = destructor;
    }
    return idx;
}

/*!
    Returns true if the custom datatype with ID \a type is registered;
    otherwise returns false.

    \sa type(), typeName(), Type
*/
bool QMetaType::isRegistered(int type)
{
    QReadLocker locker(customTypesLock());

    const QVector<QCustomTypeInfo> * const ct = customTypes();
    return (type >= User) && (ct && ct->count() > type - User);
}

/*!
    Returns a handle to the type called \a typeName, or 0 if there is
    no such type.

    \sa isRegistered(), typeName(), Type
*/
int QMetaType::type(const char *typeName)
{
#ifdef QT_NO_QOBJECT
    const ::QByteArray normalizedTypeName = typeName;
#else
    const ::QByteArray normalizedTypeName = QMetaObject::normalizedType(typeName);
#endif

    QReadLocker locker(customTypesLock());
    return qMetaTypeType_unlocked(normalizedTypeName.constData());
}

#ifndef QT_NO_DATASTREAM
/*!
    Writes the object pointed to by \a data with the ID \a type to
    the given \a stream. The type must have been registered with
    qRegisterMetaType() and qRegisterMetaTypeStreamOperators()
    beforehand.

    Normally, you should not need to call this function directly.
    Instead, use QVariant's \c operator<<(), which relies on save()
    to stream custom types.

    \sa load(), qRegisterMetaTypeStreamOperators()
*/
bool QMetaType::save(QDataStream &stream, int type, const void *data)
{
    // FIXME - also stream simple types?
    if (!data || !isRegistered(type))
        return false;
    const QVector<QCustomTypeInfo> * const ct = customTypes();
    if (!ct)
        return false;

    SaveOperator saveOp = 0;
    {
        QReadLocker locker(customTypesLock());
        saveOp = ct->at(type - User).saveOp;
    }

    if (!saveOp)
        return false;
    saveOp(stream, data);
    return true;
}

/*!
    Reads the object of the specified \a type from the given \a
    stream into \a data. The type must have been registered with
    qRegisterMetaType() and qRegisterMetaTypeStreamOperators()
    beforehand.

    Normally, you should not need to call this function directly.
    Instead, use QVariant's \c operator>>(), which relies on load()
    to stream custom types.

    \sa save(), qRegisterMetaTypeStreamOperators()
*/
bool QMetaType::load(QDataStream &stream, int type, void *data)
{
    // FIXME - also stream simple types?
    if (!data || !isRegistered(type))
        return false;
    const QVector<QCustomTypeInfo> * const ct = customTypes();
    if (!ct)
        return false;

    LoadOperator loadOp = 0;
    {
        QReadLocker locker(customTypesLock());
        loadOp = ct->at(type - User).loadOp;
    }

    if (!loadOp)
        return false;
    loadOp(stream, data);
    return true;
}
#endif

/*!
    Returns a copy of \a copy, assuming it is of type \a type. If \a
    copy is zero, creates a default type.

    \sa destroy(), isRegistered(), Type
*/
void *QMetaType::construct(int type, const void *copy)
{
    if (copy) {
        switch(type) {
        case QMetaType::VoidStar:
        case QMetaType::QObjectStar:
        case QMetaType::QWidgetStar:
            return new void *(*static_cast<void* const *>(copy));
        case QMetaType::Long:
            return new long(*static_cast<const long*>(copy));
        case QMetaType::Int:
            return new int(*static_cast<const int*>(copy));
        case QMetaType::Short:
            return new short(*static_cast<const short*>(copy));
        case QMetaType::Char:
            return new char(*static_cast<const char*>(copy));
        case QMetaType::ULong:
            return new ulong(*static_cast<const ulong*>(copy));
        case QMetaType::UInt:
            return new uint(*static_cast<const uint*>(copy));
        case QMetaType::UShort:
            return new ushort(*static_cast<const ushort*>(copy));
        case QMetaType::UChar:
            return new uchar(*static_cast<const uchar*>(copy));
        case QMetaType::Bool:
            return new bool(*static_cast<const bool*>(copy));
        case QMetaType::Float:
            return new float(*static_cast<const float*>(copy));
        case QMetaType::Double:
            return new double(*static_cast<const double*>(copy));
        case QMetaType::QChar:
            return new ::QChar(*static_cast<const ::QChar*>(copy));
        case QMetaType::QByteArray:
            return new ::QByteArray(*static_cast<const ::QByteArray*>(copy));
        case QMetaType::QString:
            return new ::QString(*static_cast<const ::QString*>(copy));
        case QMetaType::Void:
            return 0;
        default:
            ;
        }
    } else {
        switch(type) {
        case QMetaType::VoidStar:
        case QMetaType::QObjectStar:
        case QMetaType::QWidgetStar:
            return new void *;
        case QMetaType::Long:
            return new long;
        case QMetaType::Int:
            return new int;
        case QMetaType::Short:
            return new short;
        case QMetaType::Char:
            return new char;
        case QMetaType::ULong:
            return new ulong;
        case QMetaType::UInt:
            return new uint;
        case QMetaType::UShort:
            return new ushort;
        case QMetaType::UChar:
            return new uchar;
        case QMetaType::Bool:
            return new bool;
        case QMetaType::Float:
            return new float;
        case QMetaType::Double:
            return new double;
        case QMetaType::QChar:
            return new ::QChar;
        case QMetaType::QByteArray:
            return new ::QByteArray;
        case QMetaType::QString:
            return new ::QString;
        case QMetaType::Void:
            return 0;
        default:
            ;
        }
    }

    Constructor constr = 0;
    {
        const QVector<QCustomTypeInfo> * const ct = customTypes();
        QReadLocker locker(customTypesLock());
        if (type < User || !ct || ct->count() <= type - User)
            return 0;

        constr = ct->at(type - User).constr;
    } // unlock to prevent reentrancy

    return constr(copy);
}

/*!
    Destroys the \a data, assuming it is of the \a type given.

    \sa construct(), isRegistered(), Type
*/
void QMetaType::destroy(int type, void *data)
{
    if (!data)
        return;
    switch(type) {
    case QMetaType::VoidStar:
    case QMetaType::QObjectStar:
    case QMetaType::QWidgetStar:
        delete static_cast<void**>(data);
        break;
    case QMetaType::Long:
        delete static_cast<long*>(data);
        break;
    case QMetaType::Int:
        delete static_cast<int*>(data);
        break;
    case QMetaType::Short:
        delete static_cast<short*>(data);
        break;
    case QMetaType::Char:
        delete static_cast<char*>(data);
        break;
    case QMetaType::ULong:
        delete static_cast<ulong*>(data);
        break;
    case QMetaType::UInt:
        delete static_cast<uint*>(data);
        break;
    case QMetaType::UShort:
        delete static_cast<ushort*>(data);
        break;
    case QMetaType::UChar:
        delete static_cast<uchar*>(data);
        break;
    case QMetaType::Bool:
        delete static_cast<bool*>(data);
        break;
    case QMetaType::Float:
        delete static_cast<float*>(data);
        break;
    case QMetaType::Double:
        delete static_cast<double*>(data);
        break;
    case QMetaType::QChar:
        delete static_cast< ::QChar*>(data);
        break;
    case QMetaType::QByteArray:
        delete static_cast< ::QByteArray*>(data);
        break;
    case QMetaType::QString:
        delete static_cast< ::QString*>(data);
        break;
    case QMetaType::Void:
        break;
    default:
        {

            const QVector<QCustomTypeInfo> * const ct = customTypes();
            Destructor destr = 0;
            {
                QReadLocker locker(customTypesLock());
                if (type < User || !ct || ct->count() <= type - User)
                    break;
                destr = ct->at(type - User).destr;
            } // unlock to prevent reentrancy
            destr(data);
            break;
        }
    }
}

/*!
    \fn int qRegisterMetaType(const char *typeName)
    \relates QMetaType
    \threadsafe

    Registers the type name \a typeName to the type \c{T}. Returns
    the internal ID used by QMetaType. Any class or struct that has a
    public constructor, a public copy constructor, and a public
    destructor can be registered.

    After a type has been registered, you can create and destroy
    objects of that type dynamically at run-time.

    This example registers the class \c{MyClass}:

    \code
        qRegisterMetaType<MyClass>("MyClass");
    \endcode

    \sa qRegisterMetaTypeStreamOperators(), QMetaType::isRegistered(),
        Q_DECLARE_METATYPE()
*/

/*!
    \fn int qRegisterMetaTypeStreamOperators(const char *typeName)
    \relates QMetaType
    \threadsafe

    Registers the stream operators for the type \c{T} called \a
    typeName.

    Afterward, the type can be streamed using QMetaType::load() and
    QMetaType::save(). These functions are used when streaming a
    QVariant.

    \code
        qRegisterMetaTypeStreamOperator<MyClass>("MyClass");
    \endcode

    The stream operators should have the following signatures:

    \code
        QDataStream &operator<<(QDataStream &out, const MyClass &myObj);
        QDataStream &operator>>(QDataStream &in, MyClass &myObj);
    \endcode

    \sa qRegisterMetaType(), QMetaType::isRegistered(), Q_DECLARE_METATYPE()
*/

/*! \typedef QMetaType::Destructor
    \internal
*/
/*! \typedef QMetaType::Constructor
    \internal
*/
/*! \typedef QMetaType::SaveOperator
    \internal
*/
/*! \typedef QMetaType::LoadOperator
    \internal
*/

/*!
    \fn int qRegisterMetaType()
    \relates QMetaType
    \threadsafe
    \since 4.2

    Call this function to register the type \c T. \c T must be declared with
    Q_DECLARE_METATYPE(). Returns the meta type Id.

    Example:

    \code
        int id = qRegisterMetaType<MyStruct>();
    \endcode

    \bold{Note:} To use the type \c T in QVariant, using Q_DECLARE_METATYPE() is
    sufficient. To use the type \c T in queued signal and slot connections,
    \c{qRegisterMetaType<T>()} must be called before the first connection
    is established.

    \sa Q_DECLARE_METATYPE()
 */

/*! \fn int qMetaTypeId()
    \relates QMetaType
    \threadsafe
    \since 4.1

    Returns the meta type id of type \c T at compile time. If the
    type was not declared with Q_DECLARE_METATYPE(), compilation will
    fail.

    Typical usage:

    \code
        int id = qMetaTypeId<QString>();    // id is now QMetaType::QString
        id = qMetaTypeId<MyStruct>();       // compile error if MyStruct not declared
    \endcode

    QMetaType::type() returns the same ID as qMetaTypeId(), but does
    a lookup at runtime based on the name of the type.
    QMetaType::type() is a bit slower, but compilation succeeds if a
    type is not registered.

    \sa Q_DECLARE_METATYPE(), QMetaType::type()
*/
