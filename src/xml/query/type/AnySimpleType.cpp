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

#include "BuiltinTypes.h"

#include "AnySimpleType.h"

using namespace Patternist;

AnySimpleType::AnySimpleType()
{
}

AnySimpleType::~AnySimpleType()
{
}

QName AnySimpleType::name(const NamePool::Ptr &np) const
{
    return np->allocateQName(StandardNamespaces::xs, QLatin1String("anySimpleType"));
}

QString AnySimpleType::displayName(const NamePool::Ptr &np) const
{
    return np->displayName(name(np));
}

SchemaType::Ptr AnySimpleType::wxsSuperType() const
{
    return BuiltinTypes::xsAnyType;
}

SchemaType::TypeCategory AnySimpleType::category() const
{
    return None;
}

SchemaType::DerivationMethod AnySimpleType::derivationMethod() const
{
    return DerivationRestriction;
}

// vim: et:ts=4:sw=4:sts=4
