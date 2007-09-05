/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include "CommonSequenceTypes.h"
#include "PatternistLocale.h"
#include "QNameValue.h"
#include "AtomicString.h"

#include "ErrorFN.h"

using namespace Patternist;

Item ErrorFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    QString msg;

    switch(m_operands.count())
    {
        case 0: /* No args. */
        {
            context->error(tr("%1 was called.").arg(formatFunction(context->namePool(), signature())),
                            ReportContext::FOER0000, this);
            return Item();
        }
        case 3:
        /* Fallthrough, we don't use the 'error object' param. */
        case 2:
            msg = m_operands.at(1)->evaluateSingleton(context).stringValue();
        /* Fall through. */
        case 1:
        {
            const QNameValue::Ptr qName(m_operands.first()->evaluateSingleton(context).as<QNameValue>());

            if(qName)
                context->error(msg, qName->qName(), this);
            else
                context->error(msg, ReportContext::FOER0000, this);

            return Item();
        }
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "Invalid number of arguments passed to fn:error.");
            return Item();
        }
    }
}

FunctionSignature::Ptr ErrorFN::signature() const
{
    const FunctionSignature::Ptr e(FunctionCall::signature());

    if(m_operands.count() != 1)
        return e;

    FunctionSignature::Ptr nev(FunctionSignature::Ptr(new FunctionSignature(e->name(),
                                                      e->minimumArguments(),
                                                      e->maximumArguments(),
                                                      e->returnType(),
                                                      e->properties())));
    const FunctionArgument::List args(e->arguments());
    FunctionArgument::List nargs;
    const QName argName(StandardNamespaces::empty, StandardLocalNames::error);
    nargs.append(FunctionArgument::Ptr(new FunctionArgument(argName, CommonSequenceTypes::ExactlyOneQName)));
    nargs.append(args[1]);
    nargs.append(args[2]);
    nev->setArguments(nargs);

    return nev;
}

// vim: et:ts=4:sw=4:sts=4
