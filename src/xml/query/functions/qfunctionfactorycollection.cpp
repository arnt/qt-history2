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

#include "BasicTypesFactory.h"
#include "ConstructorFunctionsFactory.h"
#include "FunctionCall.h"
#include "XPath10CoreFunctions.h"
#include "XPath20CoreFunctions.h"

#include "FunctionFactoryCollection.h"

using namespace Patternist;

// STATIC DATA
static FunctionFactory::Ptr s_XPath20functionFactory(0);
static FunctionFactory::Ptr s_XPath10functionFactory(0);

Expression::Ptr FunctionFactoryCollection::createFunctionCall(const QName name,
                                                              const Expression::List &arguments,
                                                              const StaticContext::Ptr &context,
                                                              const SourceLocationReflection *const r)
{
    const_iterator it;
    const_iterator e(constEnd());
    Expression::Ptr function;

    for(it = constBegin(); it != e; ++it)
    {
        function = (*it)->createFunctionCall(name, arguments, context, r);

        if(function)
            break;
    }

    return function;
}

bool FunctionFactoryCollection::isAvailable(const NamePool::Ptr &np, const QName name, const xsInteger arity)
{
    const_iterator it;
    const_iterator e(constEnd());

    for(it = constBegin(); it != e; ++it)
        if((*it)->isAvailable(np, name, arity))
            return true;

    return false;
}

FunctionSignature::Hash FunctionFactoryCollection::functionSignatures() const
{
    /* We simply grab the function signatures for each library, and
     * put them all in one list. */

    const const_iterator e(constEnd());
    FunctionSignature::Hash result;

    for(const_iterator it(constBegin()); it != e; ++it)
    {
        const FunctionSignature::Hash::const_iterator e2((*it)->functionSignatures().constEnd());
        FunctionSignature::Hash::const_iterator sit((*it)->functionSignatures().constBegin());

        for(; sit != e2; ++sit)
            result.insert(sit.key(), sit.value());
    }

    return result;
}

FunctionSignature::Ptr FunctionFactoryCollection::retrieveFunctionSignature(const NamePool::Ptr &, const QName name)
{
    return functionSignatures().value(name);
}

FunctionFactory::Ptr FunctionFactoryCollection::xpath10Factory()
{
    if(s_XPath10functionFactory)
        return s_XPath10functionFactory;

    s_XPath10functionFactory = FunctionFactory::Ptr(new XPath10CoreFunctions());
    return s_XPath10functionFactory;
}

FunctionFactory::Ptr FunctionFactoryCollection::xpath20Factory(const NamePool::Ptr &np)
{
    if(s_XPath20functionFactory)
        return s_XPath20functionFactory;

    const FunctionFactoryCollection::Ptr fact(new FunctionFactoryCollection());
    fact->append(xpath10Factory());
    fact->append(FunctionFactory::Ptr(new XPath20CoreFunctions()));
    fact->append(FunctionFactory::Ptr(
                            new ConstructorFunctionsFactory(np, BasicTypesFactory::self(np))));
    s_XPath20functionFactory = fact;

    return s_XPath20functionFactory;
}

// vim: et:ts=4:sw=4:sts=4
