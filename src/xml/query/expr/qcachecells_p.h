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

#ifndef Patternist_CacheCells_H
#define Patternist_CacheCells_H

#include <QList>
#include <QVector>

#include "qitem_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Represents a cache entry for a single Item,
     * as opposed to for a sequence of items.
     *
     * A characteristic of the ItemCacheCell is that it has two states:
     * either its full or it's not, since it only deals with a single
     * item.
     *
     * Remember that cachedItem doesn't tell the state of the ItemCacheCell.
     * For instance, it can have a null pointer, the empty sequence, and that
     * can be the value of its cache.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ItemCacheCell
    {
    public:
        typedef QList<ItemCacheCell> List;
        typedef QVector<ItemCacheCell> Vector;
        enum CacheState
        {
            Full,
            Empty
        };

        inline ItemCacheCell() : cacheState(Empty)
        {
        }

        Item        cachedItem;
        CacheState  cacheState : 2;
    };

    /**
     * @short Represents a cache entry for a sequence of items.
     *
     * As opposed to ItemCacheCell, ItemSequenceCacheCell can be partially
     * populated: e.g, four items is in the cache while three remains in the
     * source. For that reason ItemSequenceCacheCell in addition to the source
     * also carried an Iterator which is the source, such that it can continue
     * to populate the cache when it runs out.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ItemSequenceCacheCell
    {
    public:
        typedef QList<ItemSequenceCacheCell> List;
        typedef QVector<ItemSequenceCacheCell> Vector;

        enum CacheState
        {
            Full,
            Empty,
            PartiallyPopulated
        };

        inline ItemSequenceCacheCell() : cacheState(Empty)
        {
        }

        Item::List          cachedItems;
        Item::Iterator::Ptr sourceIterator;
        CacheState          cacheState : 2;
    };
}

Q_DECLARE_TYPEINFO(Patternist::ItemCacheCell, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Patternist::ItemSequenceCacheCell, Q_MOVABLE_TYPE);

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
