/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "cursor.h"
#include "item.h"

Cursor::Cursor()
    : m_totalSteps(0)
{
}

Cursor::~Cursor()
{
    qDeleteAll(m_items);
    m_items.clear();
}

void Cursor::setPosition(const QPoint &pt, bool addStep)
{
    m_pos = pt;
    if (addStep)
        ++m_totalSteps;
}

void Cursor::addItem(const Item *item)
{
    m_items << item;
}
