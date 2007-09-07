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

#ifndef Patternist_ForClause_H
#define Patternist_ForClause_H

#include "qpaircontainer_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements XPath 2.0's @c for expression.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-for-expressions">XML Path Language
     * (XPath) 2.0, 3.7 For Expressions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ForClause : public PairContainer
    {
    public:
        ForClause(const VariableSlotID varSlot,
                  const Expression::Ptr &bindingSequence,
                  const Expression::Ptr &returnExpression,
                  const VariableSlotID positionSlot);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        virtual SequenceType::Ptr staticType() const;
        virtual SequenceType::List expectedOperandTypes() const;
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual QList<PlainSharedPtr<OptimizationPass> > optimizationPasses() const;

        inline Item mapToItem(const Item &item,
                                   const DynamicContext::Ptr &context) const;

        inline Item::Iterator::Ptr mapToSequence(const Item &item,
                                                 const DynamicContext::Ptr &context) const;

        /**
         * Sets m_allowsMany properly.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

    private:
        inline void riggPositionalVariable(const DynamicContext::Ptr &context,
                                           const Item::Iterator::Ptr &source) const;
        typedef PlainSharedPtr<ForClause> Ptr;
        const VariableSlotID m_varSlot;
        const VariableSlotID m_positionSlot;
        /**
         * Initialized to @c false. This default is always safe.
         */
        bool m_allowsMany;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
