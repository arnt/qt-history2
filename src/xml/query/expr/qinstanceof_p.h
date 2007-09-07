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

#ifndef Patternist_InstanceOf_H
#define Patternist_InstanceOf_H

#include "SingleContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements XPath 2.0's <tt>instance of</tt> expression.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-instance-of">XML Path Language (XPath) 2.0,
     * 3.10.1 Instance Of</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class InstanceOf : public SingleContainer
    {
    public:

        InstanceOf(const Expression::Ptr &operand,
                   const SequenceType::Ptr &targetType);

        virtual bool evaluateEBV(const DynamicContext::Ptr &) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        /**
         * @returns the SequenceType that this <tt>instance of</tt> Expression
         * is testing its operand against.
         */
        SequenceType::Ptr targetType() const;

    private:
        const SequenceType::Ptr m_targetType;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
