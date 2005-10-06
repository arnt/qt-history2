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

#include <QtDesigner/QFormBuilder>
#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFormBuilder builder;

    QString pluginsPath = QLibraryInfo::location(QLibraryInfo::PluginsPath);
    builder.addPluginPath(pluginsPath + QLatin1String("/designer"));

    QFile file(":/forms/form.ui");
    if (file.open(QFile::ReadOnly)) {
        QWidget *widget = builder.load(&file, 0);
        Q_ASSERT(widget != 0);
        widget->show();
    }

    return app.exec();
}
