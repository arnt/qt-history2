/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include "AccelIterators.h"

using namespace Patternist;

xsInteger AccelIterator::position() const
{
    return m_position;
}

Item AccelIterator::current() const
{
    return m_current;
}

Item FollowingIterator::next()
{
    /* "the following axis contains all nodes that are descendants
     *  of the root of the tree in which the context node is found,
     *  are not descendants of the context node, and occur after
     *  the context node in document order." */

    if(m_position == 0)
    {
        /* Skip the descendants. */
        m_currentPre += m_document->size(m_preNumber) + 1;
    }

    if(m_currentPre > m_document->maximumPreNumber())
        return closedExit();

    while(m_document->kind(m_currentPre) == Node::Attribute)
    {
        ++m_currentPre;
        if(m_currentPre > m_document->maximumPreNumber())
            return closedExit();
    }

    m_current = m_document->createNode(m_currentPre);
    ++m_position;
    ++m_currentPre;
    return m_current;
}

Item PrecedingIterator::next()
{
    qDebug() << Q_FUNC_INFO;
    if(m_currentPre == -1)
        return closedExit();

    /* Skip ancestors. */
    while(m_document->postNumber(m_currentPre) > m_postNumber)
        ++m_currentPre;

    while(m_document->kind(m_currentPre) == Node::Attribute)
        ++m_currentPre;

    if(m_currentPre >= m_preNumber)
    {
        m_currentPre = -1;
        return closedExit();
    }

    /* Phew, m_currentPre is now 1) not an ancestor; and
     * 2) not an attribute; and 3) preceds the context node. */

    m_current = m_document->createNode(m_currentPre);
    ++m_position;
    ++m_currentPre;

    return m_current;
}

Item::Iterator::Ptr PrecedingIterator::copy() const
{
    return Item::Iterator::Ptr(new PrecedingIterator(m_document, m_preNumber));
}

Item::Iterator::Ptr FollowingIterator::copy() const
{
    return Item::Iterator::Ptr(new FollowingIterator(m_document, m_preNumber));
}

Item ChildIterator::next()
{
    if(m_currentPre == -1)
        return closedExit();

    ++m_position;
    m_current = m_document->createNode(m_currentPre);

    /* We get the count of the descendants, and increment m_currentPre. After
     * this, m_currentPre is the node after the descendants. */
    m_currentPre += m_document->size(m_currentPre);
    ++m_currentPre;

    if(m_currentPre > m_document->maximumPreNumber() || m_document->depth(m_currentPre) != m_depth)
        m_currentPre = -1;

    return m_current;
}

Item::Iterator::Ptr ChildIterator::copy() const
{
    return Item::Iterator::Ptr(new ChildIterator(m_document, m_preNumber));
}

Item AttributeIterator::next()
{
    if(m_currentPre == -1)
        return closedExit();
    else
    {
        m_current = m_document->createNode(m_currentPre);
        ++m_position;

        ++m_currentPre;

        if(m_currentPre > m_document->maximumPreNumber() ||
           m_document->kind(m_currentPre) != Node::Attribute)
            m_currentPre = -1;

        return m_current;
    }
}

Item::Iterator::Ptr AttributeIterator::copy() const
{
    return Item::Iterator::Ptr(new AttributeIterator(m_document, m_preNumber));
}

// vim: et:ts=4:sw=4:sts=4
