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
#include "qnodebuilder_p.h"
#include "qoutputvalidator_p.h"
#include "qqnamevalue_p.h"

#include "qelementconstructor_p.h"

using namespace Patternist;

ElementConstructor::ElementConstructor(const Expression::Ptr &op1,
                                       const Expression::Ptr &op2) : PairContainer(op1, op2)
{
}

Item ElementConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item name(m_operand1->evaluateSingleton(context));

    const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(m_staticBaseURI));
    const SequenceReceiver::Ptr validator(new OutputValidator(nodeBuilder, context, this));
    const DynamicContext::Ptr receiverContext(context->createReceiverContext(validator));

    nodeBuilder->startElement(name.as<QNameValue>()->qName());
    m_operand2->evaluateToSequenceReceiver(receiverContext);
    nodeBuilder->endElement();

    const NodeModel::Ptr nm(nodeBuilder->builtDocument());
    context->addNodeModel(nm);

    return nm->root(Node());
}

void ElementConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    /* We create an OutputValidator here too. If we're serializing(a common case, unfortunately)
     * the receiver is already validating in order to catch cases where a computed attribute
     * constructor is followed by an element constructor, but in the cases where we're not serializing
     * it's necessary that we validate in this step. */
    const Item name(m_operand1->evaluateSingleton(context));
    const SequenceReceiver::Ptr receiver(context->outputReceiver());
    const SequenceReceiver::Ptr validator(new OutputValidator(receiver, context, this));
    const DynamicContext::Ptr receiverContext(context->createReceiverContext(validator));

    receiver->startElement(name.as<QNameValue>()->qName());
    m_operand2->evaluateToSequenceReceiver(receiverContext);
    receiver->endElement();
}

Expression::Ptr ElementConstructor::typeCheck(const StaticContext::Ptr &context,
                                              const SequenceType::Ptr &reqType)
{
    m_staticBaseURI = context->baseURI();
    return PairContainer::typeCheck(context, reqType);
}

SequenceType::Ptr ElementConstructor::staticType() const
{
    return CommonSequenceTypes::ExactlyOneElement;
}

SequenceType::List ElementConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ExactlyOneQName);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

Expression::Properties ElementConstructor::properties() const
{
    return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
ElementConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
