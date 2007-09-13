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
#include "qcommonsequencetypes_p.h"
#include "qdebug_p.h"

#include "qemptysequencetype_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

EmptySequenceType::EmptySequenceType()
{
}

bool EmptySequenceType::xdtTypeMatches(const ItemType::Ptr &other) const
{
    return *other == *this ||
           CommonSequenceTypes::None->xdtTypeMatches(other);
}

bool EmptySequenceType::itemMatches(const Item &) const
{
    return false;
}

QString EmptySequenceType::displayName(const NamePool::Ptr &) const
{
    return QLatin1String("empty-sequence()");
}

ItemType::Ptr EmptySequenceType::xdtSuperType() const
{
    return BuiltinTypes::item;
}

Cardinality EmptySequenceType::cardinality() const
{
    return Cardinality::empty();
}

ItemType::Ptr EmptySequenceType::itemType() const
{
    return ItemType::Ptr(const_cast<EmptySequenceType *>(this));
}

bool EmptySequenceType::isNodeType() const
{
    return false;
}

bool EmptySequenceType::isAtomicType() const
{
    return false;
}

ItemType::Ptr EmptySequenceType::atomizedType() const
{
    return BuiltinTypes::xsAnyAtomicType;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
