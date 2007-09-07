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

#include "qatomictype_p.h"
#include "qbuiltintypes_p.h"
#include "qcastas_p.h"
#include "qcommonnamespaces_p.h"
#include "qcommonsequencetypes_p.h"
#include "qfunctionargument_p.h"
#include "qfunctioncall_p.h"
#include "qgenericsequencetype_p.h"
#include "qschematype_p.h"
#include "qschematypefactory_p.h"

#include "qconstructorfunctionsfactory_p.h"

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
