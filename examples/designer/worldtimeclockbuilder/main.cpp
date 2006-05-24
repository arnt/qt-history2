/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtUiTools>
#include <QtGui>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(worldtimeclockbuilder);

    QApplication app(argc, argv);

    QUiLoader loader;

    QFile file(":/forms/form.ui");
    file.open(QFile::ReadOnly);

    QWidget *widget = loader.load(&file);

    file.close();
    widget->show();

    return app.exec();
}
