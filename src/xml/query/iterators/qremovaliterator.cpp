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

#include "qlistiterator_p.h"

#include "qremovaliterator_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

RemovalIterator::RemovalIterator(const Item::Iterator::Ptr &target,
                                 const xsInteger pos) : m_target(target),
                                                        m_removalPos(pos),
                                                        m_position(0)
{
    Q_ASSERT(target);
    Q_ASSERT(pos >= 1);
}

Item RemovalIterator::next()
{
    if(m_position == -1)
        return Item();

    m_current = m_target->next();

    if(!m_current)
    {
        m_position = -1;
        m_current.reset();
        return Item();
    }

    ++m_position;

    if(m_position == m_removalPos)
    {
        next(); /* Recurse, return the next item. */
        --m_position; /* Don't count the one we removed. */
        return m_current;
    }

    return m_current;
}

xsInteger RemovalIterator::count()
{
    const xsInteger itc = m_target->count();

    if(itc < m_removalPos)
        return itc;
    else
        return itc - 1;
}

Cardinality RemovalIterator::cardinality()
{
    return Cardinality::fromCount(count());
}

Item RemovalIterator::current() const
{
    return m_current;
}

xsInteger RemovalIterator::position() const
{
    return m_position;
}

Item::Iterator::Ptr RemovalIterator::copy() const
{
    return Item::Iterator::Ptr(new RemovalIterator(m_target->copy(), m_removalPos));
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
