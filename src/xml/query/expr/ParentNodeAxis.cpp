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

#include "BuiltinTypes.h"
#include "CommonSequenceTypes.h"

#include "ParentNodeAxis.h"

using namespace Patternist;

ParentNodeAxis::ParentNodeAxis()
{
}

Item ParentNodeAxis::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(context->contextItem());
    return item.asNode().parent();
}

Expression::Properties ParentNodeAxis::properties() const
{
    return DisableElimination | RequiresContextItem;
}

ExpressionVisitorResult::Ptr ParentNodeAxis::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

ItemType::Ptr ParentNodeAxis::expectedContextItemType() const
{
    return BuiltinTypes::node;
}

SequenceType::Ptr ParentNodeAxis::staticType() const
{
    // Parentless node exists.
    return CommonSequenceTypes::ZeroOrOneNode;
}

// vim: et:ts=4:sw=4:sts=4
