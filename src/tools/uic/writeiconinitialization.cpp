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

#include "writeiconinitialization.h"
#include "writeicondata.h"
#include "driver.h"
#include "ui4.h"

WriteIconInitialization::WriteIconInitialization(Driver *drv)
    : driver(drv), output(drv->output()), option(drv->option())
{
}

void WriteIconInitialization::accept(DomUI *node)
{
    if (node->elementImages() == 0)
        return;

    QString className = node->elementClass() + option.postfix;

    output << "inline QPixmap " << className << "::icon(" << className << "::IconID id)\n"
           << "{\n";

    WriteIconData(driver).accept(node);

    output << option.indent << "switch (id) {\n";

    TreeWalker::accept(node);

    output << option.indent << option.indent << "default: return QPixmap();\n";

    output << option.indent << "}\n"
           << "}\n\n";
}

void WriteIconInitialization::accept(DomImages *images)
{
    TreeWalker::accept(images);
}

void WriteIconInitialization::accept(DomImage *image)
{
    QString imageId = image->attributeName() + "_ID";
    QString ind = option.indent + option.indent;
    output << ind << "case " << imageId
           << ": return " << "QPixmap((const char**)" << image->attributeName() << "_data" << ");\n";
}

