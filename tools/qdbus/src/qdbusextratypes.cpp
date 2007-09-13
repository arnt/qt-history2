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

#include "qdbusextratypes.h"
#include "qdbusutil_p.h"

QT_BEGIN_NAMESPACE

void QDBusObjectPath::check()
{
    if (!QDBusUtil::isValidObjectPath(*this))
        clear();
}

void QDBusSignature::check()
{
    if (!QDBusUtil::isValidSignature(*this))
        clear();
}

/*!
    \class QDBusVariant
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusVariant class enables the programmer to identify
    the variant type provided by the D-BUS typesystem.

    \sa {The QtDBus type system}
*/

/*!
    \fn QDBusVariant::QDBusVariant()

    Constructs a new variant.
*/

/*!
    \fn QDBusVariant::QDBusVariant(const QVariant &variant)

    Constructs a new variant from the given \a variant.

    \sa setVariant()
*/

/*!
    \fn QVariant QDBusVariant::variant() const

    Returns this variant as a QVariant object.

    \sa setVariant()
*/

/*!
    \fn void QDBusVariant::setVariant(const QVariant &variant)

    Assigns the value of the given \a variant to this variant.

    \sa variant()
*/

/*!
    \class QDBusObjectPath
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusObjectPath class enables the programmer to
    identify the \c OBJECT_PATH type provided by the D-BUS typesystem.

    \sa {The QtDBus type system}
*/

/*!
    \fn QDBusObjectPath::QDBusObjectPath()

    Constructs a new object path.
*/

/*!
    \fn QDBusObjectPath::QDBusObjectPath(const char *path)

    Constructs a new object path from the given \a path.

    \sa setPath()
*/

/*!
    \fn QDBusObjectPath::QDBusObjectPath(const QLatin1String &path)

    Constructs a new object path from the given \a path.
*/

/*!
    \fn QDBusObjectPath::QDBusObjectPath(const QString &path)

    Constructs a new object path from the given \a path.
*/

/*!
    \fn QString QDBusObjectPath::path() const

    Returns this object path.

    \sa setPath()
*/

/*!
    \fn void QDBusObjectPath::setPath(const QString &path)

    Assigns the value of the given \a path to this object path.

    \sa path()
*/


/*!
    \class QDBusSignature
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusSignature class enables the programmer to
    identify the \c SIGNATURE type provided by the D-BUS typesystem.

    \sa {The QtDBus type system}
*/

/*!
    \fn QDBusSignature::QDBusSignature()

    Constructs a new signature.

    \sa setSignature()
*/

/*!
    \fn QDBusSignature::QDBusSignature(const char *signature)

    Constructs a new signature from the given \a signature.
*/

/*!
    \fn QDBusSignature::QDBusSignature(const QLatin1String &signature)

    Constructs a new signature from the given \a signature.
*/

/*!
    \fn QDBusSignature::QDBusSignature(const QString &signature)

    Constructs a new signature from the given \a signature.
*/

/*!
    \fn QString QDBusSignature::signature() const

    Returns this signature.

    \sa setSignature()
*/

/*!
    \fn void QDBusSignature::setSignature(const QString &signature)

    Assigns the value of the given \a signature to this signature.


QT_END_NAMESPACE
    \sa signature()
*/
