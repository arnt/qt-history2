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

#include <QApplication>
#include <QTimer>
#include "movieplayer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MoviePlayer player;
    QStringList arguments = app.arguments();
    if (arguments.count() > 1)
        player.openFile(arguments.last());
    player.show();
    return app.exec();
}
