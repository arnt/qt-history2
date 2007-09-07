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
****************************************************************************/

#include "BuiltinTypes.h"

#include "Untyped.h"

using namespace Patternist;

Untyped::Untyped()
{
}

SchemaType::Ptr Untyped::wxsSuperType() const
{
    return BuiltinTypes::xsAnyType;
}

QName Untyped::name(const NamePool::Ptr &np) const
{
    return np->allocateQName(StandardNamespaces::xs, QLatin1String("untyped"));
}

ItemType::Ptr Untyped::atomizedType() const
{
    return BuiltinTypes::xsUntypedAtomic;
}

SchemaType::TypeCategory Untyped::category() const
{
    return SchemaType::ComplexType;
}

SchemaType::DerivationMethod Untyped::derivationMethod() const
{
    return NoDerivation;
}

// vim: et:ts=4:sw=4:sts=4
