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

#include <QtDebug>

#include "ListIterator.h"

#include "CachingIterator.h"

using namespace Patternist;

CachingIterator::CachingIterator(ItemSequenceCacheCell::Vector &cacheCells,
                                 const VariableSlotID slot,
                                 const DynamicContext::Ptr &context) : m_position(0),
                                                                       m_slot(slot),
                                                                       m_context(context),
                                                                       m_cacheCells(cacheCells),
                                                                       m_usingCache(true)
{
    Q_ASSERT(m_slot > -1);
    Q_ASSERT(m_context);
    Q_ASSERT(m_cacheCells.at(m_slot).sourceIterator);
    Q_ASSERT_X((m_cacheCells.at(m_slot).cachedItems.isEmpty() && m_cacheCells.at(m_slot).cacheState == ItemSequenceCacheCell::Empty) ||
               m_cacheCells.at(m_slot).cacheState == ItemSequenceCacheCell::PartiallyPopulated,
               Q_FUNC_INFO,
               "It makes no sense to construct a CachingIterator for a cache that is ItemSequenceCacheCell::Full.");
}

Item CachingIterator::next()
{
    ItemSequenceCacheCell &cell = m_cacheCells[m_slot];
    qDebug() << Q_FUNC_INFO
             << this
             << "c state:"      << cell.cacheState
             << "using Cache?"  << m_usingCache
             << "m_position:"   << m_position
             << "cached items:" << cell.cachedItems.count();
    if(m_position == -1)
        return Item();

    if(m_usingCache)
    {
        ++m_position;

        /* Iterator::position() starts at 1, while Qt's container classes
         * starts at 0. */
        if(m_position - 1 < cell.cachedItems.count())
        {
            m_current = cell.cachedItems.at(m_position - 1);
            return m_current;
        }
        else
        {
            cell.cacheState = ItemSequenceCacheCell::PartiallyPopulated;
            m_usingCache = false;
            /* We decrement here so we don't have to add a branch for this
             * when using the source Iterator below. */
            --m_position;
        }
    }

    m_current = cell.sourceIterator->next();
    qDebug() << "Have item?" << m_current;

    if(m_current)
    {
        cell.cachedItems.append(m_current);
        Q_ASSERT(cell.cacheState == ItemSequenceCacheCell::PartiallyPopulated);
        ++m_position;
        return m_current;
    }
    else
    {
        m_position = -1;
        cell.cacheState = ItemSequenceCacheCell::Full;
        return Item();
    }
}

Item CachingIterator::current() const
{
    return m_current;
}

xsInteger CachingIterator::position() const
{
    return m_position;
}

Item::Iterator::Ptr CachingIterator::copy() const
{
    const ItemSequenceCacheCell &cell = m_cacheCells.at(m_slot);
    if(cell.cacheState == ItemSequenceCacheCell::Full)
        return makeListIterator(cell.cachedItems);
    else
        return Item::Iterator::Ptr(new CachingIterator(m_cacheCells, m_slot, m_context));
}

// vim: et:ts=4:sw=4:sts=4
