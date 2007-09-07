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

#ifndef Patternist_ExpressionVariableReference_H
#define Patternist_ExpressionVariableReference_H

#include "qvariabledeclaration_p.h"
#include "qvariablereference_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A reference to a variable declared with <tt>declare variable</tt> or @c let.
     *
     * It's also used by variable bindings in @c case branches of the @c typeswitch
     * expression.
     *
     * This AST node is only used up until the typeCheck() stage. Therefore it
     * has no functions for evaluation, such as evaluateSequence().
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ExpressionVariableReference : public VariableReference
    {
    public:
        ExpressionVariableReference(const VariableSlotID slot,
                                    const VariableDeclaration::Ptr varDecl);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual SequenceType::Ptr staticType() const;
        virtual ID id() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        inline const Expression::Ptr &sourceExpression() const;

    private:
        const VariableDeclaration::Ptr m_varDecl;
    };

    inline const Expression::Ptr &ExpressionVariableReference::sourceExpression() const
    {
        return m_varDecl->expression();
    }

}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
