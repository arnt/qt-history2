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

#include "qmultiitemtype_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

MultiItemType::MultiItemType(const ItemType::List &list) : m_types(list),
                                                           m_end(list.constEnd())
{
    Q_ASSERT_X(list.count() >= 2, Q_FUNC_INFO,
               "It makes no sense to use MultiItemType for types less than two.");
    Q_ASSERT_X(list.count(ItemType::Ptr()) == 0, Q_FUNC_INFO,
               "No member in the list can be null.");
}

QString MultiItemType::displayName(const NamePool::Ptr &np) const
{
    QString result;
    ItemType::List::const_iterator it(m_types.constBegin());

    while(true)
    {
        result += (*it)->displayName(np);
        ++it;

        if(it != m_end)
            result += QLatin1String(" | ");
        else
            break;
    }

    return result;
}

bool MultiItemType::itemMatches(const Item &item) const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->itemMatches(item))
            return true;

    return false;
}

bool MultiItemType::xdtTypeMatches(const ItemType::Ptr &type) const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->xdtTypeMatches(type))
            return true;

    return false;
}

bool MultiItemType::isNodeType() const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->isNodeType())
            return true;

    return false;
}

bool MultiItemType::isAtomicType() const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->isAtomicType())
            return true;

    return false;
}

ItemType::Ptr MultiItemType::xdtSuperType() const
{
    ItemType::List::const_iterator it(m_types.constBegin());
    /* Load the first one, and jump over it in the loop. */
    ItemType::Ptr result((*it)->xdtSuperType());
    ++it;

    for(; it != m_end; ++it)
        result |= (*it)->xdtSuperType();

    return result;
}

ItemType::Ptr MultiItemType::atomizedType() const
{
    ItemType::List::const_iterator it(m_types.constBegin());
    /* Load the first one, and jump over it in the loop. */
    ItemType::Ptr result((*it)->atomizedType());
    ++it;

    for(; it != m_end; ++it)
        result |= (*it)->atomizedType();

    return result;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
