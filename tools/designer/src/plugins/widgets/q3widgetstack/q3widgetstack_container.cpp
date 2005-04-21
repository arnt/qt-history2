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

#include "q3widgetstack_container.h"
#include <Qt3Support/Q3WidgetStack>

#include <QtCore/qdebug.h>

Q3WidgetStackContainer::Q3WidgetStackContainer(Q3WidgetStack *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{}

int Q3WidgetStackContainer::count() const
{ return m_pages.count(); }

QWidget *Q3WidgetStackContainer::widget(int index) const
{
    if (index == -1)
        return 0;

    return m_pages.at(index);
}

int Q3WidgetStackContainer::currentIndex() const
{ return m_pages.indexOf(m_widget->visibleWidget()); }

void Q3WidgetStackContainer::setCurrentIndex(int index)
{ m_widget->raiseWidget(m_pages.at(index)); }

void Q3WidgetStackContainer::addWidget(QWidget *widget)
{
    m_pages.append(widget);
    m_widget->addWidget(widget);
}

void Q3WidgetStackContainer::insertWidget(int index, QWidget *widget)
{
    m_pages.insert(index, widget);
    m_widget->addWidget(widget);
}

void Q3WidgetStackContainer::remove(int index)
{
    m_widget->removeWidget(m_pages.at(index));
    m_pages.removeAt(index);
}

Q3WidgetStackContainerFactory::Q3WidgetStackContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *Q3WidgetStackContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (Q3WidgetStack *w = qobject_cast<Q3WidgetStack*>(object))
        return new Q3WidgetStackContainer(w, parent);

    return 0;
}

