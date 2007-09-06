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
#include "Debug.h"
#include "ItemMappingIterator.h"
#include "GenericSequenceType.h"
#include "ListIterator.h"
#include "ParentNodeAxis.h"

#include "AxisStep.h"

using namespace Patternist;

namespace Patternist
{
    /**
     * This operator is needed for the s_whenAxisNodeKindEmpty array. The @c int constructors
     * ensure we invoke another operator| such that we don't get an infinite loop.
     */
    static inline Node::NodeKind operator|(const Node::NodeKind &op1, const Node::NodeKind &op2)
    {
        return static_cast<Node::NodeKind>(int(op1) | int(op2));
    }
}

/**
 * @note The order is significant. It is of the same order as the values in Node::Axis is declared.
 */
const Node::NodeKind AxisStep::s_whenAxisNodeKindEmpty[] =
{
    Node::Attribute|Node::Text|Node::ProcessingInstruction|Node::Comment|Node::Namespace,   // child;
    Node::Attribute|Node::Text|Node::ProcessingInstruction|Node::Comment|Node::Namespace,   // descendant;
    Node::Document|Node::Attribute|Node::Text|Node::ProcessingInstruction|Node::Comment|Node::Namespace,// attribute;
    static_cast<Node::NodeKind>(0),                         // self;
    static_cast<Node::NodeKind>(0),                         // descendant-or-self;
    Node::Document|Node::Attribute|Node::Text|Node::ProcessingInstruction|Node::Comment|Node::Namespace,    // namespace;
    Node::Document,                                         // following;
    Node::Document,                                         // parent;
    Node::Document,                                         // ancestor
    Node::Document|Node::Attribute|Node::Namespace,         // preceding-sibling;
    Node::Document|Node::Attribute|Node::Namespace,         // following-sibling;
    Node::Document,                                         // preceding;
    static_cast<Node::NodeKind>(0)                          // ancestor-or-self;
};

bool AxisStep::isAlwaysEmpty(const Node::Axis axis, const Node::NodeKind nodeKind)
{
    qDebug() << Q_FUNC_INFO << "axis: " << axis << " nodeKind: " << nodeKind;
    qDebug();
    return (s_whenAxisNodeKindEmpty[(1 >> axis) - 1] & nodeKind) != 0;
}

AxisStep::AxisStep(const Node::Axis a,
                   const ItemType::Ptr &nt) : m_axis(a),
                                              m_nodeTest(nt)
{
    Q_ASSERT(m_nodeTest);
}

Item AxisStep::mapToItem(const Item &node,
                              const DynamicContext::Ptr &) const
{
    if(m_nodeTest->itemMatches(node))
        return node;
    else
        return Item();
}

Item::Iterator::Ptr AxisStep::evaluateSequence(const DynamicContext::Ptr &context) const
{
    //qDebug() << Q_FUNC_INFO << (Patternist::Node::axisName(axis()) + QLatin1String("::") + nodeTest()->displayName(context->namePool()));

    /* If we don't have a focus, it's either a bug or our parent isn't a Path
     * that have advanced the focus iterator. Hence, attempt to advance the focus on our own. */
    if(!context->contextItem())
        context->focusIterator()->next();

    Q_ASSERT(context->contextItem());

    const Item::Iterator::Ptr source(context->contextItem().asNode().iterate(m_axis));

    return makeItemMappingIterator<Item>(AxisStep::Ptr(const_cast<AxisStep *>(this)),
                                         source, context);
}

Item AxisStep::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    /* If we don't have a focus, it's either a bug or our parent isn't a Path
     * that have advanced the focus iterator. Hence, attempt to advance the focus on our own. */
    if(!context->contextItem())
        context->focusIterator()->next();

    Q_ASSERT(context->contextItem());

    const Item::Iterator::Ptr it(context->contextItem().asNode().iterate(m_axis));
    Item next(it->next());

    while(next)
    {
        const Item candidate(mapToItem(next, context));

        if(candidate)
            return candidate;
        else
            next = it->next();
    };

    return Item();
}

Expression::Ptr AxisStep::typeCheck(const StaticContext::Ptr &context,
                                    const SequenceType::Ptr &reqType)
{
    if(m_axis == Node::Parent && *m_nodeTest == *BuiltinTypes::node)
    {
        /* We only rewrite parent::node() to ParentNodeAxis. */
        return rewrite(Expression::Ptr(new ParentNodeAxis()), context)->typeCheck(context, reqType);
    }
    /* TODO temporarily disabled
    else if(isAlwaysEmpty(m_axis, static_cast<const AnyNodeType *>(m_nodeTest.get())->nodeKind()))
        return EmptySequence::create(this, context);
        */
    else
        return EmptyContainer::typeCheck(context, reqType);
}

SequenceType::Ptr AxisStep::staticType() const
{
    return makeGenericSequenceType(m_nodeTest,
                                   m_axis == Node::AttributeAxis ? Cardinality::zeroOrOne() : Cardinality::zeroOrMore());
}

SequenceType::List AxisStep::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreNodes);
    return result;
}

Expression::Properties AxisStep::properties() const
{
    return RequiresContextItem | DisableElimination;
}

ItemType::Ptr AxisStep::expectedContextItemType() const
{
    return BuiltinTypes::node;
}

ExpressionVisitorResult::Ptr AxisStep::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Node::Axis AxisStep::axis() const
{
    return m_axis;
}

// vim: et:ts=4:sw=4:sts=4
