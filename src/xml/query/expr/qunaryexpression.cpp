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

#include "qarithmeticexpression_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"

#include "qunaryexpression_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

Expression::Ptr UnaryExpression::create(const AtomicMathematician::Operator op,
                                        const Expression::Ptr &expr,
                                        const StaticContext::Ptr &context)
{
    Q_ASSERT(op == AtomicMathematician::Substract ||
             op == AtomicMathematician::Add);
    Q_ASSERT(expr);
    Q_ASSERT(context);

    return Expression::Ptr(new ArithmeticExpression(wrapLiteral(CommonValues::IntegerZero, context, expr.get()), op, expr));
}


// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
