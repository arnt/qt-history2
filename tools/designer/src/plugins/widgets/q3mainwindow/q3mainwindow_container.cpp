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

#include "q3mainwindow_container.h"

#include <Qt3Support/Q3MainWindow>

#include <QtCore/qdebug.h>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>

Q3MainWindowContainer::Q3MainWindowContainer(Q3MainWindow *widget, QObject *parent)
    : QObject(parent),
      m_mainWindow(widget)
{}

int Q3MainWindowContainer::count() const
{
    return m_mainWindow->centralWidget() ? 1 : 0;
}

QWidget *Q3MainWindowContainer::widget(int index) const
{
    if (index == -1)
        return 0;

    return m_mainWindow->centralWidget();
}

int Q3MainWindowContainer::currentIndex() const
{
    return m_mainWindow->centralWidget() ? 0 : -1;
}

void Q3MainWindowContainer::setCurrentIndex(int index)
{
    Q_UNUSED(index);
}

void Q3MainWindowContainer::addWidget(QWidget *widget)
{
    if (qobject_cast<QToolBar*>(widget)) {
        // ### add the toolbar
    } else if (qobject_cast<QMenuBar*>(widget)) {
        (void) m_mainWindow->menuBar();
    } else if (qobject_cast<QStatusBar*>(widget)) {
        (void) m_mainWindow->statusBar();
    } else {
        Q_ASSERT(m_mainWindow->centralWidget() == 0);
        widget->setParent(m_mainWindow);
        m_mainWindow->setCentralWidget(widget);
    }
}

void Q3MainWindowContainer::insertWidget(int index, QWidget *widget)
{
    Q_UNUSED(index);
    Q_ASSERT(m_mainWindow->centralWidget() == 0);

    addWidget(widget);
}

void Q3MainWindowContainer::remove(int index)
{
    Q_UNUSED(index);
    Q_ASSERT(0);
}

Q3MainWindowContainerFactory::Q3MainWindowContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *Q3MainWindowContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (Q3MainWindow *w = qobject_cast<Q3MainWindow*>(object))
        return new Q3MainWindowContainer(w, parent);

    return 0;
}

