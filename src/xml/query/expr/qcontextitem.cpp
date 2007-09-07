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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"

#include "qcontextitem_p.h"

using namespace Patternist;

Item ContextItem::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return context->contextItem();
}

Expression::Ptr ContextItem::compress(const StaticContext::Ptr &context)
{
    m_itemType = context->contextItemType();
    return EmptyContainer::compress(context);
}

Expression::Ptr ContextItem::typeCheck(const StaticContext::Ptr &context,
                                       const SequenceType::Ptr &reqType)
{
    m_itemType = context->contextItemType();
    return EmptyContainer::typeCheck(context, reqType);
}

SequenceType::Ptr ContextItem::staticType() const
{
    /* We test m_itemType here because Patternist View calls staticType() before the typeCheck()
     * stage. */
    if(m_itemType)
        return makeGenericSequenceType(m_itemType, Cardinality::exactlyOne());
    else
        return CommonSequenceTypes::ExactlyOneItem;
}

Expression::Properties ContextItem::properties() const
{
    return DisableElimination | RequiresContextItem;
}

ExpressionVisitorResult::Ptr ContextItem::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID ContextItem::id() const
{
    return IDContextItem;
}

ItemType::Ptr ContextItem::expectedContextItemType() const
{
    return BuiltinTypes::item;
}

const SourceLocationReflection *ContextItem::actualReflection() const
{
    if(m_expr)
        return m_expr.get();
    else
        return this;
}

// vim: et:ts=4:sw=4:sts=4
