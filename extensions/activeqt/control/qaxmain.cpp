/****************************************************************************
**
** Implementation of a default "main" function.
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qaxfactory.h>

int main(int argc, char **argv)
{
    QAxFactory::startServer();
    QApplication app(argc, argv);

    return app.exec();
}
