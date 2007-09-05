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

#include "AtomicType.h"
#include "BuiltinTypes.h"
#include "CastAs.h"
#include "CommonNamespaces.h"
#include "CommonSequenceTypes.h"
#include "FunctionArgument.h"
#include "FunctionCall.h"
#include "GenericSequenceType.h"
#include "SchemaType.h"
#include "SchemaTypeFactory.h"

#include "ConstructorFunctionsFactory.h"

using namespace Patternist;

ConstructorFunctionsFactory::ConstructorFunctionsFactory(const NamePool::Ptr &np, const SchemaTypeFactory::Ptr &f) : m_typeFactory(f)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_typeFactory);
    Q_ASSERT(np);
    SchemaType::Hash::const_iterator it(m_typeFactory->types().constBegin());
    const SchemaType::Hash::const_iterator end(m_typeFactory->types().constEnd());

    FunctionArgument::List args;
    const QName argName(StandardNamespaces::empty, StandardLocalNames::sourceValue);

    args.append(FunctionArgument::Ptr(new FunctionArgument(argName,
                                                           CommonSequenceTypes::ZeroOrOneAtomicType)));

    while(it != end)
    {
        if(!BuiltinTypes::xsAnyAtomicType->wxsTypeMatches(*it) ||
           *BuiltinTypes::xsAnyAtomicType == *static_cast<const AtomicType *>((*it).get()) ||
           *BuiltinTypes::xsNOTATION == *static_cast<const AtomicType *>((*it).get()))
        {
            /* It's not a valid type for a constructor function -- skip it. */
            ++it;
            continue;
        }

        const QName name((*it)->name(np));
        FunctionSignature::Ptr s(new FunctionSignature(name, 1, 1,
                                                       makeGenericSequenceType(AtomicType::Ptr(*it),
                                                                               Cardinality::zeroOrOne())));
        s->setArguments(args);
        m_signatures.insert(name, s);
        ++it;
    }
}

Expression::Ptr ConstructorFunctionsFactory::retrieveExpression(const QName name,
                                                                const Expression::List &args,
                                                                const FunctionSignature::Ptr &sign) const
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(sign);

    /* This function is only called if the callsite is valid, so createSchemaType() will always
     * return an AtomicType. */
    const AtomicType::Ptr at(static_cast<AtomicType *>(m_typeFactory->createSchemaType(name).get()));

    return Expression::Ptr(new CastAs(args.first(),
                                      makeGenericSequenceType(at,
                                                              Cardinality::zeroOrOne())));
}

FunctionSignature::Ptr ConstructorFunctionsFactory::retrieveFunctionSignature(const NamePool::Ptr &np, const QName name)
{
    Q_UNUSED(np);
    qDebug() << Q_FUNC_INFO << "contains name?" << functionSignatures().value(name);
    return functionSignatures().value(name);
}

// vim: et:ts=4:sw=4:sts=4
