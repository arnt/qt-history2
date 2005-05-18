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

#include "qdesigner_dockwidget_p.h"
#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QMainWindow>

QDesignerDockWidget::QDesignerDockWidget(QWidget *parent)
    : QDockWidget(parent)
{
}

QDesignerDockWidget::~QDesignerDockWidget()
{
}

Qt::DockWidgetArea QDesignerDockWidget::dockWidgetArea() const
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget()))
        return mainWindow->dockWidgetArea(const_cast<QDesignerDockWidget*>(this));

    return Qt::DockWidgetArea(0);
}

void QDesignerDockWidget::setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea)
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget())) {
        mainWindow->removeDockWidget(this);
        mainWindow->addDockWidget(dockWidgetArea, this);
    }
}

bool QDesignerDockWidget::inMainWindow() const
{
    return qobject_cast<QMainWindow*>(parentWidget()) != 0;
}

