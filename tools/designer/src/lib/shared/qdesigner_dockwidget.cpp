/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_dockwidget_p.h"
#include "layoutinfo_p.h"
#include <QtDesigner/QtDesigner>
#include <QtGui/QMainWindow>
#include <QLayout>

QDesignerDockWidget::QDesignerDockWidget(QWidget *parent)
    : QDockWidget(parent)
{
}

QDesignerDockWidget::~QDesignerDockWidget()
{
}

bool QDesignerDockWidget::docked() const
{
    return qobject_cast<QMainWindow*>(parentWidget()) != 0;
}

void QDesignerDockWidget::setDocked(bool b)
{
    if (QMainWindow *mainWindow = findMainWindow()) {
        QDesignerFormEditorInterface *core = formWindow()->core();
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), mainWindow);
        if (b && !docked()) {
            // Dock it
            // ### undo/redo stack
            setParent(0);
            c->addWidget(this);
            formWindow()->emitSelectionChanged();
        } else if (!b && docked()) {
            // Undock it
            for (int i = 0; i < c->count(); ++i) {
                if (c->widget(i) == this) {
                    c->remove(i);
                    break;
                }
            }
            // #### restore the position
            setParent(mainWindow->centralWidget());
            show();
            formWindow()->emitSelectionChanged();
        }
    }
}

Qt::DockWidgetArea QDesignerDockWidget::dockWidgetArea() const
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget()))
        return mainWindow->dockWidgetArea(const_cast<QDesignerDockWidget*>(this));

    return Qt::LeftDockWidgetArea;
}

void QDesignerDockWidget::setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea)
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget())) {
        mainWindow->addDockWidget(dockWidgetArea, this);
    }
}

bool QDesignerDockWidget::inMainWindow() const
{
    if (parentWidget() && parentWidget()->layout()) {
        QLayout *layout = parentWidget()->layout();

        if (layout->indexOf(const_cast<QDesignerDockWidget*>(this)) != -1)
            return false;
    }

    return findMainWindow() != 0;
}

QDesignerFormWindowInterface *QDesignerDockWidget::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerDockWidget*>(this));
}

QMainWindow *QDesignerDockWidget::findMainWindow() const
{
    if (QDesignerFormWindowInterface *fw = formWindow())
        return qobject_cast<QMainWindow*>(fw->mainContainer());
    return 0;
}
