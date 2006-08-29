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
    \since 4.2

    \brief The QDBusVariant class is provided to be able to separate
    D-BUS variants from objects of the QVariant type.

    A D-BUS variant type can contain any type, including other
    variants.

    \sa QVariant
*/

/*!
    \fn QDBusVariant::QDBusVariant()

    Constructs a new variant.
*/

/*!
    \fn QDBusVariant::QDBusVariant(const QVariant &variant)
    \overload

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
