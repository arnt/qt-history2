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

#include "gradients.h"

#include <QApplication>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(gradients);

    QApplication app(argc, argv);

    GradientWidget gradientWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    gradientWidget.setStyle(arthurStyle);
    QList<QWidget *> widgets = qFindChildren<QWidget *>(&gradientWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
    gradientWidget.show();

    return app.exec();
}
