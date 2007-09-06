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

#ifndef Patternist_IndexOfIterator_H
#define Patternist_IndexOfIterator_H

#include "Item.h"
#include "AtomicComparator.h"
#include "ComparisonPlatform.h"
#include "DynamicContext.h"
#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Forms an Iterator over a sequence of integers, which each is the position
     * of where a search parameter appeared in another Iterator.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-index-of">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 15.1.3 fn:index-of</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    class IndexOfIterator : public Item::Iterator
                          , protected ComparisonPlatform<IndexOfIterator, false>
                          , public SourceLocationReflection
    {
    public:

        /**
         * Creates an IndexOfIterator, whose next() function returns integers being
         * the index positions of where @p searchParam was found in @p inputSequence.
         *
         * @param comp the AtomicComparator to be used for comparing values. This may be @c null,
         * meaning the IndexOfIterator iterator will dynamically determine what comparator to use
         * on an item per item basis, which is slower.
         * @param searchParam the item which should be compared to the items in @p inputSequence.
         * @param inputSequence the input sequence which indexes of the @p searchParam should
         * be returned for.
         * @param context the usual DynamicContext
         */
        IndexOfIterator(const Item::Iterator::Ptr &inputSequence,
                        const Item &searchParam,
                        const AtomicComparator::Ptr &comp,
                        const DynamicContext::Ptr &context,
                        const Expression::Ptr &expr);

        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;
        virtual Item::Iterator::Ptr copy() const;

        inline AtomicComparator::Operator operatorID() const
        {
            return AtomicComparator::OperatorEqual;
        }

        virtual const SourceLocationReflection *actualReflection() const;

    private:
        const Item::Iterator::Ptr m_seq;
        const Item m_searchParam;
        const DynamicContext::Ptr m_context;
        const Expression::Ptr m_expr;
        Item m_current;
        xsInteger m_position;
        xsInteger m_seqPos;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
