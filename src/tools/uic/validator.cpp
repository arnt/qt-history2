/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "validator.h"
#include "driver.h"
#include "ui4.h"

#include <qtextstream.h>

Validator::Validator(Driver *drv)
    : driver(drv), output(drv->output()), option(drv->option())
{
}

void Validator::accept(DomUI *node)
{
    TreeWalker::accept(node);
}

void Validator::accept(DomWidget *node)
{
    QString widgetClass = node->attributeClass();

    if (widgetClass == QLatin1String("Line"))
        node->setAttributeClass(QLatin1String("QFrame"));

    TreeWalker::accept(node);
}

