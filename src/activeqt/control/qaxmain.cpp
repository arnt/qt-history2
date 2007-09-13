/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qaxfactory.h>

int main(int argc, char **argv)
{
    QT_USE_NAMESPACE
    QAxFactory::startServer();
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    return app.exec();
}
