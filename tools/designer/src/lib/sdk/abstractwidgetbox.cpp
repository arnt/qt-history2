/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "abstractwidgetbox.h"

AbstractWidgetBox::AbstractWidgetBox(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

AbstractWidgetBox::~AbstractWidgetBox()
{
}

int AbstractWidgetBox::findOrInsertCategory(const QString &categoryName)
{
    int count = categoryCount();
    for (int index=0; index<count; ++index) {
        Category c = category(index);
        if (c.name() == categoryName)
            return index;
    }

    addCategory(Category(categoryName));
    return count;
}

