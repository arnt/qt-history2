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

#include <QApplication>
#include <QDir>
#include <QDirModel>
#include <QTreeView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDirModel *model = new QDirModel(QDir());

    QTreeView *tree = new QTreeView;
    tree->setModel(model);

    tree->setWindowTitle(app.tr("Qt Example - Directory Browser"));
    tree->show();
    app.setMainWidget(tree);

    return app.exec();
}
