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

#include <QHash>

#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qitem_p.h"

#include "qnamespacenametest_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

NamespaceNameTest::NamespaceNameTest(const ItemType::Ptr &primaryType,
                                     const QName::NamespaceCode namespaceURI) : AbstractNodeTest(primaryType),
                                                                                m_namespaceURI(namespaceURI)
{
}

ItemType::Ptr NamespaceNameTest::create(const ItemType::Ptr &primaryType, const QName::NamespaceCode namespaceURI)
{
    Q_ASSERT(primaryType);

    return ItemType::Ptr(new NamespaceNameTest(primaryType, namespaceURI));
}

bool NamespaceNameTest::itemMatches(const Item &item) const
{
    Q_ASSERT(item.isNode());
    return m_primaryType->itemMatches(item) &&
           item.asNode().name().namespaceURI() == m_namespaceURI;
}

QString NamespaceNameTest::displayName(const NamePool::Ptr &np) const
{
    qDebug() << Q_FUNC_INFO << np->stringForNamespace(m_namespaceURI) << m_namespaceURI;
    return QLatin1Char('{') + np->stringForNamespace(m_namespaceURI) + QLatin1String("}:*");
}

ItemType::InstanceOf NamespaceNameTest::instanceOf() const
{
    return ClassNamespaceNameTest;
}

bool NamespaceNameTest::operator==(const ItemType &other) const
{
    return other.instanceOf() == ClassNamespaceNameTest &&
           static_cast<const NamespaceNameTest &>(other).m_namespaceURI == m_namespaceURI;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
