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

#include "default_container.h"
#include "qdesigner_stackedbox.h"
#include "qdesigner_tabwidget.h"
#include "qdesigner_toolbox.h"

#include <QtGui/QMainWindow>
#include <QtGui/QDockWidget>

QDesignerContainer::QDesignerContainer(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
}

QDesignerContainer::~QDesignerContainer()
{
}

int QDesignerContainer::count() const
{
    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        return static_cast<QDesignerStackedWidget*>(m_widget)->count();
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        return static_cast<QDesignerTabWidget*>(m_widget)->count();
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        return static_cast<QDesignerToolBox*>(m_widget)->count();
    else if (qobject_cast<QMainWindow*>(m_widget))
        return 1;
    else if (qobject_cast<QDockWidget*>(m_widget))
        return 1;

    Q_ASSERT(0);
    return 0;
}

QWidget *QDesignerContainer::widget(int index) const
{
    if (QDesignerStackedWidget *stackedWidget = qobject_cast<QDesignerStackedWidget*>(m_widget))
        return stackedWidget->widget(index);
    else if (QDesignerTabWidget *tabWidget = qobject_cast<QDesignerTabWidget*>(m_widget))
        return tabWidget->widget(index);
    else if (QDesignerToolBox *toolBox = qobject_cast<QDesignerToolBox*>(m_widget))
        return toolBox->widget(index);
    else if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(m_widget))
        return mainWindow->centralWidget();
    else if (QDockWidget *dockWidget = qobject_cast<QDockWidget*>(m_widget))
        return dockWidget->widget();

    Q_ASSERT(0);
    return 0;
}

int QDesignerContainer::currentIndex() const
{
    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        return static_cast<QDesignerStackedWidget*>(m_widget)->currentIndex();
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        return static_cast<QDesignerTabWidget*>(m_widget)->currentIndex();
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        return static_cast<QDesignerToolBox*>(m_widget)->currentIndex();
    else if (qobject_cast<QMainWindow*>(m_widget))
        return 0;
    else if (qobject_cast<QDockWidget*>(m_widget))
        return 0;

    Q_ASSERT(0);
    return -1;
}

void QDesignerContainer::setCurrentIndex(int index)
{
    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->setCurrentIndex(index);
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->setCurrentIndex(index);
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->setCurrentIndex(index);
    else if (qobject_cast<QMainWindow*>(m_widget)) {
        /* ignore */
    } else if (qobject_cast<QDockWidget*>(m_widget)) {
        /* ignore */
    } else
        Q_ASSERT(0);
}

void QDesignerContainer::addWidget(QWidget *widget)
{
    if (widget->parentWidget())
        widget->setParent(0);

    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->addWidget(widget);
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->addTab(widget, QString::fromUtf8("Page"));
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->addItem(widget, QString::fromUtf8("Page"));
    else if (qobject_cast<QMainWindow*>(m_widget)) {
        static_cast<QMainWindow*>(m_widget)->setCentralWidget(widget);
    } else if (qobject_cast<QDockWidget*>(m_widget)) {
        /* ignore */
    } else
        Q_ASSERT(0);
}

void QDesignerContainer::insertWidget(int index, QWidget *widget)
{
    if (widget->parentWidget())
        widget->setParent(0);

    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->insertWidget(index, widget);
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->insertTab(index, widget, QString::fromUtf8("Page"));
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->insertItem(index, widget, QString::fromUtf8("Page"));
    else if (qobject_cast<QMainWindow*>(m_widget)) {
        Q_ASSERT(0);
    } else if (qobject_cast<QDockWidget*>(m_widget)) {
        /* ignore */
    } else
        Q_ASSERT(0);
}

void QDesignerContainer::remove(int index)
{
    if (qobject_cast<QDesignerStackedWidget*>(m_widget))
        static_cast<QDesignerStackedWidget*>(m_widget)->removeWidget(widget(index));
    else if (qobject_cast<QDesignerTabWidget*>(m_widget))
        static_cast<QDesignerTabWidget*>(m_widget)->removeTab(index);
    else if (qobject_cast<QDesignerToolBox*>(m_widget))
        static_cast<QDesignerToolBox*>(m_widget)->removeItem(index);
    else if (qobject_cast<QMainWindow*>(m_widget)) {
        /* ignore */
    } else if (qobject_cast<QDockWidget*>(m_widget)) {
        /* ignore */
    } else
        Q_ASSERT(0);
}

QDesignerContainerFactory::QDesignerContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerContainerExtension)
        && (qobject_cast<QDesignerStackedWidget*>(object)
                || qobject_cast<QDesignerTabWidget*>(object)
                || qobject_cast<QDesignerToolBox*>(object)
                || qobject_cast<QMainWindow*>(object)
                || qobject_cast<QDockWidget*>(object)))
                return new QDesignerContainer(static_cast<QWidget*>(object), parent);

    return 0;
}
