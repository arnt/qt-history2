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

#include "composition.h"

#include <QApplication>

int main(int argc, char **argv)
{
 //   Q_INIT_RESOURCE(deform);

    QApplication app(argc, argv);

    CompositionWidget compWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    compWidget.setStyle(arthurStyle);

    QList<QWidget *> widgets = qFindChildren<QWidget *>(&compWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
    compWidget.show();

    return app.exec();
}
