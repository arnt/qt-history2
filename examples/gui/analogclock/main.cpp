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

#include <QApplication>

#include "analogclock.h"

/*
    Creates an analog clock widget with a default size, and displays it.
*/

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AnalogClock *clock = new AnalogClock;
    clock->resize( 100, 100 );
    clock->show();
    app.setMainWidget(clock);

    return app.exec();
}
