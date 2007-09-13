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

#include "qabstractfloat_p.h"
#include "qanyuri_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonnamespaces_p.h"
#include "qcommonvalues_p.h"
#include "qdebug_p.h"
#include "qliteral_p.h"
#include "qatomicstring_p.h"

#include "qnodefns_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

Item NameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item node(m_operands.first()->evaluateSingleton(context));

    if(node)
    {
        const QName name(node.asNode().name());

        if(name.isNull())
            return CommonValues::EmptyString;
        else
            return AtomicString::fromValue(context->namePool()->toLexical(name));
    }
    else
        return CommonValues::EmptyString;
}

Item LocalNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item node(m_operands.first()->evaluateSingleton(context));

    if(node)
    {
        const QName name(node.asNode().name());

        if(name.isNull())
            return CommonValues::EmptyString;
        else
            return AtomicString::fromValue(context->namePool()->stringForLocalName(name.localName()));
    }
    else
        return CommonValues::EmptyString;
}

Item NamespaceURIFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item node(m_operands.first()->evaluateSingleton(context));

    if(node)
    {
        const QName name(node.asNode().name());

        if(name.isNull())
            return CommonValues::EmptyAnyURI;
        else
            return toItem(AnyURI::fromValue(context->namePool()->stringForNamespace(name.namespaceURI())));
    }
    else
        return CommonValues::EmptyAnyURI;
}

Item NumberFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operands.first()->evaluateSingleton(context));

    if(!item)
        return CommonValues::DoubleNaN;

    const Item val(cast(item, context));
    Q_ASSERT(val);

    if(val.as<AtomicValue>()->hasError())
        return CommonValues::DoubleNaN;
    else
        return val;
}

Expression::Ptr NumberFN::typeCheck(const StaticContext::Ptr &context,
                                    const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(FunctionCall::typeCheck(context, reqType));
    const ItemType::Ptr sourceType(m_operands.first()->staticType()->itemType());

    if(BuiltinTypes::xsDouble->xdtTypeMatches(sourceType))
    {
        /* The operand is already xs:double, no need for fn:number(). */
        return m_operands.first()->typeCheck(context, reqType);
    }
    else if(prepareCasting(context, sourceType))
        return me;
    else
    {
        /* Casting to xs:double will never succeed and we would always return NaN.*/
        return wrapLiteral(CommonValues::DoubleNaN, context, this)->typeCheck(context, reqType);
    }
}

bool LangFN::isLangMatch(const QString &candidate, const QString &toMatch)
{
    qDebug() << Q_FUNC_INFO << candidate << toMatch;
    if(QString::compare(candidate, toMatch, Qt::CaseInsensitive) == 0)
        return true;

    return candidate.startsWith(toMatch, Qt::CaseInsensitive)
           && candidate.length() > toMatch.length()
           && candidate.at(toMatch.length()) == QLatin1Char('-');
}

Item LangFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item langArg(m_operands.first()->evaluateSingleton(context));
    const QString lang(langArg ? langArg.stringValue() : QString());

    const QName xmlLang(StandardNamespaces::xml, StandardLocalNames::lang, StandardPrefixes::xml);
    const Node langNode(m_operands.at(1)->evaluateSingleton(context).asNode());

    const Item::Iterator::Ptr ancestors(langNode.iterate(Node::AncestorOrSelf));
    Item ancestor(ancestors->next());

    while(ancestor)
    {
        const Item::Iterator::Ptr attributes(ancestor.asNode().iterate(Node::AttributeAxis));
        Item attribute(attributes->next());

        while(attribute)
        {
            Q_ASSERT(attribute.asNode().kind() == Node::Attribute);

            if(attribute.asNode().name() == xmlLang)
            {
                if(isLangMatch(attribute.asNode().stringValue(), lang))
                    return CommonValues::BooleanTrue;
                else
                    return CommonValues::BooleanFalse;
            }

            attribute = attributes->next();
        }

        ancestor = ancestors->next();
    }

    return CommonValues::BooleanFalse;
}

Item RootFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item arg(m_operands.first()->evaluateSingleton(context));

    if(arg)
        return arg.asNode().root();
    else
        return Item();
}

SequenceType::Ptr RootFN::staticType() const
{
    if(m_operands.isEmpty())
        return makeGenericSequenceType(BuiltinTypes::node, Cardinality::exactlyOne());
    else
        return makeGenericSequenceType(BuiltinTypes::node, m_operands.first()->staticType()->cardinality().toWithoutMany());
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
