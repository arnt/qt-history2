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

#ifndef Patternist_LetClause_H
#define Patternist_LetClause_H

#include "qpaircontainer_p.h"
#include "qvariabledeclaration_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{

    /**
     * @short Represents a <tt>let</tt>-clause, but is only used at compile
     * time.
     *
     * LetClause is inserted into the AST tree for the single purpose of
     * ensuring that the focus is correct for the binding expression. Once
     * that is done, LetClause rewrites itself to its
     * <tt>return</tt> expression, and the ExpressionVariableReference will
     * handle the evaluation of the variable.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class LetClause : public PairContainer
    {
    public:
        LetClause(const Expression::Ptr &operand1,
                  const Expression::Ptr &operand2,
                  const VariableDeclaration::Ptr &decl);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual SequenceType::Ptr staticType() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

    private:
        inline DynamicContext::Ptr bindVariable(const DynamicContext::Ptr &context) const;

        const VariableDeclaration::Ptr m_varDecl;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
