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
#include "uic.h"

#include <qtextstream.h>

Validator::Validator(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
}

void Validator::accept(DomUI *node)
{
    TreeWalker::accept(node);
}

void Validator::accept(DomWidget *node)
{
    (void) driver->findOrInsertWidget(node);

    QString widgetClass = node->attributeClass();

    if (widgetClass == QLatin1String("Line"))
        node->setAttributeClass(QLatin1String("QFrame"));

    TreeWalker::accept(node);
}

void Validator::accept(DomLayoutItem *node)
{
    (void) driver->findOrInsertLayoutItem(node);

    TreeWalker::accept(node);
}

void Validator::accept(DomLayout *node)
{
    (void) driver->findOrInsertLayout(node);

    TreeWalker::accept(node);
}

void Validator::accept(DomActionGroup *node)
{
    (void) driver->findOrInsertActionGroup(node);

    TreeWalker::accept(node);
}

void Validator::accept(DomAction *node)
{
    (void) driver->findOrInsertAction(node);

    TreeWalker::accept(node);
}

