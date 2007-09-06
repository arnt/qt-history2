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

#include "ExceptIterator.h"

using namespace Patternist;

ExceptIterator::ExceptIterator(const Item::Iterator::Ptr &it1,
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

Item ExceptIterator::next()
{
    qDebug() << Q_FUNC_INFO;

    while(true)
    {
        if(!m_node1)
        {
            m_position = -1;
            m_current = Item();
            return Item();
        }

        if(!m_node2)
            return nextFromFirstOperand();

        switch(m_node1.asNode().compareOrderTo(m_node2.asNode()))
        {
            case Node::Precedes:
                return nextFromFirstOperand();
            case Node::Follows:
            {
                m_node2 = m_it2->next();
                if(!m_node2)
                    return nextFromFirstOperand();

                break;
            }
            default:
            {
                m_node1 = m_it1->next();
                m_node2 = m_it2->next();
            }
        }
    }

    return Item();
}

Item ExceptIterator::current() const
{
    return m_current;
}

xsInteger ExceptIterator::position() const
{
    return m_position;
}

Item::Iterator::Ptr ExceptIterator::copy() const
{
    return Item::Iterator::Ptr(new ExceptIterator(m_it1->copy(), m_it2->copy()));
}

// vim: et:ts=4:sw=4:sts=4
