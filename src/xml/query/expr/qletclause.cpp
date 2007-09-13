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

#include "qboolean_p.h"
#include "qcommonsequencetypes_p.h"
#include "qdynamiccontextstore_p.h"
#include "qliteral_p.h"

#include "qletclause_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

LetClause::LetClause(const Expression::Ptr &operand1,
                     const Expression::Ptr &operand2,
                     const VariableDeclaration::Ptr &decl) : PairContainer(operand1, operand2)
                                                           , m_varDecl(decl)
{
    Q_ASSERT(m_varDecl);
}

DynamicContext::Ptr LetClause::bindVariable(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    context->setExpressionVariable(m_varDecl->slot, Expression::Ptr(new DynamicContextStore(m_operand1, context)));
    return context;
}

Item::Iterator::Ptr LetClause::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return m_operand2->evaluateSequence(bindVariable(context));
}

Item LetClause::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return m_operand2->evaluateSingleton(bindVariable(context));
}

bool LetClause::evaluateEBV(const DynamicContext::Ptr &context) const
{
    return m_operand2->evaluateEBV(bindVariable(context));
}

void LetClause::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    m_operand2->evaluateToSequenceReceiver(bindVariable(context));
}

Expression::Ptr LetClause::typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType)
{
    qDebug() << Q_FUNC_INFO;

    /* Consider the following query:
     *
     * <tt>let $d := \<child type=""/>
     * return $d//\*[let $i := @type
     *              return $d//\*[$i]]</tt>
     *
     * The node test <tt>@type</tt> is referenced from two different places,
     * where each reference have a different focus. So, in the case of that the source
     * uses the focus, we need to use a DynamicContextStore to ensure the variable
     * is always evaluated with the correct focus, regardless of where it is referenced
     * from.
     *
     * We miss out a lot of false positives. For instance, the case of where the focus
     * is identical for everyone. One reason we cannot check this, is that Expression
     * doesn't know about its parent.
     */
    qDebug() << "has flag set?" <<  m_operand1->deepProperties().testFlag(RequiresFocus);
    m_varDecl->canSourceRewrite = !m_operand1->deepProperties().testFlag(RequiresFocus);

    if(m_varDecl->canSourceRewrite)
        return m_operand2->typeCheck(context, reqType);
    else
        return PairContainer::typeCheck(context, reqType);
}

SequenceType::List LetClause::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

SequenceType::Ptr LetClause::staticType() const
{
    return m_operand2->staticType();
}

ExpressionVisitorResult::Ptr LetClause::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
