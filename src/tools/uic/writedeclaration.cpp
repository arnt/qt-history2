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
#include "writeicondeclaration.h"
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

    if (node->elementImages()) {
        output << "\n"
            << option.indent << "enum IconID\n"
            << option.indent << "{\n";
        WriteIconDeclaration(driver).accept(node);

        output << option.indent << option.indent << "unknown_ID\n"
            << option.indent << "};\n";

        output << "\n" << option.indent << "static QPixmap icon(IconID id);\n";
    }

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

void WriteDeclaration::accept(DomActionGroup *node)
{
    output << option.indent << "QActionGroup *" << driver->findOrInsertActionGroup(node) << ";\n";

    TreeWalker::accept(node);
}

void WriteDeclaration::accept(DomAction *node)
{
    output << option.indent << "QAction *" << driver->findOrInsertAction(node) << ";\n";

    TreeWalker::accept(node);
}
