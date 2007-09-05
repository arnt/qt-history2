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

#ifndef Patternist_RangeVariableReference_H
#define Patternist_RangeVariableReference_H

#include "VariableReference.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A reference to a variable declared with @c for or a quantification
     * expression, but not for instance a @c let binding.
     *
     * A range variable always represents a single item, while an other
     * expression provides the binding and iteration. A @c for expression is
     * a good example.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class RangeVariableReference : public VariableReference
    {
    public:
        RangeVariableReference(const Expression::Ptr &sourceExpression,
                               const VariableSlotID slot);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;

        /**
         * @returns IDRangeVariableReference
         */
        virtual ID id() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual Properties properties() const;
    private:
        const Expression::Ptr m_sourceExpression;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
