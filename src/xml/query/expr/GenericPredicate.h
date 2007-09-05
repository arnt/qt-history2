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

#ifndef Patternist_GenericPredicate_H
#define Patternist_GenericPredicate_H

#include "PairContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A predicate that can handle all kinds of predicates and
     * is therefore not very efficient, but can cope with all the tricky scenarios.
     *
     * @see FirstItemPredicate
     * @see TruthPredicate
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class GenericPredicate : public PairContainer
    {
    public:

        /**
         * Creates a predicate expression that filters the items gained
         * from evaluating @p sourceExpression through the filter @p predicateExpression.
         *
         * This function performs type analyzis on the passed expressions, and may
         * return more specialized expressions depending on the analyzis.
         *
         * If @p predicateExpression is an invalid predicate, an error is issued
         * via the @p context.
         */
        static Expression::Ptr create(const Expression::Ptr &sourceExpression,
                                      const Expression::Ptr &predicateExpression,
                                      const StaticContext::Ptr &context,
                                      const QSourceLocation &location);

        /**
         * Creates a source iterator which is passed to the ItemMappingIterator
         * and the Focus. The ItemMappingIterator modifies it with
         * its Iterator::next() calls, and since the Focus references the same Iterator,
         * the focus is automatically moved.
         */
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

        /**
         * Doesn't return the first item from calling evaluateSequence(), but does the mapping
         * manually. This avoid allocating an ItemMappingIterator.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        inline Item mapToItem(const Item &subject,
                                   const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * @returns always CreatesFocusForLast.
         */
        virtual Properties properties() const;

        virtual QString description() const;
    protected:

        /**
         * Creates a GenericPredicate which filters the items from the @p sourceExpression
         * through @p predicate.
         *
         * This constructor is protected. The proper way to create predicates is via the static
         * create() function.
         */
        GenericPredicate(const Expression::Ptr &sourceExpression,
                         const Expression::Ptr &predicate);

        /**
         * @returns the ItemType of the first operand's staticType().
         */
        virtual ItemType::Ptr newContextItemType() const;

    private:
        typedef PlainSharedPtr<GenericPredicate> Ptr;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
