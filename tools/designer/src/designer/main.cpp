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

#include "qdesigner.h"
#include "qdesigner_mainwindow.h"

int main(int argc, char *argv[])
{
    QDesigner app(argc, argv);

    for (int i = 1; i < argc; ++i)
        app.mainWindow()->readInForm(QString::fromLocal8Bit(argv[i]));

    return app.exec();
}
