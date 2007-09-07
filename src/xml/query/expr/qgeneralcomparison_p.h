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

#ifndef Patternist_GeneralComparison_H
#define Patternist_GeneralComparison_H

#include "qatomiccomparator_p.h"
#include "qpaircontainer_p.h"
#include "qcomparisonplatform_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements XPath 2.0's general comparions, such as the <tt>=</tt> operator.
     *
     * ComparisonPlatform is inherited with @c protected scope because ComparisonPlatform
     * must access members of GeneralComparison.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-general-comparisons">XML Path Language
     * (XPath) 2.0, 3.5.2 General Comparisons</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class GeneralComparison : public PairContainer,
                              protected ComparisonPlatform<GeneralComparison,
                                                           true /* We want to report errors. */,
                                                           AtomicComparator::AsGeneralComparison>
    {
    public:
        GeneralComparison(const Expression::Ptr &op1,
                          const AtomicComparator::Operator op,
                          const Expression::Ptr &op2);

        virtual bool evaluateEBV(const DynamicContext::Ptr &) const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * @returns always IDGeneralComparison
         */
        virtual ID id() const;

        virtual QList<PlainSharedPtr<OptimizationPass> > optimizationPasses() const;

        /**
         * @returns the operator that this GeneralComparison is using.
         */
        inline AtomicComparator::Operator operatorID() const
        {
            return m_operator;
        }

        /**
         * Overriden to optimize case-insensitive compares.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

    private:
        AtomicComparator::Ptr fetchGeneralComparator(Expression::Ptr &op1,
                                                     Expression::Ptr &op2,
                                                     const ReportContext::Ptr &context) const;
        bool generalCompare(const Item &op1,
                            const Item &op2,
                            const DynamicContext::Ptr &context) const;

        const AtomicComparator::Operator m_operator;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
