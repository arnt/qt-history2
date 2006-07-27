/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ui_previewwidgetbase.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(qtconfig);

    QApplication app(argc, argv);
    MainWindow mw;
    mw.show();
    return app.exec();
}
