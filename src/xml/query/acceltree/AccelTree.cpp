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
***************************************************************************
*/

#include <QtDebug>

#include "AccelIterators.h"
#include "AccelTree.h"
#include "AtomicString.h"
#include "CommonValues.h"
#include "CompressedWhitespace.h"
#include "SequenceReceiver.h"
#include "UntypedAtomic.h"

using namespace Patternist;

void AccelTree::printStats(const NamePool::Ptr &np) const
{
    Q_ASSERT(np);
    Q_UNUSED(np); /* Needed when compiling in release mode. */
#ifndef QT_NO_DEBUG
    const int len = basicData.count();

    qDebug() << "AccelTree stats for" << (m_documentURI.isEmpty() ? QString::fromLatin1("<empty URI>") : m_documentURI.toString());
    qDebug() << "Maximum pre number:" << maximumPreNumber();
    qDebug() << "+---------------+-------+-------+---------------+-------+--------------+-------+";
    qDebug() << "| Pre number    | Depth | Size  | Post Number   | Kind  | Name         | Value |";
    qDebug() << "+---------------+-------+-------+---------------+-------+--------------+-------+";
    for(int i = 0; i < len; ++i)
    {
        const BasicNodeData &v = basicData.at(i);
        qDebug() << "|" << i
                 << "\t\t|" << v.depth()
                 << "\t|" << v.size()
                 << "\t|" << postNumber(i)
                 << "\t|" << v.kind()
                 << "\t\t|" << (v.name().isNull() ? QString::fromLatin1("(none)") : np->displayName(v.name()))
                 << "\t\t|" << data.value(i)
                 << "\t|";
        /*
        qDebug() << "|" << QString().arg(i, 14)
                 << "|" << QString().arg(v.depth(), 6)
                 << "|" << QString().arg(v.size(), 6)
                 << "|" << QString().arg(postNumber(i), 14)
                 << "|" << QString().arg(v.kind(), 6)
                 << "|";
                 */
    }
    qDebug() << "+---------------+-------+-------+---------------+-------+--------------+";
#endif
}

QUrl AccelTree::baseURI(const Node ni) const
{
    switch(kind(toPreNumber(ni)))
    {
        case Node::Document:
            return baseURI();
        case Node::Element:
        {
            const Item::Iterator::Ptr it(iterate(ni, Node::AttributeAxis));
            Item next(it->next());

            while(next)
            {
                if(next.asNode().name() == QName(StandardNamespaces::xml, StandardLocalNames::base))
                {
                    const QUrl candidate(next.stringValue());
                    //  TODO. The xml:base spec says to do URI escaping here.

                    if(!candidate.isValid())
                        return QUrl();
                    else if(candidate.isRelative())
                    {
                        const Node par(parent(ni));

                        if(par)
                            return par.baseURI().resolved(candidate);
                        else
                            return baseURI().resolved(candidate);
                    }
                    else
                        return candidate;
                }

                next = it->next();
            }

            /* We have no xml:base-attribute. Can any parent supply us a base URI? */
            const Node par(parent(ni));

            if(par)
                return par.baseURI();
            else
                return baseURI();
        }
        case Node::ProcessingInstruction:
        /* Fallthrough. */
        case Node::Comment:
        /* Fallthrough. */
        case Node::Attribute:
        /* Fallthrough. */
        case Node::Text:
        {
            const Node par(ni.parent());
            if(par)
                return par.baseURI();
            else
                return QUrl();
        }
        case Node::Namespace:
            return QUrl();
    }

    Q_ASSERT_X(false, Q_FUNC_INFO, "This line is never supposed to be reached.");
    return QUrl();
}

QUrl AccelTree::documentURI(const Node ni) const
{
    if(kind(toPreNumber(ni)) == Node::Document)
        return documentURI();
    else
        return QUrl();
}

Node::NodeKind AccelTree::kind(const Node ni) const
{
    return kind(toPreNumber(ni));
}

Node::DocumentOrder AccelTree::compareOrderTo(const Node ni1, const Node ni2) const
{
    const PreNumber p1 = ni1.data();
    const PreNumber p2 = ni2.data();

    if(p1 == p2)
        return Node::Is;
    else if(p1 < p2)
        return Node::Precedes;
    else
        return Node::Follows;
}

Node AccelTree::root(const Node) const
{
    return createNode(0);
}

Node AccelTree::parent(const Node ni) const
{
    const AccelTree::PreNumber p = basicData.at(toPreNumber(ni)).parent();

    if(p == -1)
        return createNullNode();
    else
        return createNode(p);
}

Item::Iterator::Ptr AccelTree::iterate(const Node ni, const Node::Axis axis) const
{
    const PreNumber preNumber = toPreNumber(ni);

    switch(axis)
    {
        case Node::Child:
        {
            if(hasChildren(preNumber))
                return Item::Iterator::Ptr(new ChildIterator(this, preNumber));
            else
                return CommonValues::emptyIterator;
        }
        case Node::Ancestor:
        {
            if(hasParent(preNumber))
                return Item::Iterator::Ptr(new AncestorIterator<false>(this, preNumber));
            else
                return CommonValues::emptyIterator;
        }
        case Node::AncestorOrSelf:
            return Item::Iterator::Ptr(new AncestorIterator<true>(this, preNumber));
        case Node::Parent:
        {
            qDebug() << "AccelNode::iterate(Parent)" << preNumber;
            if(hasParent(preNumber))
                return makeSingletonIterator(Item(createNode(parent(preNumber))));
            else
                return CommonValues::emptyIterator;
        }
        case Node::Descendant:
        {
            if(hasChildren(preNumber))
                return Item::Iterator::Ptr(new DescendantIterator<false>(this, preNumber));
            else
                return CommonValues::emptyIterator;
        }
        case Node::DescendantOrSelf:
            return Item::Iterator::Ptr(new DescendantIterator<true>(this, preNumber));
        case Node::Following:
        {
            if(preNumber == maximumPreNumber())
                return CommonValues::emptyIterator;
            else
                return Item::Iterator::Ptr(new FollowingIterator(this, preNumber));
        }
        case Node::AttributeAxis:
        {
            if(hasChildren(preNumber) && kind(preNumber + 1) == Node::Attribute)
                return Item::Iterator::Ptr(new AttributeIterator(this, preNumber));
            else
                return CommonValues::emptyIterator;
        }
        case Node::Preceding:
        {
            if(preNumber == 0)
                return CommonValues::emptyIterator;
            else
                return Item::Iterator::Ptr(new PrecedingIterator(this, preNumber));
        }
        case Node::Self:
            return makeSingletonIterator(Item(createNode(toPreNumber(ni))));
        case Node::FollowingSibling:
        {
            if(preNumber == maximumPreNumber())
                return CommonValues::emptyIterator;
            else
                return Item::Iterator::Ptr(new SiblingIterator<true>(this, preNumber));
        }
        case Node::PrecedingSibling:
        {
            qDebug() << "PrecedingSibling" << preNumber;
            if(preNumber == 0)
                return CommonValues::emptyIterator;
            else
                return Item::Iterator::Ptr(new SiblingIterator<false>(this, preNumber));
        }
        case Node::NamespaceAxis:
            return CommonValues::emptyIterator;
        case Node::ForwardAxis:
        /* Fallthrough. */
        case Node::ReverseAxis:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "It makes no sense to use these values on their own.");
            return Item::Iterator::Ptr();
        }
    }

    Q_ASSERT(false);
    return Item::Iterator::Ptr();
}

QName AccelTree::name(const Node ni) const
{
    /* If this node type does not have a name(for instance, it's a comment)
     * we will return the default constructed value, which is conformant with
     * this function's contract. */
    return name(toPreNumber(ni));
}

NamespaceBinding::Vector AccelTree::namespaceBindings(const Node ni) const
{
    /* We get a hold of the ancestor, and loop them in reverse document
     * order(first the parent, then the parent's parent, etc). As soon
     * we find a binding that hasn't already been added, we add it to the
     * result list. In that way, declarations appearing further down override
     * those further up. */

    const PreNumber preNumber = toPreNumber(ni);

    const Item::Iterator::Ptr it(new AncestorIterator<true>(this, preNumber));
    NamespaceBinding::Vector result;
    Item n(it->next());

    /* Whether xmlns="" has been encountered. */
    bool hasUndeclaration = false;

    while(n)
    {
        const NamespaceBinding::Vector forNode(namespaces.value(toPreNumber(n.asNode())));
        const int len = forNode.size();

        for(int i = 0; i < len; ++i)
        {
            const NamespaceBinding &nsb = forNode.at(i);

            if(nsb.prefix() == StandardPrefixes::empty &&
               nsb.namespaceURI() == StandardNamespaces::empty)
            {
                hasUndeclaration = true;
                continue;
            }

            if(!hasPrefix(result, nsb.prefix()))
            {
                /* We've already encountered an undeclaration, so we're supposed to skip
                 * them. */
                if(hasUndeclaration && nsb.prefix() == StandardPrefixes::empty)
                    continue;
                else
                    result.append(forNode.at(i));
            }
        }

        n = it->next();
    }

    result.append(NamespaceBinding(StandardPrefixes::xml, StandardNamespaces::xml));

    return result;
}

void AccelTree::sendNamespaces(const Node n,
                               const PlainSharedPtr<SequenceReceiver> &receiver) const
{
    const PreNumber preNumber = toPreNumber(n);

    if(kind(preNumber) != Node::Element)
        return;

    const NamespaceBinding::Vector &nss = namespaces.value(preNumber);

    /* This is by far the most common case. */
    if(nss.isEmpty())
        return;

    const int len = nss.count();

    for(int i = 0; i < len; ++i)
        receiver->namespaceBinding(nss.at(i));
}

QString AccelTree::stringValue(const Node ni) const
{
    const PreNumber preNumber = toPreNumber(ni);

    switch(kind(preNumber))
    {
        case Node::Element:
        {
            /* Concatenate all text nodes that are descendants of this node. */
            if(!hasChildren(preNumber))
                return QString();

            const AccelTree::PreNumber stop = preNumber + size(preNumber);
            AccelTree::PreNumber pn = preNumber + 1; /* Jump over ourselves. */
            QString result;

            for(; pn <= stop; ++pn)
            {
                if(kind(pn) == Node::Text)
                    result.append(data.value(pn));
            }

            return result;
        }
        case Node::Text:
        {
            if(isCompressed(preNumber))
                return CompressedWhitespace::decompress(data.value(preNumber));
            /* Else, fallthrough. It's not compressed so use it as it is. */
        }
        case Node::Attribute:
        /* Fallthrough */
        case Node::ProcessingInstruction:
        /* Fallthrough */
        case Node::Comment:
            qDebug() << "Returning uncompressed whitespace.";
            return data.value(preNumber);
        case Node::Document:
        {
            /* Concatenate all text nodes in the whole document. */

            QString result;
            // Perhaps we can QString::reserve() the result based on the size?
            const AccelTree::PreNumber max = maximumPreNumber();

            for(AccelTree::PreNumber i = 0; i <= max; ++i)
            {
                if(kind(i) == Node::Text)
                    result += data.value(i);
            }

            return result;
        }
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "A node type that doesn't exist in the XPath Data Model was encountered.");
            return QString(); /* Dummy, silence compiler warning. */
        }
    }
}

bool AccelTree::hasPrefix(const NamespaceBinding::Vector &nbs, const QName::PrefixCode prefix)
{
    const int size = nbs.size();

    for(int i = 0; i < size; ++i)
    {
        if(nbs.at(i).prefix() == prefix)
            return true;
    }

    return false;
}

ItemType::Ptr AccelTree::type(const Node ni) const
{
    /* kind() is manually inlined here to avoid a virtual call. */
    return typeFromKind(basicData.at(toPreNumber(ni)).kind());
}

Item::Iterator::Ptr AccelTree::typedValue(const Node n) const
{
    const PreNumber preNumber = toPreNumber(n);

    switch(kind(preNumber))
    {
        case Node::Element:
        /* Fallthrough. */
        case Node::Document:
        /* Fallthrough. */
        case Node::Attribute:
            return makeSingletonIterator(Item(UntypedAtomic::fromValue(stringValue(n))));

        case Node::Text:
        /* Fallthrough. */
        case Node::ProcessingInstruction:
        /* Fallthrough. */
        case Node::Comment:
            return makeSingletonIterator(Item(AtomicString::fromValue(stringValue(n))));
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       qPrintable(QString::fromLatin1("A node type that doesn't exist "
                                                      "in the XPath Data Model was encountered.").arg(kind(preNumber))));
            return Item::Iterator::Ptr(); /* Dummy, silence compiler warning. */
        }
    }
}

// vim: et:ts=4:sw=4:sts=4
