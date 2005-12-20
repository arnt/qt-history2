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

#include <QtGui>

#include "treemodel.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(simpletreemodel);

    QApplication app(argc, argv);

    QFile file(":/default.txt");
    file.open(QIODevice::ReadOnly);
    TreeModel model(file.readAll());
    file.close();

    QTreeView view;
    view.setModel(&model);
    view.setWindowTitle(QObject::tr("Simple Tree Model"));
    view.show();
    return app.exec();
}
