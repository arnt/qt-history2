/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

#include "stylewindow.h"

int main(int argv, char *args[])
{
    QApplication app(argv, args);
    QApplication::setStyle(QStyleFactory::create("simplestyle"));

    StyleWindow window;
    window.resize(200, 50);
    window.show();

    return app.exec();
}
