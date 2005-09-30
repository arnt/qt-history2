/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "xform.h"

#include <QApplication>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(affine);

    QApplication app(argc, argv);

    XFormWidget xformWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    xformWidget.setStyle(arthurStyle);

    QList<QWidget *> widgets = qFindChildren<QWidget *>(&xformWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);

    xformWidget.show();

    return app.exec();
}
