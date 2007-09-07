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
#include "qdebug_p.h"
#include "qdynamiccontextstore_p.h"
#include "qevaluationcache_p.h"
#include "qlistiterator_p.h"
#include "quserfunction_p.h"

#include "quserfunctioncallsite_p.h"

using namespace Patternist;

UserFunctionCallsite::UserFunctionCallsite(const QName nameP,
                                           const FunctionSignature::Arity ar) : m_name(nameP),
                                                                                m_arity(ar),
                                                                                m_slotOffset(-2),
                                                                                m_isRecursive(false)

{
    qDebug() << Q_FUNC_INFO << "arity: " << ar;
    Q_ASSERT(!m_name.isNull());
}

Item::Iterator::Ptr UserFunctionCallsite::evaluateSequence(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    return m_body->evaluateSequence(bindVariables(context));
}

Item UserFunctionCallsite::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    return m_body->evaluateSingleton(bindVariables(context));
}

bool UserFunctionCallsite::evaluateEBV(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    return m_body->evaluateEBV(bindVariables(context));
}

void UserFunctionCallsite::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    m_body->evaluateToSequenceReceiver(bindVariables(context));
}

DynamicContext::Ptr UserFunctionCallsite::bindVariables(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const DynamicContext::Ptr stackContext(context->createStack());
    Q_ASSERT(stackContext);

    const Expression::List::const_iterator end(m_operands.constEnd());
    Expression::List::const_iterator it(m_operands.constBegin());

    VariableSlotID slot = m_slotOffset;

    /* We only need the DynamicContextStore if we're recursive. */
    if(m_isRecursive)
    {
        for(; it != end; ++it)
        {
            qDebug() << "Setting slot: " << slot;
            stackContext->setExpressionVariable(slot,
                                                Expression::Ptr(new DynamicContextStore(*it, context)));
            ++slot;
        }
    }
    else
    {
        for(; it != end; ++it)
        {
            qDebug() << "Setting slot: " << slot;
            //stackContext->setExpressionVariable(slot, *it);
            stackContext->setExpressionVariable(slot,
                                                Expression::Ptr(new DynamicContextStore(*it, context)));
            ++slot;
        }
    }

    return stackContext;
}

SequenceType::List UserFunctionCallsite::expectedOperandTypes() const
{
    qDebug() << Q_FUNC_INFO;
    SequenceType::List result;

    if(m_signature)
    {
        qDebug() << Q_FUNC_INFO << "Have signature.. ";
        const FunctionArgument::List args(m_signature->arguments());
        const FunctionArgument::List::const_iterator end(args.constEnd());
        FunctionArgument::List::const_iterator it(args.constBegin());

        for(; it != end; ++it)
            result.append((*it)->type());
    }
    else
        result.append(CommonSequenceTypes::ZeroOrMoreItems);

    return result;
}

Expression::Ptr UserFunctionCallsite::typeCheck(const StaticContext::Ptr &context,
                                                const SequenceType::Ptr &reqType)
{
    qDebug() << Q_FUNC_INFO << "m_myArity: " << m_arity << " m_isRecursive: " << m_isRecursive;
    /* Ensure that the return value of the function is properly
     * converted/does match from where it is called(which is here). */
    if(m_isRecursive)
        return UnlimitedContainer::typeCheck(context, reqType);
    else
    {
        m_body = m_body->typeCheck(context, reqType);

        /* We just act as a pipe for m_body, so we don't have to typecheck ourselves. However,
         * the arguments must match the function declaration. */
        typeCheckOperands(context);
        return Expression::Ptr(this);
    }
}

Expression::Properties UserFunctionCallsite::properties() const
{
    return DisableElimination;
}

SequenceType::Ptr UserFunctionCallsite::staticType() const
{
    /* Our return type, is the static type of the function body. We could have also used
     * m_signature->returnType(), but it doesn't get updated when function conversion is
     * applied.
     * We can't use m_body's type if we're recursive, because m_body computes its type
     * from its children, and we're at least one of the children. Hence, we would
     * recurse infinitely if we did.
     *
     * m_body can be null here if we're called before setSource().
     */
    if(m_isRecursive || !m_body)
        return CommonSequenceTypes::ZeroOrMoreItems; // TODO use the declaration, it can have a type explicitly.
    else
        return m_body->staticType();
}

ExpressionVisitorResult::Ptr UserFunctionCallsite::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID UserFunctionCallsite::id() const
{
    return IDUserFunctionCallsite;
}

Expression::Ptr UserFunctionCallsite::body() const
{
    return m_body;
}

bool UserFunctionCallsite::isSignatureValid(const FunctionSignature::Ptr &sign) const
{
    Q_ASSERT(sign);

    return sign->name() == m_name
           &&
           sign->isArityValid(m_arity);
}

bool UserFunctionCallsite::configureRecursion(const FunctionSignature::Ptr &sign)
{
    Q_ASSERT(sign);

    m_isRecursive = isSignatureValid(sign);
    qDebug();
    return m_isRecursive;
}

void UserFunctionCallsite::setSource(const Expression::Ptr &bodyP,
                                     const FunctionSignature::Ptr &sign,
                                     const VariableSlotID slotOffset,
                                     const VariableDeclaration::List &varDecls)
{
    qDebug() << Q_FUNC_INFO << "slotOffset: " << slotOffset;
    Q_ASSERT(bodyP);
    Q_ASSERT(sign);
    Q_ASSERT(slotOffset > -1);

    m_body = bodyP;
    m_signature = sign;
    m_slotOffset = slotOffset;

    const int len = m_operands.size();

    for(int i = 0; i < len; ++i)
    {
        /* We don't want evaluation caches for range variables, it's not necessary since
         * the item is already cached in DynamicContext::rangeVariable(). */
        if(m_operands.at(i)->is(IDRangeVariableReference))
            continue;

        m_operands[i] = Expression::Ptr(new EvaluationCache(m_operands.at(i),
                                                            varDecls.at(i),
                                                            varDecls.at(i)->slot));
    }
}

FunctionSignature::Arity UserFunctionCallsite::arity() const
{
    return m_arity;
}

QName UserFunctionCallsite::name() const
{
    return m_name;
}

FunctionSignature::Ptr UserFunctionCallsite::signature() const
{
    return m_signature;
}

// vim: et:ts=4:sw=4:sts=4
