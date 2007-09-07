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

#ifndef Patternist_RangeExpression_H
#define Patternist_RangeExpression_H

#include "qpaircontainer_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements XPath 2.0's @c to expression.
     *
     * Despite its name, RangeExpression is not related to RangeVariableDeclaration.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#construct_seq">XML Path Language
     * (XPath) 2.0, 3.3.1 Constructing Sequences</a>
     * @see RangeIterator
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class RangeExpression : public PairContainer
    {
    public:
        RangeExpression(const Expression::Ptr &operand1, const Expression::Ptr &operand2);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;
        /**
         * It's likely that this function gets called if staticType() inferred
         * the cardinality to an exact number. In that case, we know that the
         * first arguments is the same as the second argument.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;

        /**
         * @returns always CommonSequenceTypes::ZeroOrMoreIntegers
         */
        virtual SequenceType::Ptr staticType() const;

        /**
         * Disables compression for optimization reasons. For example, the
         * expression "1 to 1000" would consume thousand allocated instances
         * of Integer, and RangeIterator is well suited for dynamic evaluation.
         *
         * @returns Expression::DisableElimination
         */
        virtual Expression::Properties properties() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
