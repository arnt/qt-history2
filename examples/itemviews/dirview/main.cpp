/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDirModel *model = new QDirModel;

    QTreeView *tree = new QTreeView;
    tree->setModel(model);

    tree->setWindowTitle(QObject::tr("Dir View"));
    tree->resize(640, 480);
    tree->show();

    return app.exec();
}
