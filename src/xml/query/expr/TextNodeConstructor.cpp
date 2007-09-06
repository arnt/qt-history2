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

#include <QUrl>

#include "CommonSequenceTypes.h"
#include "NodeBuilder.h"

#include "TextNodeConstructor.h"

using namespace Patternist;

TextNodeConstructor::TextNodeConstructor(const Expression::Ptr &op) : SingleContainer(op)
{
}

Item TextNodeConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item chars(m_operand->evaluateSingleton(context));

    if(!chars)
        return Item();

    const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(QUrl()));
    nodeBuilder->characters(chars.stringValue());

    const NodeModel::Ptr nm(nodeBuilder->builtDocument());
    context->addNodeModel(nm);

    return nm->root(Node());
}

void TextNodeConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    const Item item(m_operand->evaluateSingleton(context));

    if(item)
    {
        const SequenceReceiver::Ptr receiver(context->outputReceiver());
        receiver->characters(item.stringValue());
    }
}

SequenceType::Ptr TextNodeConstructor::staticType() const
{
    if(m_operand->staticType()->cardinality().allowsEmpty())
        return CommonSequenceTypes::ZeroOrOneTextNode;
    else
        return CommonSequenceTypes::ExactlyOneTextNode;
}

SequenceType::List TextNodeConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrOneString);
    return result;
}

Expression::Properties TextNodeConstructor::properties() const
{
    return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
TextNodeConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
