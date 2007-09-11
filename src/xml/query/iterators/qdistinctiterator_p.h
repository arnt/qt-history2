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

#ifndef Patternist_DistinctIterator_H
#define Patternist_DistinctIterator_H

#include <QList>

#include "qexpression_p.h"
#include "qitem_p.h"
#include "qatomiccomparator_p.h"
#include "qcomparisonplatform_p.h"
#include "qsourcelocationreflection_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Filters another sequence by removing duplicates such that the items are unique.
     *
     * DistinctIterator takes an input sequence, and returns a sequence where each
     * item is unique. Thus, DistinctIterator removes the duplicates of items
     * in a sequence. DistinctIterator is central in the implementation of the
     * <tt>fn:distinct-values()</tt> function.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-distinct-values">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 15.1.6 fn:distinct-values</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    class DistinctIterator : public Item::Iterator
                           , public ComparisonPlatform<DistinctIterator, false>
                           , public SourceLocationReflection
    {
    public:
        /**
         * Creates a DistinctIterator.
         * @param comp the AtomicComparator to be used for comparing values. This may be @c null,
         * meaning the IndexOfIterator iterator will dynamically determine what comparator to use
         * @param seq the sequence whose duplicates should be filtered out
         * @param context the usual context, used for error reporting and by AtomicComparators.
         */
        DistinctIterator(const Item::Iterator::Ptr &seq,
                         const AtomicComparator::Ptr &comp,
                         const Expression::Ptr &expression,
                         const DynamicContext::Ptr &context);

        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;
        virtual Item::Iterator::Ptr copy() const;
        virtual const SourceLocationReflection *actualReflection() const;

        inline AtomicComparator::Operator operatorID() const
        {
            return AtomicComparator::OperatorEqual;
        }

    private:
        const Item::Iterator::Ptr   m_seq;
        const DynamicContext::Ptr   m_context;
        const Expression::Ptr       m_expr;
        Item                   m_current;
        xsInteger                   m_position;
        Item::List                  m_processed;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
