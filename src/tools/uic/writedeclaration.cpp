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

#include "writedeclaration.h"
#include "driver.h"
#include "ui4.h"

#include <qtextstream.h>

WriteDeclaration::WriteDeclaration(Driver *drv)
    : driver(drv), output(drv->output()), option(drv->option())
{
}

void WriteDeclaration::accept(DomUI *node)
{
    QString className = node->elementClass() + option.postfix;

    QString varName = driver->findOrInsertWidget(node->elementWidget());
    QString widgetClassName = node->elementWidget()->attributeClass();

    output << "struct " << className << "\n"
           << "{\n";

    TreeWalker::accept(node->elementWidget());

    output << "\n" << option.indent << "inline void setupUI(" << widgetClassName << " *" << varName << ");\n";

    output << "};\n\n";

}

void WriteDeclaration::accept(DomWidget *node)
{
    QString className = QLatin1String("QWidget");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    output << option.indent << className << " *" << driver->findOrInsertWidget(node) << ";\n";

    TreeWalker::accept(node);
}

void WriteDeclaration::accept(DomLayout *node)
{
    QString className = QLatin1String("QLayout");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    output << option.indent << className << " *" << driver->findOrInsertLayout(node) << ";\n";

    TreeWalker::accept(node);
}

void WriteDeclaration::accept(DomSpacer *node)
{
    output << option.indent << "QSpacerItem *" << driver->findOrInsertSpacer(node) << ";\n";

    TreeWalker::accept(node);
}
