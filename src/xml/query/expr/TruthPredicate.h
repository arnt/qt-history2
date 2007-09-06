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

#ifndef Patternist_TruthPredicate_H
#define Patternist_TruthPredicate_H

#include "GenericPredicate.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A predicate which is optimized for filter expressions that
     * are of type @c xs:boolean.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class TruthPredicate : public GenericPredicate
    {
    public:
        /**
         * Creates a TruthPredicate which filters the items from the @p sourceExpression
         * through @p predicate.
         *
         * This constructor is protected. The proper way to create predicates is via the static
         * create() function.
         */
        TruthPredicate(const Expression::Ptr &sourceExpression,
                       const Expression::Ptr &predicate);

        inline Item mapToItem(const Item &item, const DynamicContext::Ptr &context) const
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "This is practically dead code because it never gets called in GenericPredicate, "
                                           "which binds to its own mapToItem for completely legitime reasons.");
            if(m_operand2->evaluateEBV(context))
                return item;
            else
                return Item();
        }

        inline Item::Iterator::Ptr map(const Item &item,
                                       const DynamicContext::Ptr &context) const
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "I don't expect this function to be called, for the same reasons as above.");
            if(m_operand2->evaluateEBV(context))
                return makeSingletonIterator(item);
            else
                return CommonValues::emptyIterator;
        }

        virtual SequenceType::List expectedOperandTypes() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
