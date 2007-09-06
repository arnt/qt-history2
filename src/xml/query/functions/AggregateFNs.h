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

#ifndef Patternist_AggregateFNs_H
#define Patternist_AggregateFNs_H

#include "Aggregator.h"
#include "AtomicComparator.h"
#include "AtomicMathematician.h"
#include "ComparisonPlatform.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#aggregate-functions">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 15.4 Aggregate Functions</a>.
 *
 * @todo document that some functions have both eval funcs implented.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the function <tt>fn:count()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CountFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        /**
         * If @p reqType is CommonSequenceTypes::EBV, this function call is rewritten
         * into a call to <tt>fn:exists()</tt>. Hence, <tt>if(count(X)) then ...</tt> is
         * rewritten into <tt>if(exists(X)) then ...</tt>.
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        /**
         * If CountFN's operand has a Cardinality that is exact, as per Cardinality::isExact(),
         * it is rewritten to the Cardinality's count.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);
    };

    /**
     * @short Base class for the implementations of the <tt>fn:avg()</tt> and <tt>fn:sum()</tt> function.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AddingAggregate : public FunctionCall
    {
    public:
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
    protected:
        AtomicMathematician::Ptr m_mather;
    };

    /**
     * @short Implements the function <tt>fn:avg()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AvgFN : public AddingAggregate
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        virtual SequenceType::Ptr staticType() const;
    private:
        AtomicMathematician::Ptr m_adder;
        AtomicMathematician::Ptr m_divider;
    };

    /**
     * @short Implements the function <tt>fn:sum()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SumFN : public AddingAggregate
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual SequenceType::Ptr staticType() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
