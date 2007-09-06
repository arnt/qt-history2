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

#include "CommonSequenceTypes.h"
#include "PatternistLocale.h"
#include "NodeBuilder.h"

#include "CommentConstructor.h"

using namespace Patternist;

CommentConstructor::CommentConstructor(const Expression::Ptr &op) : SingleContainer(op)
{
}

QString CommentConstructor::evaluateContent(const DynamicContext::Ptr &context) const
{
    const Item item(m_operand->evaluateSingleton(context));

    if(!item)
        return QString();

    const QString content(item.stringValue());

    if(content.contains(QLatin1String("--")))
    {
        context->error(tr("The content of a comment cannot contain %1").arg(formatData("--")),
                       ReportContext::XQDY0072, this);
    }
    else if(content.endsWith(QLatin1Char('-')))
    {
        context->error(tr("The content of a comment cannot end with a %1.").arg(formatData(QLatin1Char('-'))),
                       ReportContext::XQDY0072, this);
    }

    return content;
}

Item CommentConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QString content(evaluateContent(context));
    const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(QUrl()));
    nodeBuilder->comment(content);

    const NodeModel::Ptr nm(nodeBuilder->builtDocument());
    context->addNodeModel(nm);

    return nm->root(Node());
}

void CommentConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    const QString content(evaluateContent(context));
    const SequenceReceiver::Ptr receiver(context->outputReceiver());

    receiver->comment(content);
}

SequenceType::Ptr CommentConstructor::staticType() const
{
    return CommonSequenceTypes::ExactlyOneComment;
}

SequenceType::List CommentConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrOneString);
    return result;
}

Expression::Properties CommentConstructor::properties() const
{
    return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
CommentConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
