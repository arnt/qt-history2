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

#include "qdbusargument.h"

#include <qatomic.h>
#include <qbytearray.h>
#include <qlist.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>

#include "qdbusargument_p.h"
#include "qdbusmetatype_p.h"

QByteArray QDBusArgumentPrivate::createSignature(int id)
{
    QByteArray signature;
    QDBusMarshaller *marshaller = new QDBusMarshaller;
    marshaller->ba = &signature;

    // run it
    void *null = 0;
    QVariant v(id, null);
    QDBusArgument arg;
    arg.d = marshaller;
    QDBusMetaType::marshall(arg, v.userType(), v.constData());
    arg.d = 0;

    // delete it
    bool ok = marshaller->ok;
    delete marshaller;

    if (signature.isEmpty() || !ok || !dbus_signature_validate_single(signature, 0)) {
        qWarning("QDBusMarshaller: type `%s' produces invalid D-BUS signature `%s' "
                 "(Did you forget to call newStructure() ?)",
                 QVariant::typeToName( QVariant::Type(id) ),
                 signature.isEmpty() ? "<empty>" : signature.constData());
        return "";
    } else if (signature.at(0) != DBUS_TYPE_ARRAY && signature.at(0) != DBUS_STRUCT_BEGIN_CHAR ||
               (signature.at(0) == DBUS_TYPE_ARRAY && (signature.at(1) == DBUS_TYPE_BYTE ||
                                                       signature.at(1) == DBUS_TYPE_STRING))) {
        qWarning("QDBusMarshaller: type `%s' attempts to redefine basic D-BUS type '%s' (%s) "
                 "(Did you forget to call newStructure() ?)",
                 QVariant::typeToName( QVariant::Type(id) ),
                 signature.constData(),
                 QVariant::typeToName( QVariant::Type(QDBusMetaType::signatureToType(signature))) );
        return "";
    }
    return signature;
}

bool QDBusArgumentPrivate::checkWrite()
{
    if (direction == Marshalling)
        return marshaller()->ok;

#ifdef QT_DEBUG
    qFatal("QDBusArgument: write from a read-only object");
#else
    qWarning("QDBusArgument: write from a read-only object");
#endif
    return false;
}

bool QDBusArgumentPrivate::checkRead()
{
    if (direction == Demarshalling)
        return true;

#ifdef QT_DEBUG
    qFatal("QDBusArgument: read from a write-only object");
#else
    qWarning("QDBusArgument: read from a write-only object");
#endif

    return false;
}

/*!
    \class QDBusArgument
    \inmodule QtDBus
    \brief The QDBusArgument class is used to marshall and demarshall D-BUS arguments.

    The class is used to send arguments over D-BUS to remote
    applications and to receive them back. D-BUS offers an extensible
    type system, based on a few primitive types and associations of
    them. See the \l {qdbustypesystem.html}{QtDBus type system} page
    for more information on the type system.

    QDBusArgument is the central class in the QtDBus type system,
    providing functions to marshall and demarshall the primitive
    types. The compound types are then created by association of one
    or more of the primitive types in arrays, dictionaries or
    structures.

    The following example illustrates how a structure containing an
    integer and a string can be constructed using the \l
    {qdbustypesystem.html}{QtDBus type system}:

    \code
        struct MyStructure
        {
            int count;
            QString name;
        };
        Q_DECLARE_METATYPE(MyStructure)

        // Marshall the MyStructure data into a D-BUS argument
        QDBusArgument &operator<<(QDBusArgument &argument, const MyStructure &mystruct)
        {
            argument.newStructure() << mystruct.count << mystruct.name;
            return argument;
        }

        // Retrieve the MyStructure data from the D-BUS argument
        const QDBusArgument &operator>>(const QDBusArgument &argument, MyStructure &mystruct)
        {
            argument.structure() >> mystruct.count >> mystruct.name;
            return argument;
        }
    \endcode

    The type has to be registered with qDBusRegisterMetaType() before
    it can be used with QDBusArgument. Therefore, somewhere in your
    program, you should add the following code:

    \code
        qDBusRegisterMetaType<MyStructure>("MyStructure");
    \endcode

    Once registered, a type can be used in outgoing method calls
    (placed with QDBusAbstractInterface::call()), signal emissions
    from registered objects or in incoming calls from remote
    applications.

    It is important to note that the \c{operator<<} and \c{operator>>}
    streaming functions must always produce the same number of entries
    in case of structures, both in reading and in writing (marshalling
    and demarshalling), otherwise calls and signals may start to
    silently fail.

    The following example illustrates this wrong usage
    in context of a class that may contain invalid data:

    \badcode
        struct MyTime
        {
            bool isValid;
            int hour, minute, second;
        };
        Q_DECLARE_METATYPE(MyTime)

        // Marshall the MyTime data into a D-BUS argument
        QDBusArgument &operator<<(QDBusArgument &argument, const MyTime &mytime)
        {
            if (mytime.isValid)
                argument.newStructure() << true << mytime.hour
                                        << mytime.minute << mytime.second;
            else
                argument.newStructure() << false;
            return argument;
        }

        // Retrieve the MyTime data from the D-BUS argument
        const QDBusArgument &operator>>(const QDBusArgument &argument, MyTime &mytime)
        {
            QDBusArgument s = argument.structure();
            s >> mytime.isValid;
            if (mytime.isValid)
                s >> s.hour >> s.minute >> s.second;
            return argument;
        }
    \endcode

    In this example, both the \c{operator<<} and the \c{operator>>}
    functions may produce a different number of reads/writes. This can
    confuse the QtDBus type system and should be avoided.

    \sa QDBusAbstractInterface, {qdbustypesystem.html}{The QtDBus type system}
    {usingadaptors.html}{Using Adaptors}, qdbus_cast(const QDBusArgument&)
*/

/*!
    \fn qdbus_cast(const QDBusArgument &argument)

    Attempts to demarshall the contents of the QDBusArgument object
    into the type \c{T}.

    This function is used like the following:
    \code
        MyType item = qdbus_cast<Type>(argument);
    \endcode

    It's also valid to note that it is equivalent to the following:
    \code
        MyType item;
        argument >> item;
    \endcode
*/

/*!
    Constructs an empty QDBusArgument argument.

    An empty QDBusArgument object does not allow either reading or
    writing to be performed.
*/
QDBusArgument::QDBusArgument()
    : d(0)
{
}

/*!
    Constructs a copy of the \a other QDBusArgument object.

    Both objects will therefore contain the same state from this point
    forward. QDBusArguments are explicitly shared and, therefore, any
    modification to either copy will affect the other one too.
*/
QDBusArgument::QDBusArgument(const QDBusArgument &other)
    : d(other.d)
{
    if (d)
        d->ref.ref();
}

/*!
    Copies the \a other QDBusArgument object into this one.

    Both objects will therefore contain the same state from this point
    forward. QDBusArguments are explicitly shared and, therefore, any
    modification to either copy will affect the other one too.
*/
QDBusArgument &QDBusArgument::operator=(const QDBusArgument &other)
{
    if (other.d)
        other.d->ref.ref();
    QDBusArgumentPrivate *old = qAtomicSetPtr(&d, other.d);
    if (old && !old->ref.deref())
        delete old;
    return *this;
}

/*!
    Disposes of the resources associated with this QDBusArgument
    object.
*/
QDBusArgument::~QDBusArgument()
{
    if (d && !d->ref.deref())
        delete d;
}

/*!
    Appends the primitive value \a arg of type \c{BYTE} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(uchar arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{BOOLEAN} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(bool arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{INT16} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(short arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{UINT16} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(ushort arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{INT32} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(int arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{UINT32} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(uint arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{INT64} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(qlonglong arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{UINT64} to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(qulonglong arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{DOUBLE} (double-precision
    floating-point) to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(double arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{STRING} (Unicode character
    string) to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(const QString &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    \internal
    Appends the primitive value \a arg of type \c{OBJECT_PATH} (path to a D-BUS
    object) to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(const QDBusObjectPath &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    \internal
    Appends the primitive value \a arg of type \c{SIGNATURE} (D-BUS type
    signature) to the D-BUS stream.
*/
QDBusArgument &QDBusArgument::operator<<(const QDBusSignature &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the primitive value \a arg of type \c{VARIANT} to the D-BUS stream.

    A D-BUS variant type can contain any type, including other
    variants. It is similar to the Qt QVariant type.
*/
QDBusArgument &QDBusArgument::operator<<(const QDBusVariant &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the QStringList given by \a arg as \c{ARRAY of STRING}
    to the D-BUS stream.

    QStringList and QByteArray are the only two non-primitive types
    that are supported directly by QDBusArgument because of their
    widespread usage in Qt applications.

    Other arrays are supported through compound types in QtDBus.
*/
QDBusArgument &QDBusArgument::operator<<(const QStringList &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \overload
    Appends the QByteArray given by \a arg as \c{ARRAY of BYTE}
    to the D-BUS stream.

    QStringList and QByteArray are the only two non-primitive types
    that are supported directly by QDBusArgument because of their
    widespread usage in Qt applications.

    Other arrays are supported through compound types in QtDBus.
*/
QDBusArgument &QDBusArgument::operator<<(const QByteArray &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

/*!
    \internal
    Returns the type signature of the D-BUS type this QDBusArgument
    object is currently pointing to.
*/    
QString QDBusArgument::currentSignature() const
{
    if (d && d->checkRead())
        return d->demarshaller()->currentSignature();

    return QString();
}

/*!
    Extracts one D-BUS primitive argument of type \c{BYTE} from the
    D-BUS stream.
*/
uchar QDBusArgument::toByte() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toByte();

    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{BOOLEAN} from the
    D-BUS stream.
*/
bool QDBusArgument::toBool() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toBool();
    return false;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{UINT16} from the
    D-BUS stream.
*/
ushort QDBusArgument::toUShort() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toUShort();

    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{INT16} from the
    D-BUS stream.
*/
short QDBusArgument::toShort() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toShort();

    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{INT32} from the
    D-BUS stream.
*/
int QDBusArgument::toInt() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toInt();

    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{UINT32} from the
    D-BUS stream.
*/
uint QDBusArgument::toUInt() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toUInt();


    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{INT64} from the
    D-BUS stream.
*/
qlonglong QDBusArgument::toLongLong() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toLongLong();


    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{UINT64} from the
    D-BUS stream.
*/
qulonglong QDBusArgument::toULongLong() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toULongLong();


    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{DOUBLE}
    (double-precision floating pount) from the D-BUS stream.
*/
double QDBusArgument::toDouble() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toDouble();


    return 0;
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{STRING} (Unicode
    character string) from the D-BUS stream.
*/
QString QDBusArgument::toString() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toString();


    return QString();
}

/*!
    \overload
    \internal
    Extracts one D-BUS primitive argument of type \c{OBJECT_PATH}
    (D-BUS path to an object) from the D-BUS stream.
*/
QDBusObjectPath QDBusArgument::toObjectPath() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toObjectPath();

    return QDBusObjectPath();
}

/*!
    \overload
    \internal
    Extracts one D-BUS primitive argument of type \c{SIGNATURE} (D-BUS
    type signature) from the D-BUS stream.
*/
QDBusSignature QDBusArgument::toSignature() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toSignature();

    return QDBusSignature();
}

/*!
    \overload
    Extracts one D-BUS primitive argument of type \c{VARIANT} from the
    D-BUS stream.

    A D-BUS variant type can contain any type, including other
    variants. It is similar to the Qt QVariant type.

    In case the variant contains a type not directly supported by
    QDBusArgument, the value of the returned QDBusVariant will contain
    another QDBusArgument. It is your responsibility to further
    demarshall it into another type.
*/
QDBusVariant QDBusArgument::toVariant() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toVariant();

    return QDBusVariant();
}

/*!
    \overload
    Extracts an array of strings from the D-BUS stream and return it
    as a QStringList.

    QStringList and QByteArray are the only two non-primitive types
    that are supported directly by QDBusArgument because of their
    widespread usage in Qt applications.

    Other arrays are supported through compound types in QtDBus.
*/    
QStringList QDBusArgument::toStringList() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toStringList();

    return QStringList();
}

/*!
    \overload
    Extracts an array of bytes from the D-BUS stream and return it
    as a QByteArray.

    QStringList and QByteArray are the only two non-primitive types
    that are supported directly by QDBusArgument because of their
    widespread usage in Qt applications.

    Other arrays are supported through compound types in QtDBus.
*/    
QByteArray QDBusArgument::toByteArray() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toByteArray();

    return QByteArray();
}

/*!
    Creates a D-BUS structure and return a QDBusArgument suitable for
    appending new arguments.

    This function is used usually in \c{operator<<} streaming
    operators, as in the following example:

    \code
        QDBusArgument &operator<<(QDBusArgument &argument, const MyStructure &mystruct)
        {
            argument.newStructure() << mystruct.member1 << mystruct.member2 << ... ;
            return argument;
        }
    \endcode

    Structures can contain other structures, so the following code is
    also valid:

    \code
        QDBusArgument &operator<<(QDBusArgument &argument, const MyStructure &mystruct)
        {
            QDBusArgument s = argument.newStructure();
            s << mystruct.member1 << mystruct.member2;
            s.newStructure() << mystruct.member3.subMember1 << mystruct.member3.subMember2;
            s << mystruct.member4;
            return argument;
        }
    \endcode

    \b Caveat: The QDBusArgument object returned by this function must
    be disposed of before you can continue to append elements to this
    object.

    \sa structure(), newArray(), newMap()
*/
QDBusArgument QDBusArgument::newStructure()
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseStructure();

    return QDBusArgument();
}

/*!
    Creates a D-BUS array and return a QDBusArgument suitable for
    appending elements of meta-type \a id.

    This function is used usually in \c{operator<<} streaming
    operators, as in the following example:

    \code
        // append an array of MyElement types
        QDBusArgument &operator<<(QDBusArgument &argument, const MyArray &myarray)
        {
            QDBusArgument argArray = argument.newArray( qMetaTypeId<MyElement>() );
            for ( int i = 0; i < myarray.length; ++i )
                argArray << myarray.elements[i];
            return argument;
        }
    \endcode

    If the type you want to marshall is a QList, QVector or any of the
    Qt's {containers.html}{containers} that take one template
    parameter, you need not declare an \c{operator<<} function for
    it, since QtDBus provides generic templates to do the job of
    marshalling the data. The same applies for STL's sequence
    containers, such as \c {std::list}, \c {std::vector}, etc.

    \b Caveat: The QDBusArgument object returned by this function must
    be disposed of before you can continue to append elements to this
    object.

    \sa array(), newStructure(), newMap()
*/
QDBusArgument QDBusArgument::newArray(int id)
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseArray(id);

    return QDBusArgument();
}

/*!
    Creates a D-BUS map and return a QDBusArgument suitable for
    appending elements. Maps are containers that associate one entry
    (the key) to another (the value), such as Qt's QMap or QHash. The
    ids of the map's key and value meta types must be passed in \a kid
    and \a vid respectively.

    This function is used usually in \c{operator<<} streaming
    operators, as in the following example:

    \code
        // append a dictionary that associates ints to MyValue types
        QDBusArgument &operator<<(QDBusArgument &argument, const MyDictionary &mydict)
        {
            QDBusArgument argMap = argument.newMap( QVariant::Int, qMetaTypeId<MyValue>() );
            for ( int i = 0; i < mydict.length; ++i )
                argMap.mapEntry() << mydict.data[i].key << mydict.data[i].value;
            return argument;
        }
    \endcode

    If the type you want to marshall is a QMap or QHash, you need not
    declare an \c{operator<<} function for it, since QtDBus provides
    generic templates to do the job of marshalling the data.

    \b Caveat: The QDBusArgument object returned by this function must
    be disposed of before you can continue to append elements to this
    object.

    \sa map(), newStructure(), newArray(), newMapEntry()
*/
QDBusArgument QDBusArgument::newMap(int kid, int vid)
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseMap(kid, vid);

    return QDBusArgument();
}

/*!
    Creates a D-BUS map entry and return a QDBusArgument suitable for
    appending the key and value entries. This function is only valid
    in QDBusArgument objects created with newMap().

    See newMap() for an example of usage of this function.

    \sa mapEntry(), newMap()
*/
QDBusArgument QDBusArgument::newMapEntry()
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseMapEntry();

    return QDBusArgument();
}    

/*!
    Returns a QDBusArgument object suitable for extracting the
    elements of a D-BUS structure created with newStructure().

    This function is used usually in \c{operator>>} streaming
    operators, as in the following example:

    \code
        const QDBusArgument &operator>>(const QDBusArgument &argument, MyStructure &mystruct)
        {
            argument.structure() >> mystruct.member1 >> mystruct.member2 >> mystruct.member3 >> ...;
            return argument;
        }
    \endcode

    \sa newStructure(), array(), map()
*/
QDBusArgument QDBusArgument::structure() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseStructure();

    return QDBusArgument();
}

/*!
    Returns a QDBusArgument object suitable for extracting the
    elements of a D-BUS array created with newArray().

    This function is used usually in \c{operator>>} streaming
    operators, as in the following example:

    \code
        // extract a MyArray array of MyElement elements
        const QDBusArgument &operator>>(const QDBusArgument &argument, MyArray &myarray)
        {
            QDBusArgument argArray = argument.array();
            myarray.clear();

            while ( !argArray.atEnd() ) {
                MyElement element;
                argArray >> element;
                myarray.append( element );
            }

            return argument;
        }
    \endcode

    If the type you want to demarshall is a QList, QVector or any of the
    Qt's {containers.html}{containers} that take one template
    parameter, you need not declare an \c{operator>>} function for
    it, since QtDBus provides generic templates to do the job of
    demarshalling the data. The same applies for STL's sequence
    containers, such as \c {std::list}, \c {std::vector}, etc.

    \sa newArray(), atEnd(), structure(), map()
*/
QDBusArgument QDBusArgument::array() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseArray();

    return QDBusArgument();
}

/*!
    Returns a QDBusArgument object suitable for extracting the
    elements of a D-BUS dictionary created with newMap().

    This function is used usually in \c{operator>>} streaming
    operators, as in the following example:

    \code
        // extract a MyDictionary map that associates ints to MyValue elements
        const QDBusArgument &operator>>(const QDBusArgument &argument, MyDictionary &mydict)
        {
            QDBusArgument argMap = argument.map();
            mydict.clear();

            while ( !argMap.atEnd() ) {
                int key;
                MyValue value;
                argMap.mapEntry() >> key >> value;
                mydict.append( key, value );
            }

            return argument;
        }
    \endcode

    If the type you want to demarshall is a QMap or QHash, you need not
    declare an \c{operator>>} function for it, since QtDBus provides
    generic templates to do the job of demarshalling the data.

    \sa newMap(), atEnd(), mapEntry(), structure(), array()
*/
QDBusArgument QDBusArgument::map() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseMap();

    return QDBusArgument();
}

/*!
    Returns a QDBusArgument object suitable for extracting the key and
    value pair that comprises one entry of a dictionary.

    See map() for an example of how this function is usually used.

    \sa newMapEntry(), map()
*/
QDBusArgument QDBusArgument::mapEntry() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseMapEntry();

    return QDBusArgument();
}

/*!
    Returns true if there are no more elements to be extracted from
    this QDBusArgument. This function is usually used in QDBusArgument
    objects returned from map() and array().
*/
bool QDBusArgument::atEnd() const
{
    if (d && d->checkRead())
        return d->demarshaller()->atEnd();

    return true;                // at least, stop reading
}

// for optimization purposes, we include the marshallers here
#include "qdbusmarshaller.cpp"
#include "qdbusdemarshaller.cpp"
