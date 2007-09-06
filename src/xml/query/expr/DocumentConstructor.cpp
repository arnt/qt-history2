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
#include "DocumentContentValidator.h"
#include "NodeBuilder.h"

#include "DocumentConstructor.h"

using namespace Patternist;

DocumentConstructor::DocumentConstructor(const Expression::Ptr &op) : SingleContainer(op)
{
}

Item DocumentConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(m_staticBaseURI));
    const SequenceReceiver::Ptr validator(new DocumentContentValidator(nodeBuilder, context, Expression::Ptr(const_cast<DocumentConstructor *>(this))));
    const DynamicContext::Ptr receiverContext(context->createReceiverContext(validator));

    nodeBuilder->startDocument();
    m_operand->evaluateToSequenceReceiver(receiverContext);
    nodeBuilder->endDocument();

    const NodeModel::Ptr nm(nodeBuilder->builtDocument());
    context->addNodeModel(nm);

    return nm->root(Node());
}

void DocumentConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    const SequenceReceiver::Ptr receiver(context->outputReceiver());
    const SequenceReceiver::Ptr validator(new DocumentContentValidator(receiver, context, Expression::Ptr(const_cast<DocumentConstructor *>(this))));
    const DynamicContext::Ptr receiverContext(context->createReceiverContext(validator));

    receiver->startDocument();
    m_operand->evaluateToSequenceReceiver(receiverContext);
    receiver->endDocument();
}

Expression::Ptr DocumentConstructor::typeCheck(const StaticContext::Ptr &context,
                                               const SequenceType::Ptr &reqType)
{
    m_staticBaseURI = context->baseURI();
    return SingleContainer::typeCheck(context, reqType);
}

SequenceType::Ptr DocumentConstructor::staticType() const
{
    return CommonSequenceTypes::ExactlyOneDocumentNode;
}

SequenceType::List DocumentConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

Expression::Properties DocumentConstructor::properties() const
{
    return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
DocumentConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
