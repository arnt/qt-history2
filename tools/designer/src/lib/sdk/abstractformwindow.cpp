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

#include "abstractformwindow.h"

#include <QtGui/QMainWindow>

AbstractFormWindow::AbstractFormWindow(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

AbstractFormWindow::~AbstractFormWindow()
{
}

AbstractFormEditor *AbstractFormWindow::core() const
{
    return 0;
}

// This is very similar to the static FormWindow::findFormWindow(), please KEEP IN SYNC.
AbstractFormWindow *AbstractFormWindow::findFormWindow(QWidget *w)
{
    while (w) {
        if (AbstractFormWindow *fw = qobject_cast<AbstractFormWindow*>(w)) {
            return fw;
        } else if (qobject_cast<QMainWindow*>(w)) {
            /* skip */
        } else if (w->isWindow())
            break;

        w = w->parentWidget();
    }

    return 0;
}
