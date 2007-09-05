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
****************************************************************************/

#include <QCoreApplication>
#include <QMetaProperty>

#include "CommonValues.h"
#include "ItemMappingIterator.h"
#include "SequenceMappingIterator.h"

#include "QObjectNodeModel.h"
#include "QObjectPropertyToAttributeIterator.h"

using namespace Patternist;

QMetaProperty QObjectNodeModel::toMetaProperty(const Node n)
{
    const int propertyOffset = n.additionalData() & (~IsAttribute);
    const QObject *const qo = asQObject(n);
    qDebug() << Q_FUNC_INFO << "propertyOffset:" << propertyOffset << "count:" << qo->metaObject()->propertyCount();
    return qo->metaObject()->property(propertyOffset);
}

QObjectNodeModel::QObjectNodeModel(const NamePool::Ptr &np) : m_namePool(np)
                                                            , m_baseURI(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()))
{
    Q_ASSERT(m_baseURI.isValid());
}

const QObject *QObjectNodeModel::asQObject(const Node n)
{
    return static_cast<const QObject *>(n.internalPointer());
}

bool QObjectNodeModel::isProperty(const Node n)
{
    return n.additionalData() & IsAttribute;
}

QUrl QObjectNodeModel::baseURI(const Node) const
{
    return m_baseURI;
}

QUrl QObjectNodeModel::documentURI(const Node) const
{
    return m_baseURI;
}

Node::NodeKind QObjectNodeModel::kind(const Node ni) const
{
    if(isProperty(ni))
        return Node::Attribute;
    else
        return Node::Element;
}

Node::DocumentOrder QObjectNodeModel::compareOrderTo(const Node, const Node) const
{
    return Node::Follows; // TODO
}

Node QObjectNodeModel::root(const Node n) const
{
    const QObject *p = asQObject(n);
    Q_ASSERT(p);

    do
    {
        QObject *const candidate = p->parent();
        if(candidate)
            p = candidate;
        else
            break;
    }
    while(true);

    return createNode(p);
}

Node QObjectNodeModel::parent(const Node ni) const
{
    const QObject *const p = asQObject(ni)->parent();

    if(p)
        return createNode(p);
    else
        return createNullNode();
}

Item QObjectNodeModel::mapToItem(const QObject *const object,
                                 const DynamicContext::Ptr &) const
{
    return createNode(object);
}

Iterator<QObject *>::Ptr QObjectNodeModel::mapToSequence(const QObject *const object,
                                                         const DynamicContext::Ptr &) const
{
    return makeListIterator(object->children());
}

Item::List QObjectNodeModel::ancestors(const Node n) const
{
    /* We simply throw all of them into a QList and return an iterator over it. */
    const QObject *p = asQObject(n);
    Q_ASSERT(p);

    Item::List result;
    do
    {
        QObject *const candidate = p->parent();
        if(candidate)
        {
            result.append(createNode(candidate));
            p = candidate;
        }
        else
            break;
    }
    while(true);

    return result;
}

Item::Iterator::Ptr QObjectNodeModel::iterate(const Node ni, const Node::Axis axis) const
{
    switch(axis)
    {
        case Node::Child:
            return makeItemMappingIterator<Item>(this, makeListIterator(asQObject(ni)->children()), DynamicContext::Ptr());
        case Node::Self:
            return makeSingletonIterator(Item(ni));
        case Node::Descendant:
            return makeItemMappingIterator<Item>(this, makeSequenceMappingIterator<QObject *>(this, mapToSequence(asQObject(ni), DynamicContext::Ptr()), DynamicContext::Ptr()),
                                                 DynamicContext::Ptr());
        case Node::AttributeAxis:
            return Item::Iterator::Ptr(new QObjectPropertyToAttributeIterator(this, asQObject(ni)));
        case Node::Ancestor:
            return makeListIterator(ancestors(ni));
        case Node::AncestorOrSelf:
        {
            Item::List ans(ancestors(ni));
            ans.prepend(ni);
            return makeListIterator(ans);
        }
        case Node::DescendantOrSelf:
        case Node::Following:
        case Node::FollowingSibling:
        case Node::ForwardAxis:
        case Node::Parent:
            Q_ASSERT_X(false, Q_FUNC_INFO, "This shouldn't be received.");
        case Node::Preceding:
        case Node::PrecedingSibling:
        case Node::ReverseAxis:
        case Node::NamespaceAxis:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "Not implemented.");
            return Item::Iterator::Ptr();
        }
    }

    Q_ASSERT_X(false, Q_FUNC_INFO,
               "Unknown axis.");
    return Item::Iterator::Ptr();
}

QName QObjectNodeModel::name(const Node ni) const
{
    const char * className = 0;

    if(isProperty(ni))
        className = toMetaProperty(ni).name();
    else
        className = asQObject(ni)->metaObject()->className();

    Q_ASSERT(className);
    QName qName;
    qName.setLocalName(m_namePool->allocateLocalName(QLatin1String(className)));

    return qName;
}

NamespaceBinding::Vector QObjectNodeModel::namespaceBindings(const Node) const
{
    return NamespaceBinding::Vector();
}

void QObjectNodeModel::sendNamespaces(const Node,
                                      const PlainSharedPtr<SequenceReceiver> &) const
{
    /* Do nothing. */
}

Item QObjectNodeModel::propertyValue(const Node n) const
{
    const QObject *const qo = asQObject(n);

    const QVariant value(toMetaProperty(n).read(qo));

    /* Property "objectName", possibly others, may have no values, and therefore
     * invalid QVariants will be returned. */
    if(value.isValid())
        return AtomicValue::toXDM(value, this);
    else
        return Item();
}

QString QObjectNodeModel::stringValue(const Node n) const
{
    if(isProperty(n))
    {
        const Item p(propertyValue(n));
        if(p)
            return p.stringValue();
        else
            return QString();
    }
    else
        return QString();
}

Item::Iterator::Ptr QObjectNodeModel::typedValue(const Node n) const
{
    if(isProperty(n))
        return makeSingletonIterator(Item(propertyValue(n)));
    else
        return CommonValues::emptyIterator;
}

ItemType::Ptr QObjectNodeModel::type(const Node ni) const
{
    return typeFromKind(isProperty(ni) ? Node::Attribute : Node::Element);
}

