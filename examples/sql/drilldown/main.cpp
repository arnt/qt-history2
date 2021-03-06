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

#include <QtGui>

#include "../connection.h"
#include "view.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(drilldown);

    QApplication app(argc, argv);

    if (!createConnection())
        return 1;

    View view("trolltechoffice", "images");
    view.show();
    return app.exec();
}
