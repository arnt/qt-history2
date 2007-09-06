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

#ifndef Patternist_RemovalIterator_H
#define Patternist_RemovalIterator_H

#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Removes one items at a specified position from an input Iterator.
     *
     * RemoveIterator removes an item from a sequence at a certain position,
     * while retaining the pull-based characteristic of being an Iterator itself. The
     * RemovalIterator's constructor is passed an Iterator, the Iterator to
     * remove from, and the position of the item to remove. When calling the RemovalIterator's
     * functions, it acts as an ordinary Iterator, taking into account that
     * one item is removed from the source Iterator.
     *
     * The RemovalIterator class contains the central business logic for implementing the
     * <tt>fn:remove()</tt> function, whose definition therefore specifies the detailed behaviors
     * of RemovalIterator.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-remove">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 15.1.8 fn:remove</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    class RemovalIterator : public Item::Iterator
    {
    public:

        /**
         * Creates an RemovalIterator.
         *
         * @param target the Iterator containing the sequence of items
         * which the item at position @p position should be removed from.
         * @param position the position of the item to remove. Must be
         * 1 or larger.
         */
        RemovalIterator(const Item::Iterator::Ptr &target,
                        const xsInteger position);

        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;

        /**
         * The Iterator's count is computed by substracting one from the source
         * Iterator's count.
         */
        virtual xsInteger count();

        /**
         * Computes the Cardinality from count().
         */
        virtual Cardinality cardinality();
        virtual Item::Iterator::Ptr copy() const;

    private:
        const Item::Iterator::Ptr m_target;
        const xsInteger m_removalPos;
        Item m_current;
        xsInteger m_position;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
