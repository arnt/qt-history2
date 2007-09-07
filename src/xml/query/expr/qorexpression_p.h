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

#ifndef Patternist_OrExpression_H
#define Patternist_OrExpression_H

#include "qandexpression_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements XPath 2.0's logical expression @c or.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-logical-expressions">XML Path Language
     * (XPath) 2.0, 3.6 Logical Expressions</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class OrExpression : public AndExpression
    {
    public:
        OrExpression(const Expression::Ptr &operand1,
                     const Expression::Ptr &operand2);

        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

        virtual Expression::Ptr compress(const StaticContext::Ptr &context);
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
