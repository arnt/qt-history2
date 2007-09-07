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

#include "qbuiltintypes_p.h"

#include "qnonetype_p.h"

using namespace Patternist;

NoneType::NoneType()
{
}

bool NoneType::itemMatches(const Item &) const
{
    return false;
}

bool NoneType::xdtTypeMatches(const ItemType::Ptr &t) const
{
    return *this == *t;
}

const ItemType &NoneType::operator|(const ItemType &other) const
{
    return other;
}

QString NoneType::displayName(const NamePool::Ptr &) const
{
    return QLatin1String("none");
}

Cardinality NoneType::cardinality() const
{
    return Cardinality::zeroOrMore();
}

ItemType::Ptr NoneType::itemType() const
{
    return ItemType::Ptr(const_cast<NoneType *>(this));
}

bool NoneType::isAtomicType() const
{
    return false;
}

bool NoneType::isNodeType() const
{
    return false;
}

ItemType::Ptr NoneType::atomizedType() const
{
    return BuiltinTypes::xsAnyAtomicType;
}

ItemType::Ptr NoneType::xdtSuperType() const
{
    return BuiltinTypes::item;
}

// vim: et:ts=4:sw=4:sts=4
