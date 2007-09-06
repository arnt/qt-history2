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

#include <QUrl>

#include "CommonSequenceTypes.h"
#include "NodeBuilder.h"
#include "QNameValue.h"

#include "AttributeConstructor.h"

using namespace Patternist;

AttributeConstructor::AttributeConstructor(const Expression::Ptr &op1,
                                           const Expression::Ptr &op2) : PairContainer(op1, op2)
{
}

QString AttributeConstructor::processValue(const QName name,
                                           const QString &value)
{
    if(name == QName(StandardNamespaces::xml, StandardLocalNames::id))
        return value.simplified();
    else
        return value;
}

Item AttributeConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item nameItem(m_operand1->evaluateSingleton(context));
    const Item content(m_operand2->evaluateSingleton(context));

    const QString value(content ? content.stringValue() : QString());
    const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(QUrl()));
    const QName name(nameItem.as<QNameValue>()->qName());

    nodeBuilder->attribute(name, processValue(name, value));

    const NodeModel::Ptr nm(nodeBuilder->builtDocument());
    context->addNodeModel(nm);
    return nm->root(Node());
}

void AttributeConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    const SequenceReceiver::Ptr receiver(context->outputReceiver());
    const Item nameItem(m_operand1->evaluateSingleton(context));

    const Item content(m_operand2->evaluateSingleton(context));
    const QString value(content ? content.stringValue() : QString());
    const QName name(nameItem.as<QNameValue>()->qName());

    receiver->attribute(name, processValue(name, value));
}

SequenceType::Ptr AttributeConstructor::staticType() const
{
    return CommonSequenceTypes::ExactlyOneAttribute;
}

SequenceType::List AttributeConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ExactlyOneQName);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

Expression::Properties AttributeConstructor::properties() const
{
    return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
AttributeConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID AttributeConstructor::id() const
{
    return IDAttributeConstructor;
}

// vim: et:ts=4:sw=4:sts=4
