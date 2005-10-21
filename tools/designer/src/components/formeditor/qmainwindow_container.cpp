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

#include "qmainwindow_container.h"

#include <QtCore/qdebug.h>

#include <QtGui/QLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QDockWidget>

using namespace qdesigner_internal;

QMainWindowContainer::QMainWindowContainer(QMainWindow *widget, QObject *parent)
    : QObject(parent),
      m_mainWindow(widget)
{
    Q_ASSERT(widget->centralWidget() == 0);
}

int QMainWindowContainer::count() const
{
    return m_widgets.count();
}

QWidget *QMainWindowContainer::widget(int index) const
{
    if (index == -1)
        return 0;

    return m_widgets.at(index);
}

int QMainWindowContainer::currentIndex() const
{
    return m_mainWindow->centralWidget() ? 0 : -1;
}

void QMainWindowContainer::setCurrentIndex(int index)
{
    Q_UNUSED(index);
}

static Qt::ToolBarArea toolBarArea(QToolBar *me)
{
    if (QMainWindow *mw = qobject_cast<QMainWindow*>(me->parentWidget())) {
        if (mw->layout() && mw->layout()->indexOf(me) != -1) {
            return mw->toolBarArea(me);
        }
    }

    return Qt::TopToolBarArea;
}

static Qt::DockWidgetArea dockWidgetArea(QDockWidget *me)
{
    if (QMainWindow *mw = qobject_cast<QMainWindow*>(me->parentWidget())) {
        // Make sure that me is actually managed by mw, otherwise
        // QMainWindow::dockWidgetArea() will be VERY upset
        QList<QLayout*> candidates;
        if (mw->layout()) {
            candidates.append(mw->layout());
            candidates += qFindChildren<QLayout*>(mw->layout());
        }
        foreach (QLayout *l, candidates) {
            if (l->indexOf(me) != -1) {
                return mw->dockWidgetArea(me);
            }
        }
    }

    return Qt::LeftDockWidgetArea;
}

void QMainWindowContainer::addWidget(QWidget *widget)
{
    if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
        m_widgets.append(widget);
        m_mainWindow->addToolBar(toolBarArea(toolBar), toolBar);
        toolBar->show();
    } else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(widget)) {
        m_widgets.append(widget);
        m_mainWindow->setMenuBar(menuBar);
        menuBar->show();
    } else if (QStatusBar *statusBar = qobject_cast<QStatusBar*>(widget)) {
        m_widgets.append(widget);
        m_mainWindow->setStatusBar(statusBar);
        statusBar->show();
    } else if (QDockWidget *dockWidget = qobject_cast<QDockWidget*>(widget)) {
        m_widgets.append(widget);
        m_mainWindow->addDockWidget(dockWidgetArea(dockWidget), dockWidget);
        dockWidget->show();
    } else if (widget != m_mainWindow->centralWidget()) {
        Q_ASSERT(m_mainWindow->centralWidget() == 0);
        widget->setParent(m_mainWindow);
        m_mainWindow->setCentralWidget(widget);
        m_widgets.prepend(widget);
    }
}

void QMainWindowContainer::insertWidget(int index, QWidget *widget)
{
    Q_UNUSED(index);

    addWidget(widget);
}

void QMainWindowContainer::remove(int index)
{
    QWidget *widget = m_widgets.at(index);
    if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
        m_mainWindow->removeToolBar(toolBar);
    } else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(widget)) {
        menuBar->hide();
        menuBar->setParent(0);
        m_mainWindow->setMenuBar(0);
    } else if (QStatusBar *statusBar = qobject_cast<QStatusBar*>(widget)) {
        statusBar->hide();
        statusBar->setParent(0);
        m_mainWindow->setStatusBar(0);
    } else if (QDockWidget *dockWidget = qobject_cast<QDockWidget*>(widget)) {
        m_mainWindow->removeDockWidget(dockWidget);
    }
    m_widgets.removeAt(index);
}

QMainWindowContainerFactory::QMainWindowContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QMainWindowContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (QMainWindow *w = qobject_cast<QMainWindow*>(object))
        return new QMainWindowContainer(w, parent);

    return 0;
}

