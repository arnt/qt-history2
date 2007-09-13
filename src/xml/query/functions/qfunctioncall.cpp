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

#include <QtDebug>

#include "qcontextitem_p.h"
#include "qcommonsequencetypes_p.h"
#include "qemptysequence_p.h"
#include "qfunctionsignature_p.h"
#include "qgenericsequencetype_p.h"
#include "qcollationchecker_p.h"
#include "qcommonnamespaces_p.h"

#include "qfunctioncall_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

SequenceType::List FunctionCall::expectedOperandTypes() const
{
    const FunctionArgument::List args(signature()->arguments());
    FunctionArgument::List::const_iterator it(args.constBegin());
    const FunctionArgument::List::const_iterator end(args.constEnd());
    // TODO reserve/resize()
    SequenceType::List result;

    for(; it != end; ++it)
        result.append((*it)->type());

    return result;
}

Expression::Ptr FunctionCall::typeCheck(const StaticContext::Ptr &context,
                                        const SequenceType::Ptr &reqType)
{
    /* We don't cache properties() at some stages because it can be invalidated
     * by the typeCheck(). */

    const FunctionSignature::Arity maxArgs = signature()->maximumArguments();
    /* We do this before the typeCheck() such that the appropriate conversions
     * are applied to the ContextItem. */
    if(m_operands.count() < maxArgs &&
       has(UseContextItem))
    {
        m_operands.append(Expression::Ptr(new ContextItem()));
        context->addLocation(m_operands.last().get(), context->locationFor(this));
    }

    const Expression::Ptr me(UnlimitedContainer::typeCheck(context, reqType));
    if(me.get() != this)
        return me;

    const Properties props(properties());

    if((props & RewriteToEmptyOnEmpty) == RewriteToEmptyOnEmpty &&
       *CommonSequenceTypes::Empty == *m_operands.first()->staticType()->itemType())
    {
        return EmptySequence::create(this, context);
    }

    if((props & LastOperandIsCollation) == LastOperandIsCollation &&
       m_operands.count() == maxArgs)
    {
        m_operands.last() = Expression::Ptr(new CollationChecker(m_operands.last()));
        context->addLocation(m_operands.last().get(), context->locationFor(this));
    }

    return me;
}

void FunctionCall::setSignature(const FunctionSignature::Ptr &sign)
{
    m_signature = sign;
}

FunctionSignature::Ptr FunctionCall::signature() const
{
    Q_ASSERT(m_signature); /* It really should be set. */
    return m_signature;
}

SequenceType::Ptr FunctionCall::staticType() const
{
    Q_ASSERT(m_signature);
    if(has(EmptynessFollowsChild))
    {
        if(m_operands.isEmpty())
        {
            /* This is a function which uses the context item when having no arguments. */
            return signature()->returnType();
        }
        const Cardinality card(m_operands.first()->staticType()->cardinality());
        if(card.allowsEmpty())
            return signature()->returnType();
        else
        {
            /* Remove empty. */
            return makeGenericSequenceType(signature()->returnType()->itemType(),
                                           card & Cardinality::oneOrMore());
        }
    }
    return signature()->returnType();
}

Expression::Properties FunctionCall::properties() const
{
    Q_ASSERT(m_signature);
    return signature()->properties();
}

ExpressionVisitorResult::Ptr FunctionCall::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID FunctionCall::id() const
{
    Q_ASSERT(m_signature);
    return m_signature->id();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
