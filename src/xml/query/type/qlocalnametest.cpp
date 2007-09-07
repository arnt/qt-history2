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
#include "qxpathhelper_p.h"

#include "qlocalnametest_p.h"

using namespace Patternist;

LocalNameTest::LocalNameTest(const ItemType::Ptr &primaryType,
                             const QName::LocalNameCode &ncName) : AbstractNodeTest(primaryType),
                                                                   m_ncName(ncName)
{
}

ItemType::Ptr LocalNameTest::create(const ItemType::Ptr &primaryType, const QName::LocalNameCode localName)
{
    Q_ASSERT(primaryType);

    return ItemType::Ptr(new LocalNameTest(primaryType, localName));
}

bool LocalNameTest::itemMatches(const Item &item) const
{
    Q_ASSERT(item.isNode());
    return m_primaryType->itemMatches(item) &&
           item.asNode().name().localName() == m_ncName;
}

QString LocalNameTest::displayName(const NamePool::Ptr &np) const
{
    QString displayOther(m_primaryType->displayName(np));

    return displayOther.insert(displayOther.size() - 1,
                               QString::fromLatin1("*:") + np->stringForLocalName(m_ncName));
}

ItemType::InstanceOf LocalNameTest::instanceOf() const
{
    return ClassLocalNameTest;
}

bool LocalNameTest::operator==(const ItemType &other) const
{
    return other.instanceOf() == ClassLocalNameTest &&
           static_cast<const LocalNameTest &>(other).m_ncName == m_ncName;
}

// vim: et:ts=4:sw=4:sts=4
