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

#include "formwindow_widgetstack.h"

#include <QtGui/QWidget>
#include <QtGui/qevent.h>

#include <QtCore/qdebug.h>


FormWindowWidgetStack::FormWindowWidgetStack(QWidget *parent)
    : QWidget(parent),
      m_currentIndex(-1)
{
}

FormWindowWidgetStack::~FormWindowWidgetStack()
{
}

int FormWindowWidgetStack::count() const
{
    return m_widgets.count();
}

QWidget *FormWindowWidgetStack::widget(int index) const
{
    return m_widgets.at(index);
}

int FormWindowWidgetStack::indexOf(QWidget *widget) const
{
    return m_widgets.indexOf(widget);
}

int FormWindowWidgetStack::currentIndex() const
{
    return m_currentIndex;
}

void FormWindowWidgetStack::setCurrentIndex(int index)
{
    if (m_currentIndex == index) {
        // nothing to do
        return;
    }

    m_currentIndex = index;
    emit currentIndexChanged(m_currentIndex);
}

void FormWindowWidgetStack::addWidget(QWidget *widget)
{
    insertWidget(-1, widget);
    setCurrentIndex(count() - 1);
}

void FormWindowWidgetStack::insertWidget(int index, QWidget *widget)
{
    widget->setParent(this, 0);
    m_widgets.insert(index, widget);
}

void FormWindowWidgetStack::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    foreach (QWidget *widget, m_widgets) {
        widget->setGeometry(0, 0, event->size().width(), event->size().height());
    }

    if (m_currentIndex != -1) {
        widget(m_currentIndex)->raise();
    }
}

