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

#include "formwindow_dnditem.h"

#include <ui4.h>

#include <QtGui/QLabel>
#include <QtGui/QPixmap>

FormWindowDnDItem::FormWindowDnDItem(QWidget *widget, const QPoint &pos)
{
    m_dom_ui = 0;
    m_widget = widget;
    QLabel *label = new QLabel(0, Qt::ToolTip);
    label->setPixmap(QPixmap::grabWidget(m_widget));
    label->setWindowOpacity(0.8);

    QRect geometry = widget->geometry();
    geometry.moveTopLeft(widget->mapToGlobal(QPoint(0, 0)));
    label->setGeometry(geometry);

    m_decoration = label;

    m_hot_spot = pos - m_decoration->geometry().topLeft();
}

FormWindowDnDItem::FormWindowDnDItem(DomUI *dom_ui, QWidget *widget, const QPoint &pos)
{
    m_dom_ui = dom_ui;
    m_widget = 0;

    QLabel *label = new QLabel(0, Qt::ToolTip);
    label->setPixmap(QPixmap::grabWidget(widget));
    label->setWindowOpacity(0.8);
    QRect geometry = widget->geometry();
    geometry.moveTopLeft(widget->mapToGlobal(QPoint(0, 0)));
    label->setGeometry(geometry);

    m_decoration = label;

    m_hot_spot = pos - m_decoration->geometry().topLeft();
}

DomUI *FormWindowDnDItem::domUi() const
{
    return m_dom_ui;
}

QWidget *FormWindowDnDItem::decoration() const
{
    return m_decoration;
}

QWidget *FormWindowDnDItem::widget() const
{
    return m_widget;
}

FormWindowDnDItem::~FormWindowDnDItem()
{
    m_decoration->deleteLater();
    delete m_dom_ui;
    m_dom_ui = 0;
}

QPoint FormWindowDnDItem::hotSpot() const
{
    return m_hot_spot;
}

