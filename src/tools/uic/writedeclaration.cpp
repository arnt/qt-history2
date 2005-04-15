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

#include "writedeclaration.h"
#include "writeicondeclaration.h"
#include "writeinitialization.h"
#include "writeiconinitialization.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"
#include "databaseinfo.h"
#include "customwidgetsinfo.h"

#include <qtextstream.h>

WriteDeclaration::WriteDeclaration(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
    this->uic = uic;
}

void WriteDeclaration::acceptUI(DomUI *node)
{
    QString className = node->elementClass() + option.postfix;

    QString varName = driver->findOrInsertWidget(node->elementWidget());
    QString widgetClassName = node->elementWidget()->attributeClass();

    output << "class " << option.prefix << className << "\n"
           << "{\n"
           << "public:\n";

    QStringList connections = uic->databaseInfo()->connections();
    for (int i=0; i<connections.size(); ++i) {
        QString connection = connections.at(i);

        if (connection == QLatin1String("(default)"))
            continue;

        output << option.indent << "QSqlDatabase " << connection << "Connection;\n";
    }

    TreeWalker::acceptWidget(node->elementWidget());

    output << "\n";

    WriteInitialization(uic).acceptUI(node);

    if (node->elementImages()) {
        output << "\n"
            << "protected:\n"
            << option.indent << "enum IconID\n"
            << option.indent << "{\n";
        WriteIconDeclaration(uic).acceptUI(node);

        output << option.indent << option.indent << "unknown_ID\n"
            << option.indent << "};\n";

        WriteIconInitialization(uic).acceptUI(node);
    }

    output << "};\n\n";


    if (option.generateNamespace && !option.prefix.isEmpty()) {
        output << "namespace Ui\n"
            << "{\n"
            << option.indent << "class " << className << ": public " << option.prefix << className << " {};\n"
            << "} // namespace Ui\n\n";
    }
}

void WriteDeclaration::acceptWidget(DomWidget *node)
{
    QString className = QLatin1String("QWidget");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    output << option.indent << uic->customWidgetsInfo()->realClassName(className) << " *" << driver->findOrInsertWidget(node) << ";\n";

    TreeWalker::acceptWidget(node);
}

void WriteDeclaration::acceptLayout(DomLayout *node)
{
    QString className = QLatin1String("QLayout");
    if (node->hasAttributeClass())
        className = node->attributeClass();

    output << option.indent << className << " *" << driver->findOrInsertLayout(node) << ";\n";

    TreeWalker::acceptLayout(node);
}

void WriteDeclaration::acceptSpacer(DomSpacer *node)
{
    output << option.indent << "QSpacerItem *" << driver->findOrInsertSpacer(node) << ";\n";

    TreeWalker::acceptSpacer(node);
}

void WriteDeclaration::acceptActionGroup(DomActionGroup *node)
{
    output << option.indent << "QActionGroup *" << driver->findOrInsertActionGroup(node) << ";\n";

    TreeWalker::acceptActionGroup(node);
}

void WriteDeclaration::acceptAction(DomAction *node)
{
    output << option.indent << "QAction *" << driver->findOrInsertAction(node) << ";\n";

    TreeWalker::acceptAction(node);
}
