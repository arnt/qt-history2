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

#include "ListIterator.h"
#include "Item.h"

#include "UnionIterator.h"

using namespace Patternist;

UnionIterator::UnionIterator(const Item::Iterator::Ptr &it1,
                             const Item::Iterator::Ptr &it2) : m_it1(it1),
                                                               m_it2(it2),
                                                               m_position(0),
                                                               m_node1(m_it1->next()),
                                                               m_node2(m_it2->next())
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_it1);
    Q_ASSERT(m_it2);
}

Item UnionIterator::next()
{
    qDebug() << Q_FUNC_INFO;

    ++m_position;
    if(m_node1 && m_node2)
    {
        switch(m_node1.asNode().compareOrderTo(m_node2.asNode()))
        {
            case Node::Precedes:
            {
                m_current = m_node1;
                m_node1 = m_it1->next();
                return m_current;
            }
            case Node::Follows:
            {
                m_current = m_node2;
                m_node2 = m_it2->next();
                return m_current;
            }
            default:
            {
                m_current = m_node2;
                m_node1 = m_it1->next();
                m_node2 = m_it2->next();
                return m_current;
            }
        }
    }

    if(m_node1)
    {
        m_current = m_node1;
        m_node1 = m_it1->next();
        return m_current;
    }

    if(m_node2)
    {
        m_current = m_node2;
        m_node2 = m_it2->next();
        return m_current;
    }

    m_current.reset();
    m_position = -1;
    return Item();
}

Item UnionIterator::current() const
{
    return m_current;
}

xsInteger UnionIterator::position() const
{
    return m_position;
}

Item::Iterator::Ptr UnionIterator::copy() const
{
    return Item::Iterator::Ptr(new UnionIterator(m_it1->copy(), m_it2->copy()));
}

// vim: et:ts=4:sw=4:sts=4
