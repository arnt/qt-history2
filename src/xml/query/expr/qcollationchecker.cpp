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

#include "qcommonsequencetypes_p.h"
#include "qxpathhelper_p.h"

#include "qcollationchecker_p.h"

using namespace Patternist;

CollationChecker::CollationChecker(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item CollationChecker::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item val(m_operand->evaluateSingleton(context));
    XPathHelper::checkCollationSupport<ReportContext::FOCH0002>(val.stringValue(), context, this);
    return val;
}

SequenceType::List CollationChecker::expectedOperandTypes() const
{
    SequenceType::List list;
    list.append(CommonSequenceTypes::ExactlyOneString);
    return list;
}

SequenceType::Ptr CollationChecker::staticType() const
{
    return m_operand->staticType();
}

ExpressionVisitorResult::Ptr CollationChecker::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
