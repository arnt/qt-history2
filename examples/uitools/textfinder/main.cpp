/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include "textfinder.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(textfinder);
    QApplication app(argc, argv);

    TextFinder *textFinder = new TextFinder;
    textFinder->show();

    return app.exec();
}
