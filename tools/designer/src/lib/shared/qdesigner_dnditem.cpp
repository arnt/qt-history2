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

#include "qdesigner_dnditem.h"

#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/QLabel>

#include <ui4.h>

QDesignerDnDItem::QDesignerDnDItem()
    : m_domUi(0),
      m_widget(0),
      m_decoration(0),
      m_ownWidget(false)
{
}

QDesignerDnDItem::~QDesignerDnDItem()
{
    if (m_ownWidget)
        delete m_widget;

    delete m_decoration;
    delete m_domUi;

    m_domUi = 0;
    m_widget = 0;
    m_decoration = 0;
}

DomUI *QDesignerDnDItem::domUi() const
{
    return m_domUi;
}

QWidget *QDesignerDnDItem::decoration() const
{
    return m_decoration;
}

QPoint QDesignerDnDItem::hotSpot() const
{
    return m_hotSpot;
}

QDesignerDnDItem *QDesignerDnDItem::create(QWidget *widget, const QPoint &hotSpot)
{
    Q_UNUSED(widget);
    Q_UNUSED(hotSpot);

    Q_ASSERT(0);
    return 0;
}

QDesignerDnDItem *QDesignerDnDItem::create(DomUI *ui, QWidget *widget, const QPoint &hotSpot)
{
    Q_UNUSED(ui);
    Q_UNUSED(widget);
    Q_UNUSED(hotSpot);

    Q_ASSERT(0);
    return 0;
}

void QDesignerDnDItem::createDecoration(const QPoint &globalPos)
{
    Q_ASSERT(m_widget == 0);

    QLabel *label = new QLabel(0, Qt::ToolTip); // ### maybe we should use QWidget directly.
    label->setPixmap(QPixmap::grabWidget(m_widget));
    m_decoration = label;

    QRect geometry = m_widget->geometry();
    geometry.moveTopLeft(m_widget->mapToGlobal(QPoint(0, 0)));
    m_decoration->setGeometry(geometry);

    m_hotSpot = globalPos - m_decoration->geometry().topLeft();
}

void QDesignerDnDItem::setupDecoration(const QPoint &globalPos)
{
    Q_ASSERT(m_decoration != 0);

    QRect geometry = m_widget->geometry();
    geometry.moveTopLeft(m_widget->mapToGlobal(QPoint(0, 0)));
    if (geometry.contains(globalPos) == false) {
        // nothing to do
        return;
    }

    QPoint pos = m_decoration->mapFromGlobal(globalPos);

    QBitmap bitmap(m_decoration->size());

    QPainter p(&bitmap);
    p.fillRect(bitmap.rect(), Qt::color1);
    p.setPen(Qt::color0);
    p.drawPoint(pos);
    p.end();

    m_decoration->setMask(bitmap);

    m_decoration->setWindowOpacity(0.8);
    m_decoration->show();
}





