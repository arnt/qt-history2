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

#include "qatomictype_p.h"
#include "qitem_p.h"
#include "qbuiltintypes_p.h"
#include "qdebug_p.h"
#include "qschematype_p.h"

#include "qbuiltinatomictype_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

BuiltinAtomicType::BuiltinAtomicType(const AtomicType::Ptr &base,
                                     const AtomicComparatorLocator::Ptr &comp,
                                     const AtomicMathematicianLocator::Ptr &mather,
                                     const AtomicCasterLocator::Ptr &casterlocator)
                                     : m_superType(base),
                                       m_comparatorLocator(comp),
                                       m_mathematicianLocator(mather),
                                       m_casterLocator(casterlocator)
{
}

SchemaType::Ptr BuiltinAtomicType::wxsSuperType() const
{
    return m_superType;
}

ItemType::Ptr BuiltinAtomicType::xdtSuperType() const
{
    return m_superType;
}

bool BuiltinAtomicType::isAbstract() const
{
    return false;
}

AtomicComparatorLocator::Ptr BuiltinAtomicType::comparatorLocator() const
{
    return m_comparatorLocator;
}

AtomicMathematicianLocator::Ptr BuiltinAtomicType::mathematicianLocator() const
{
    return m_mathematicianLocator;
}

AtomicCasterLocator::Ptr BuiltinAtomicType::casterLocator() const
{
    return m_casterLocator;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
