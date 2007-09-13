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

#include "qitem_p.h"
#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qschematypefactory_p.h"
#include "qqname_p.h"

#include "qatomictype_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

AtomicType::AtomicType()
{
}

AtomicType::~AtomicType()
{
}

bool AtomicType::xdtTypeMatches(const ItemType::Ptr &other) const
{
    if(other->isAtomicType())
    {
        if(*other == *this)
            return true;
        else
            return xdtTypeMatches(other->xdtSuperType());
    }
    else
        return false;
}

bool AtomicType::itemMatches(const Item &item) const
{
    Q_ASSERT(item);
    if(item.isNode())
        return false;
    else
    {
        const SchemaType::Ptr t(static_cast<AtomicType *>(item.type().get()));
        return wxsTypeMatches(t);
    }
}

ItemType::Ptr AtomicType::atomizedType() const
{
    return AtomicType::Ptr(const_cast<AtomicType *>(this));
}

QString AtomicType::displayName(const NamePool::Ptr &) const
{
    /* A bit faster than calling name()->displayName() */
    return QLatin1String("xs:anyAtomicType");
}

bool AtomicType::isNodeType() const
{
    return false;
}

bool AtomicType::isAtomicType() const
{
    return true;
}

SchemaType::TypeCategory AtomicType::category() const
{
    return SimpleTypeAtomic;
}

SchemaType::DerivationMethod AtomicType::derivationMethod() const
{
    return DerivationRestriction;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
