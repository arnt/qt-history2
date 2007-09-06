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

#include "AbstractNodeTest.h"

using namespace Patternist;

AbstractNodeTest::AbstractNodeTest(const ItemType::Ptr &primaryType) : m_primaryType(primaryType)
{
    Q_ASSERT(m_primaryType);
}

bool AbstractNodeTest::xdtTypeMatches(const ItemType::Ptr &other) const
{
    Q_ASSERT(other);

    if(other->isNodeType())
    {
        if(*other == *this)
            return true;
        else
            return xdtTypeMatches(other->xdtSuperType());
    }
    else
        return false;
}

ItemType::Ptr AbstractNodeTest::atomizedType() const
{
    return m_primaryType->atomizedType();
}

ItemType::Ptr AbstractNodeTest::xdtSuperType() const
{
    return m_primaryType;
}

// vim: et:ts=4:sw=4:sts=4
