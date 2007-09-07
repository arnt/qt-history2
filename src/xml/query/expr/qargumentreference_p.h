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

#ifndef Patternist_ArgumentReference_H
#define Patternist_ArgumentReference_H

#include "qvariablereference_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A reference to an argument declared in a UserFunction.
     *
     * This is in other words a variable reference in side a function
     * body, that references a function argument.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ArgumentReference : public VariableReference
    {
    public:
        ArgumentReference(const SequenceType::Ptr &sourceType,
                          const VariableSlotID slot);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

    private:
        const SequenceType::Ptr m_type;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
