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

#include "AnyURI.h"
#include "BuiltinTypes.h"
#include "CommonValues.h"
#include "ListIterator.h"
#include "PatternistLocale.h"
#include "NodeNamespaceResolver.h"
#include "QNameConstructor.h"
#include "QNameValue.h"
#include "AtomicString.h"
#include "XPathHelper.h"

#include "QNameFNs.h"

using namespace Patternist;

Item QNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item paramURI(m_operands.first()->evaluateSingleton(context));
    const QString paramQName(m_operands.last()->evaluateSingleton(context).stringValue());

    QString ns;
    if(paramURI)
        ns = paramURI.stringValue();

    if(!XPathHelper::isQName(paramQName))
    {
        context->error(tr("%1 is an invalid %2").arg(formatData(paramQName),
                                                     formatType(context->namePool(), BuiltinTypes::xsQName)),
                       ReportContext::FOCA0002, this);
        return Item();
    }

    QString prefix;
    QString lname;
    XPathHelper::splitQName(paramQName, prefix, lname);
    const QName n(context->namePool()->allocateQName(ns, lname, prefix));

    if(ns.isEmpty())
    {
        if(prefix.isEmpty())
            return toItem(QNameValue::fromValue(context->namePool(), n));
        else
        {
            context->error(tr("If the first argument is the empty sequence or "
                              "a zero-length string(no namespace), a prefix "
                              "cannot be specified. The prefix %1 was specified")
                              .arg(formatKeyword(prefix)),
                           ReportContext::FOCA0002, this);
            return Item(); /* Silence compiler warning. */
        }
    }
    else
        return toItem(QNameValue::fromValue(context->namePool(), n));
}

Item ResolveQNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item itemName(m_operands.first()->evaluateSingleton(context));

    if(!itemName)
        return Item();

    const NamespaceResolver::Ptr resolver(new NodeNamespaceResolver(m_operands.last()->evaluateSingleton(context)));
    const QString strName(itemName.stringValue());
    const QName name = QNameConstructor::expandQName<DynamicContext::Ptr,
                                                     ReportContext::FOCA0002,
                                                     ReportContext::FONS0004>(strName,
                                                                              context,
                                                                              resolver,
                                                                              this);

    return toItem(QNameValue::fromValue(context->namePool(), name));
}

Item PrefixFromQNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QNameValue::Ptr arg(m_operands.first()->evaluateSingleton(context).as<QNameValue>());
    if(!arg)
        return Item();

    const QString prefix(context->namePool()->stringForPrefix(arg->qName().prefix()));

    if(prefix.isEmpty())
        return Item();
    else
        return AtomicString::fromValue(context->namePool()->stringForPrefix(arg->qName().prefix()));
}

Item LocalNameFromQNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QNameValue::Ptr arg(m_operands.first()->evaluateSingleton(context).as<QNameValue>());
    return arg ? toItem(AtomicString::fromValue(context->namePool()->stringForLocalName(arg->qName().localName()))) : Item();
}

Item NamespaceURIFromQNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QNameValue::Ptr arg(m_operands.first()->evaluateSingleton(context).as<QNameValue>());
    return arg ? toItem(AnyURI::fromValue(context->namePool()->stringForNamespace(arg->qName().namespaceURI()))) : Item();
}

Item NamespaceURIForPrefixFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item prefixItem(m_operands.first()->evaluateSingleton(context));
    QName::PrefixCode prefix;

    if(prefixItem)
        prefix = context->namePool()->allocatePrefix(prefixItem.stringValue());
    else
        prefix = StandardPrefixes::empty;

    const Item eleItem(m_operands.last()->evaluateSingleton(context));
    Q_ASSERT(eleItem);

    const QName::NamespaceCode ns = eleItem.asNode().namespaceForPrefix(prefix);
    //qDebug() << "ns:" << ns << "nsStr:" << context->namePool()->stringForNamespace(ns);

    if(ns == NamespaceResolver::NoBinding)
    {
        /* This is a bit tricky. The default namespace is not considered an in-scope binding
         * on a node, but the specification for this function do consider it a binding and therefore
         * the empty string. */
        if(prefix == StandardPrefixes::empty)
            return CommonValues::EmptyString;
        else
            return Item();
    }
    else
        return toItem(AnyURI::fromValue(context->namePool()->stringForNamespace(ns)));
}

Item::Iterator::Ptr InScopePrefixesFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item e(m_operands.first()->evaluateSingleton(context));

    const NamespaceBinding::Vector nbs(e.asNode().namespaceBindings());
    const int len = nbs.size();
    const NamePool::Ptr np(context->namePool());

    QList<Item> result;

    for(int i = 0; i < len; ++i)
        result.append(AtomicString::fromValue(np->stringForPrefix(nbs.at(i).prefix())));

    return makeListIterator(result);
}

// vim: et:ts=4:sw=4:sts=4
