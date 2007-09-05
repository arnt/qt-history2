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

#ifndef Patternist_EmptySequence_H
#define Patternist_EmptySequence_H

#include "EmptyContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of empty sequence: <tt>()</tt>.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class EmptySequence : public EmptyContainer
    {
    public:
        /**
         * @short Creates an EmptySequence that is a replacement for @p
         * replacementFor.
         *
         * @see EmptySequence()
         */
        static Expression::Ptr create(const Expression *const replacementFor,
                                      const StaticContext::Ptr &context);


        /**
         * @short Creates an instance of EmptySequence.
         *
         * @note In most cases create() should be used, since it takes care of
         * adjusting source location annotations.
         *
         * @see create()
         */
        inline EmptySequence()
        {
        }

        virtual QString stringValue() const;

        /**
         * @returns always an empty iterator, an instance of EmptyIterator.
         */
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

        /**
         * @returns always @c null.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        /**
         * Does nothing.
         */
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &) const;

        /**
         * @returns always @c false.
         */
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

        /**
         * @returns always CommonSequenceTypes::Empty
         */
        virtual ItemType::Ptr type() const;

        /**
         * @returns always CommonSequenceTypes::Empty
         */
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual ID id() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
