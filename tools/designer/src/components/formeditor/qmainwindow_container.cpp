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

#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>

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

void QMainWindowContainer::addWidget(QWidget *widget)
{
    if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
        m_mainWindow->addToolBar(toolBar);
        m_widgets.append(widget);
    } else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(widget)) {
        m_widgets.append(widget);
        m_mainWindow->setMenuBar(menuBar);
    } else if (QStatusBar *statusBar = qobject_cast<QStatusBar*>(widget)) {
        m_widgets.append(widget);
        m_mainWindow->setStatusBar(statusBar);
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

