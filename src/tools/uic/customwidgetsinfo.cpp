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

#include "customwidgetsinfo.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"

CustomWidgetsInfo::CustomWidgetsInfo(Driver *drv)
    : driver(drv)
{
}

void CustomWidgetsInfo::accept(DomUI *node)
{
    m_customWidgets.clear();

    if (node->elementCustomWidgets())
        accept(node->elementCustomWidgets());
}

void CustomWidgetsInfo::accept(DomCustomWidgets *node)
{
    TreeWalker::accept(node);
}

void CustomWidgetsInfo::accept(DomCustomWidget *node)
{
    if (node->elementClass().isEmpty())
        return;

    m_customWidgets.insert(node->elementClass(), node);
}

bool CustomWidgetsInfo::extends(const QString &className, const QString &baseClassName) const
{
    if (className == baseClassName)
        return true;

    if (DomCustomWidget *c = customWidget(className))
        return c->elementExtends() == baseClassName;

    return false;
}
