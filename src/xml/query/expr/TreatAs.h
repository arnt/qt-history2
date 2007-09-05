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

#ifndef Patternist_TreatAs_H
#define Patternist_TreatAs_H

#include "SingleContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements XPath 2.0's <tt>treat as</tt> expression.
     *
     * TreatAs is always a compile-time class only, and is always deallocated by re-writing
     * to CardinalityVerifier or ItemVerifier or both, by calling
     * TypeChecker::applyFunctionConversion(). One approach could be to skip instantiating TreatAs and
     * simply let the return value of TypeChecker::applyFunctionConversion() be inserted into the AST, but
     * that wouldn't handle type checking the context item properly, which depends on that the StaticContext
     * have been set by the parent Expression.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-treat">XML Path Language
     * (XPath) 2.0, 3.10.5 Treat</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class TreatAs : public SingleContainer
    {
    public:
        /**
         * Creats a TreatAs where it is checked that the expression @p operand conforms
         * to the type @p reqType.
         */
        TreatAs(const Expression::Ptr &operand,
                const SequenceType::Ptr &reqType);

        /**
         * This function rewrites always. First the type that this TreatAs expression tests for
         * is verified. Then, the type the the <tt>treat as</tt> expression itself must match, @p reqType,
         * is verified.
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        /**
         * @returns always the SequenceType passed in the constructor to this class. That is, the
         * SequenceType that the operand must conform to.
         */
        virtual SequenceType::Ptr staticType() const;

        /**
         * @returns a list containing one CommonSequenceTypes::ZeroOrMoreItems
         */
        virtual SequenceType::List expectedOperandTypes() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

    private:
        const SequenceType::Ptr m_reqType;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
