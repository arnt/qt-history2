/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/qapplication.h>
#include <QtDBus/qdbusconnection.h>
#include "qdbusviewer.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList arguments = app.arguments();
    if (arguments.contains("--help")) {
        printf("Arguments:\n");
        printf("    --system    Use system bus\n");
        printf("    --help      This help\n");
        return 0;
    }

    bool showSystemBus = app.arguments().contains(QLatin1String("--system"));

    QDBusViewer viewer(showSystemBus ? QDBus::systemBus() : QDBus::sessionBus());
    viewer.show();

    return app.exec();
}

