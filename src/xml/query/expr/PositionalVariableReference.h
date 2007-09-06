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
***************************************************************************
*/

#ifndef Patternist_PositionVariableReference_H
#define Patternist_PositionVariableReference_H

#include "VariableReference.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A reference to an @c at variable, declared with the
     * <tt>for</tt>-part in XQuery's FLWOR expression.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class PositionalVariableReference : public VariableReference
    {
    public:
        typedef PlainSharedPtr<PositionalVariableReference> Ptr;
        PositionalVariableReference(const VariableSlotID slot);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

        /**
         * Returns always @c true, since a positional variable is always one or more, and the
         * Effective %Boolean Value for that range is always @c true.
         *
         * @returns always @c true
         */
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

        /**
         * @returns always CommonSequenceTypes::ExactlyOneInteger
         */
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual Properties properties() const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
