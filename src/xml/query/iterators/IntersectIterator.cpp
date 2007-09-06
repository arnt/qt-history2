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

#include "ListIterator.h"
#include "Item.h"

#include "IntersectIterator.h"

using namespace Patternist;

IntersectIterator::IntersectIterator(const Item::Iterator::Ptr &it1,
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

Item IntersectIterator::next()
{
    qDebug() << Q_FUNC_INFO;
    if(!m_node1 || !m_node2)
        return closedExit();

    do
    {
        switch(m_node1.asNode().compareOrderTo(m_node2.asNode()))
        {
            case Node::Precedes:
            {
                m_node1 = m_it1->next();
                break;
            }
            case Node::Follows:
            {
                m_node2 = m_it2->next();
                break;
            }
            default:
            {
                m_current = m_node2;
                m_node1 = m_it1->next();
                m_node2 = m_it2->next();
                ++m_position;
                return m_current;
            }
        }
    }
    while(m_node1 && m_node2);

    return Item();
}

Item IntersectIterator::current() const
{
    return m_current;
}

xsInteger IntersectIterator::position() const
{
    return m_position;
}

Item::Iterator::Ptr IntersectIterator::copy() const
{
    return Item::Iterator::Ptr(new IntersectIterator(m_it1->copy(), m_it2->copy()));
}

// vim: et:ts=4:sw=4:sts=4
