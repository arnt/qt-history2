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
#include <QtGui/QtGui>

Q3MainWindowContainer::Q3MainWindowContainer(Q3MainWindow *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{}

int Q3MainWindowContainer::count() const
{
    return m_widget->centralWidget() ? 1 : 0;
}

QWidget *Q3MainWindowContainer::widget(int index) const
{
    if (index == -1)
        return 0;

    return m_widget->centralWidget();
}

int Q3MainWindowContainer::currentIndex() const
{
    return m_widget->centralWidget() ? 0 : -1;
}

void Q3MainWindowContainer::setCurrentIndex(int index)
{
    Q_UNUSED(index);
}

namespace Friendly {

class MainWindow: public Q3MainWindow
{
public:
    MainWindow() { Q_ASSERT(0); }

    friend class Q3MainWindowContainer;
};

}

void Q3MainWindowContainer::addWidget(QWidget *widget)
{
    qDebug() << "added widget:" << widget << "parentWidget:" << widget->parentWidget();

    Friendly::MainWindow *mw = static_cast<Friendly::MainWindow*>(m_widget);

    widget->setParent(m_widget);

    if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
        qDebug() << "added toolBar:" << toolBar;
        // mw->addToolBar(toolBar);
    } else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(widget)) {
        qDebug() << "added menuBar:" << menuBar;
        // mw->setMenuBar(menuBar);
    } else if (QStatusBar *statusBar = qobject_cast<QStatusBar*>(widget)) {
        qDebug() << "added statusBar" << statusBar;
        // mw->setStatusBar(statusBar);
    } else {
        qDebug() << "added centralWidget" << widget;
        Q_ASSERT(m_widget->centralWidget() == 0);

        m_widget->setCentralWidget(widget);
    }
}

void Q3MainWindowContainer::insertWidget(int index, QWidget *widget)
{
    Q_UNUSED(index);
    Q_ASSERT(m_widget->centralWidget() == 0);

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

