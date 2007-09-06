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

#ifndef Patternist_IntersectIterator_H
#define Patternist_IntersectIterator_H

#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the @c intersect operator. That is, the computation
     * of the intersection between two sequences of nodes.
     *
     * @ingroup Patternist_iterators
     */
    class IntersectIterator : public Item::Iterator
    {
    public:
        /**
         * It is assumed that @p it1 and @p it2 are in document order and
         * without duplicates.
         */
        IntersectIterator(const Item::Iterator::Ptr &it1,
                          const Item::Iterator::Ptr &it2);

        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;
        virtual Item::Iterator::Ptr copy() const;

    private:
        inline Item closedExit()
        {
            m_position = -1;
            m_current = Item();
            return Item();
        }

        const Item::Iterator::Ptr   m_it1;
        const Item::Iterator::Ptr   m_it2;
        Item                   m_current;
        xsInteger                   m_position;
        Item                   m_node1;
        Item                   m_node2;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
