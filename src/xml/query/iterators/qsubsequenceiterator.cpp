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

#include "qdebug_p.h"
#include "qlistiterator_p.h"
#include "qxpathhelper_p.h"

#include "qsubsequenceiterator_p.h"

using namespace Patternist;

SubsequenceIterator::SubsequenceIterator(const Item::Iterator::Ptr &iterator,
                                         const xsInteger start,
                                         const xsInteger len)
                                         : m_position(0),
                                           m_it(iterator),
                                           m_counter(start),
                                           m_start(start),
                                           m_len(len),
                                           m_stop(m_start + m_len)
{
    qDebug() << Q_FUNC_INFO << "start: " << start << " len: " << len;
    Q_ASSERT(iterator);
    Q_ASSERT(start >= 1);
    Q_ASSERT(len == -1 || len >= 1);

    /* Note, "The first item of a sequence is located at position 1, not position 0." */
    for(xsInteger i = 1; i != m_start; ++i)
        m_it->next();
}

Item SubsequenceIterator::next()
{
    if(m_position == -1)
        return Item();

    m_current = m_it->next();
    ++m_position;

    if(m_len == -1)
    {
        if(!m_current)
            m_position = -1;

        return m_current;
    }

    ++m_counter;

    if(!(m_counter > m_stop) && m_current)
        return m_current;

    m_position = -1;
    m_current.reset();
    return Item();
}

Item SubsequenceIterator::current() const
{
    return m_current;
}

xsInteger SubsequenceIterator::position() const
{
    return m_position;
}

Item::Iterator::Ptr SubsequenceIterator::copy() const
{
    return Item::Iterator::Ptr(new SubsequenceIterator(m_it->copy(), m_start, m_len));
}

// vim: et:ts=4:sw=4:sts=4
