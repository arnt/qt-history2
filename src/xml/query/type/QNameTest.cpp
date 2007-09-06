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

#include <QHash>

#include "BuiltinTypes.h"
#include "Item.h"
#include "Item.h"

#include "QNameTest.h"

using namespace Patternist;

QNameTest::QNameTest(const ItemType::Ptr &primaryType,
                     const QName qName) : AbstractNodeTest(primaryType)
                                        , m_qName(qName)
{
    Q_ASSERT(!qName.isNull());
}

ItemType::Ptr QNameTest::create(const ItemType::Ptr &primaryType, const QName qName)
{
    Q_ASSERT(!qName.isNull());
    Q_ASSERT(primaryType);

    return ItemType::Ptr(new QNameTest(primaryType, qName));
}

bool QNameTest::itemMatches(const Item &item) const
{
    Q_ASSERT(item.isNode());
    return m_primaryType->itemMatches(item) &&
           item.asNode().name() == m_qName;
}

QString QNameTest::displayName(const NamePool::Ptr &np) const
{
    QString displayOther(m_primaryType->displayName(np));

    return displayOther.insert(displayOther.size() - 1, np->displayName(m_qName));
}

ItemType::InstanceOf QNameTest::instanceOf() const
{
    return ClassQNameTest;
}

bool QNameTest::operator==(const ItemType &other) const
{
    return other.instanceOf() == ClassQNameTest &&
           static_cast<const QNameTest &>(other).m_qName == m_qName;
}

// vim: et:ts=4:sw=4:sts=4
