/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include "toplevel.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KAstTopLevel topLevel;
    topLevel.setWindowTitle("Ported Asteroids Game");
    topLevel.show();

    app.setQuitOnLastWindowClosed(true);
    return app.exec();
}
