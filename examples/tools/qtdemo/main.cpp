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

#include "launcher.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qtdemo);

    QApplication app(argc, argv);
    Launcher launcher;
    if (!launcher.setup())
        return 1;

    launcher.show();
    return app.exec();
}
