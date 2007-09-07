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

#ifndef Patternist_ValueComparison_H
#define Patternist_ValueComparison_H

#include "qatomiccomparator_p.h"
#include "qpaircontainer_p.h"
#include "qcomparisonplatform_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements XPath 2.0 value comparions, such as the <tt>eq</tt> operator.
     *
     * ComparisonPlatform is inherited with @c protected scope because ComparisonPlatform
     * must access members of ValueComparison.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-value-comparisons">XML Path Language
     * (XPath) 2.0, 3.5.1 Value Comparisons</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ValueComparison : public PairContainer,
                            protected ComparisonPlatform<ValueComparison, true>
    {
    public:
        ValueComparison(const Expression::Ptr &op1,
                        const AtomicComparator::Operator op,
                        const Expression::Ptr &op2);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        /**
         * @returns always CommonSequenceTypes::ExactlyOneBoolean
         */
        virtual SequenceType::Ptr staticType() const;

        virtual SequenceType::List expectedOperandTypes() const;

        /**
         * @returns IDValueComparison
         */
        virtual ID id() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual QList<PlainSharedPtr<OptimizationPass> > optimizationPasses() const;

        /**
         * Overriden to optimize case-insensitive compares.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * @returns the operator that this ValueComparison is using.
         */
        inline AtomicComparator::Operator operatorID() const
        {
            return m_operator;
        }

        /**
         * It is considered that the string value from @p op1 will be compared against @p op2. This
         * function determines whether the user intends the comparison to be case insensitive. If
         * that is the case @c true is returned, and the operands are re-written appropriately.
         *
         * This is a helper function for Expression classes that compares strings.
         *
         * @see ComparisonPlatform::useCaseInsensitiveComparator()
         */
        static bool isCaseInsensitiveCompare(Expression::Ptr &op1, Expression::Ptr &op2);

    private:
        const AtomicComparator::Operator m_operator;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
