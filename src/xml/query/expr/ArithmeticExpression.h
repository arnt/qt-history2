/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_ArithmeticExpression_H
#define Patternist_ArithmeticExpression_H

#include "AtomicMathematician.h"
#include "PairContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements arithmetics, such as multiplication and substraction.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-arithmetic">XML Path Language
     * (XPath) 2.0, 3.4 Arithmetic Expressions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ArithmeticExpression : public PairContainer
    {
    public:
        ArithmeticExpression(const Expression::Ptr &operand1,
                             const AtomicMathematician::Operator op,
                             const Expression::Ptr &operand2);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        virtual SequenceType::Ptr staticType() const;
        virtual SequenceType::List expectedOperandTypes() const;
        AtomicMathematician::Operator operatorID() const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        static Item flexiblyCalculate(const Item &op1,
                                      const AtomicMathematician::Operator op,
                                      const Item &op2,
                                      const AtomicMathematician::Ptr &mather,
                                      const DynamicContext::Ptr &context,
                                      const SourceLocationReflection *const reflection,
                                      const ReportContext::ErrorCode code = ReportContext::XPTY0004);

        static AtomicMathematician::Ptr
        fetchMathematician(Expression::Ptr &t1,
                           Expression::Ptr &t2,
                           const AtomicMathematician::Operator op,
                           const bool issueError,
                           const ReportContext::Ptr &context,
                           const SourceLocationReflection *const reflection,
                           const ReportContext::ErrorCode code = ReportContext::XPTY0004);
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

    private:
        const AtomicMathematician::Operator m_op;
        AtomicMathematician::Ptr m_mather;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
