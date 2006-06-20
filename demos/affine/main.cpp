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

#include "xform.h"

#include <QApplication>

bool USE_OPENGL = false;

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(affine);

    QApplication app(argc, argv);

    if (app.arguments().size() > 1
        && app.arguments().at(1) == QLatin1String("-opengl"))
        USE_OPENGL = true;

    XFormWidget xformWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    xformWidget.setStyle(arthurStyle);

    QList<QWidget *> widgets = qFindChildren<QWidget *>(&xformWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);

    xformWidget.show();

    return app.exec();
}
