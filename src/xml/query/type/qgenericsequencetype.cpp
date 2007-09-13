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

#include <QString>

#include "qgenericsequencetype_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

GenericSequenceType::GenericSequenceType(const ItemType::Ptr &iType,
                                         const Cardinality &card) : m_itemType(iType),
                                                                    m_cardinality(card)
{
    Q_ASSERT(m_itemType);
}

QString GenericSequenceType::displayName(const NamePool::Ptr &np) const
{
    return m_itemType->displayName(np) + m_cardinality.displayName(Cardinality::ExcludeExplanation);
}

Cardinality GenericSequenceType::cardinality() const
{
    return m_cardinality;
}

ItemType::Ptr GenericSequenceType::itemType() const
{
    return m_itemType;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
