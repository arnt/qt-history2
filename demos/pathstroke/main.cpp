/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "pathstroke.h"
#include <QApplication>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(pathstroke);

    QApplication app(argc, argv);

    PathStrokeWidget pathStrokeWidget;
    QStyle *arthurStyle = new ArthurStyle();
    pathStrokeWidget.setStyle(arthurStyle);
    QList<QWidget *> widgets = qFindChildren<QWidget *>(&pathStrokeWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
    pathStrokeWidget.show();

    return app.exec();
}
