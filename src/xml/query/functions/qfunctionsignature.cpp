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

#include "qqname_p.h"

#include "qfunctionsignature_p.h"

using namespace Patternist;

FunctionSignature::FunctionSignature(const QName nameP,
                                     const Arity minArgs,
                                     const Arity maxArgs,
                                     const SequenceType::Ptr &returnTypeP,
                                     const Expression::Properties props,
                                     const Expression::ID idP) : m_name(nameP),
                                                                 m_minArgs(minArgs),
                                                                 m_maxArgs(maxArgs),
                                                                 m_returnType(returnTypeP),
                                                                 m_arguments(),
                                                                 m_props(props),
                                                                 m_id(idP)
{
    Q_ASSERT(!nameP.isNull());
    Q_ASSERT(minArgs <= maxArgs || maxArgs == FunctionSignature::UnlimitedArity);
    Q_ASSERT(m_maxArgs >= -1);
    Q_ASSERT(returnTypeP);
}

void FunctionSignature::appendArgument(const QName::LocalNameCode nameP,
                                       const SequenceType::Ptr &type)
{
    Q_ASSERT(type);

    m_arguments.append(FunctionArgument::Ptr(new FunctionArgument(QName(StandardNamespaces::empty, nameP), type)));
}

QString FunctionSignature::displayName(const NamePool::Ptr &np) const
{
    QString result;
    result += np->displayName(m_name);
    result += QLatin1Char('(');

    FunctionArgument::List::const_iterator it(m_arguments.constBegin());
    const FunctionArgument::List::const_iterator end(m_arguments.constEnd());

    if(it != end)
    {
        while(true)
        {
            result += QLatin1Char('$');
            result += np->displayName((*it)->name());
            result += QLatin1String(" as ");
            result += (*it)->type()->displayName(np);

            ++it;
            if(it == end)
                break;

            result += QLatin1String(", ");
        }
    }

    if(m_maxArgs == FunctionSignature::UnlimitedArity)
        result += QLatin1String(", ...");

    result += QLatin1String(") as ");
    result += m_returnType->displayName(np);

    return result;
}

bool FunctionSignature::operator==(const FunctionSignature &other) const
{
    return name() == other.name() &&
           isArityValid(other.maximumArguments()) &&
           isArityValid(other.minimumArguments());
}

void FunctionSignature::setArguments(const FunctionArgument::List &args)
{
    m_arguments = args;
}

FunctionArgument::List FunctionSignature::arguments() const
{
    return m_arguments;
}

bool FunctionSignature::isArityValid(const xsInteger arity) const
{
    return arity >= m_minArgs && arity <= m_maxArgs;
}

QName FunctionSignature::name() const
{
    return m_name;
}

FunctionSignature::Arity FunctionSignature::minimumArguments() const
{
    return m_minArgs;
}

FunctionSignature::Arity FunctionSignature::maximumArguments() const
{
    return m_maxArgs;
}

SequenceType::Ptr FunctionSignature::returnType() const
{
    return m_returnType;
}

Expression::Properties FunctionSignature::properties() const
{
    return m_props;
}

Expression::ID FunctionSignature::id() const
{
    return m_id;
}

// vim: et:ts=4:sw=4:sts=4
