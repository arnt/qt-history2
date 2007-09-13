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

#ifndef Patternist_QuantifiedExpression_H
#define Patternist_QuantifiedExpression_H

#include "qpaircontainer_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Implements XPath 2.0's quantification expressions @c some and @c every.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-quantified-expressions">XML Path Language
     * (XPath) 2.0, 3.9 Quantified Expressions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class Q_AUTOTEST_EXPORT QuantifiedExpression : public PairContainer
    {
    public:
        enum Operator
        {
            Some    = 1,
            Every
        };

        QuantifiedExpression(const VariableSlotID varSlot,
                             const Operator quantifier,
                             const Expression::Ptr &inClause,
                             const Expression::Ptr &testExpression);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;
        virtual SequenceType::List expectedOperandTypes() const;

        Operator operatorID() const;

        /**
         * Determines the string representation for a quantification operator.
         *
         * @return "some" if @p quantifier is Some, or "every" if @p quantifier
         * is Every
         */
        static QString displayName(const Operator quantifier);

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        inline Item mapToItem(const Item &item, const DynamicContext::Ptr &context) const;

    private:
        typedef PlainSharedPtr<QuantifiedExpression> Ptr;
        const VariableSlotID m_varSlot;
        const Operator m_quantifier;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
