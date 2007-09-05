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

#ifndef Patternist_AndExpression_H
#define Patternist_AndExpression_H

#include "PairContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements XPath 2.0's logical expression @c and.
     *
     * The @c and expression is the same in XQuery 1.0 as in XPath 2.0.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-logical-expressions">XML Path Language
     * (XPath) 2.0, 3.6 Logical Expressions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class AndExpression : public PairContainer
    {
    public:
        AndExpression(const Expression::Ptr &operand1,
                      const Expression::Ptr &operand2);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

        virtual SequenceType::List expectedOperandTypes() const;

        virtual Expression::Ptr compress(const StaticContext::Ptr &context);
        /**
         * @returns always CommonSequenceTypes::ExactlyOneBoolean
         */
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
