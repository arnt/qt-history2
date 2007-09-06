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

#include "CommonNamespaces.h"
#include "CommonSequenceTypes.h"
#include "FunctionAvailableFN.h"
#include "SystemPropertyFN.h"

#include "XSLT10CoreFunctions.h"

using namespace Patternist;

Expression::Ptr XSLT10CoreFunctions::retrieveExpression(const QName lname,
                                                        const Expression::List &args,
                                                        const FunctionSignature::Ptr &sign) const
{
    Q_ASSERT(sign);

    Expression::Ptr fn;
#define testXSLTFN(ln, cname) else if(lname.localName() == StandardLocalNames::ln) fn = Expression::Ptr(new cname())

    if(false) /* Dummy for the macro handling. Will be optimized away anyway. */
        return Expression::Ptr();
    /* Alphabetic order. */
    testXSLTFN(function_available,  FunctionAvailableFN);
    testXSLTFN(system_property,     SystemPropertyFN);
#undef testXSLTFN

    Q_ASSERT(fn);
    fn->setOperands(args);
    fn->as<FunctionCall>()->setSignature(sign);

    return fn;
}

FunctionSignature::Ptr XSLT10CoreFunctions::retrieveFunctionSignature(const NamePool::Ptr &np, const QName name)
{
    if(StandardNamespaces::fn != name.namespaceURI())
        return FunctionSignature::Ptr();

    FunctionSignature::Ptr s(functionSignatures().value(name));

    if(!s)
    {
        /* Alphabetic order. */
        if(name.localName() == StandardLocalNames::function_available)
        {
            s = addFunction(StandardLocalNames::function_available, 1, 2, CommonSequenceTypes::ExactlyOneBoolean);
            s->appendArgument(argument(np, "function_name"), CommonSequenceTypes::ExactlyOneString);
            s->appendArgument(argument(np, "arity"), CommonSequenceTypes::ExactlyOneInteger);
        }
        else if(name.localName() == StandardLocalNames::system_property)
        {
            s = addFunction(StandardLocalNames::system_property, 1, 1, CommonSequenceTypes::ExactlyOneString);
            s->appendArgument(argument(np, "property_name"), CommonSequenceTypes::ExactlyOneString);
        }
    }

    return s;
}

// vim: et:ts=4:sw=4:sts=4
