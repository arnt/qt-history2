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

#include "writeicondeclaration.h"
#include "driver.h"
#include "ui4.h"

WriteIconDeclaration::WriteIconDeclaration(Driver *drv)
    : driver(drv), output(drv->output()), option(drv->option())
{
}

void WriteIconDeclaration::accept(DomUI *node)
{
    TreeWalker::accept(node);
}

void WriteIconDeclaration::accept(DomImages *images)
{
    TreeWalker::accept(images);
}

void WriteIconDeclaration::accept(DomImage *image)
{
    output << option.indent << option.indent << image->attributeName() << "_ID,\n";
}

