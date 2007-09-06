/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include "Item.h"
#include "ValidationError.h"

#include "AnyURI.h"

using namespace Patternist;

AnyURI::AnyURI(const QString &s) : AtomicString(s)
{
}

AnyURI::Ptr AnyURI::fromValue(const QString &value)
{
    return AnyURI::Ptr(new AnyURI(value));
}

AnyURI::Ptr AnyURI::fromValue(const QUrl &uri)
{
    return AnyURI::Ptr(new AnyURI(uri.toString()));
}

AnyURI::Ptr AnyURI::resolveURI(const QString &relative,
                               const QString &base)
{
    const QUrl urlBase(base);
    return AnyURI::fromValue(urlBase.resolved(relative).toString());
}

ItemType::Ptr AnyURI::type() const
{
    return BuiltinTypes::xsAnyURI;
}

Item AnyURI::fromLexical(const QString &value)
{
    const QUrl uri(QUrl(value.simplified()));

    if(uri.isValid())
        return Item(new AnyURI(uri.toString()));
    else
        return ValidationError::createError();
}

// vim: et:ts=4:sw=4:sts=4
