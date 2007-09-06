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

#ifndef Patternist_FirstItemPredicate_H
#define Patternist_FirstItemPredicate_H

#include "SingleContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A predicate that always selects the first item from its sequence.
     *
     * FirstItemPredicate corresponds exactly to the predicate
     * in the expression <tt>input[1]</tt>.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class FirstItemPredicate : public SingleContainer
    {
    public:
        /**
         * Creates a FirstItemPredicate that filters @p source.
         */
        FirstItemPredicate(const Expression::Ptr &source);

        /**
         * @returns the first item, if any, from evaluating the source expression.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        /**
         * @returns a list containing one CommonSequenceTypes::ZeroOrMoreItems instance.
         */
        virtual SequenceType::List expectedOperandTypes() const;

        /**
         * @returns a SequenceType where the item type is the same as the source expression
         * and where the cardinality is either Cardinality::zeroOrOne() or Cardinality::exactlyOne(),
         * depending on the source expression.
         */
        virtual SequenceType::Ptr staticType() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * Rewrites <tt>expression[1][1]</tt> into <tt>expression[1]</tt>.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * @returns always IDFirstItemPredicate.
         */
        virtual ID id() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
