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

#include "CommonSequenceTypes.h"
#include "TypeChecker.h"

#include "TreatAs.h"

using namespace Patternist;

TreatAs::TreatAs(const Expression::Ptr &operand,
                 const SequenceType::Ptr &reqType) : SingleContainer(operand),
                                                     m_reqType(reqType)
{
    Q_ASSERT(reqType);
}

Expression::Ptr TreatAs::typeCheck(const StaticContext::Ptr &context,
                                   const SequenceType::Ptr &reqType)
{
    Q_ASSERT(context);
    Q_ASSERT(reqType);
    const Expression::Ptr treated(TypeChecker::applyFunctionConversion(m_operand,
                                                                       m_reqType,
                                                                       context,
                                                                       ReportContext::XPDY0050));
    return treated->typeCheck(context, reqType);
}

ExpressionVisitorResult::Ptr TreatAs::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

SequenceType::Ptr TreatAs::staticType() const
{
    return m_reqType;
}

SequenceType::List TreatAs::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

// vim: et:ts=4:sw=4:sts=4
