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

// This is necessary because we want the widgets under the decoration to receive
// mouse events
static void makeHoleInDecoration(QWidget *deco, const QPoint &globalPos)
{
    QRect geometry = deco->geometry();
    geometry.moveTopLeft(deco->mapToGlobal(QPoint(0, 0)));
    if (!geometry.contains(globalPos)) {
        // nothing to do
        return;
    }

    QPoint pos = deco->mapFromGlobal(globalPos);

    QBitmap bitmap(deco->size());

    QPainter p(&bitmap);
    p.fillRect(bitmap.rect(), Qt::color1);
    p.setPen(Qt::color0);
    p.drawPoint(pos);
    p.end();

    deco->setMask(bitmap);

    deco->setWindowOpacity(0.8);
    deco->show();
}

QDesignerDnDItem::QDesignerDnDItem(DropType type, QWidget *source)
{
    m_source = source;
    m_type = type;
    m_dom_ui = 0;
    m_widget = 0;
    m_decoration = 0;
}

void QDesignerDnDItem::init(DomUI *ui, QWidget *widget, QWidget *decoration,
                                    const QPoint &global_mouse_pos)
{
    Q_ASSERT(widget != 0 || ui != 0);
    Q_ASSERT(decoration != 0);

    m_dom_ui = ui;
    m_widget = widget;
    m_decoration = decoration;

    makeHoleInDecoration(m_decoration, global_mouse_pos);

    QRect geometry = m_decoration->geometry();
    m_hot_spot = global_mouse_pos - m_decoration->geometry().topLeft();
}

QDesignerDnDItem::~QDesignerDnDItem()
{
    if (m_decoration != 0)
        m_decoration->deleteLater();
    delete m_dom_ui;
}

DomUI *QDesignerDnDItem::domUi() const
{
    return m_dom_ui;
}

QWidget *QDesignerDnDItem::decoration() const
{
    return m_decoration;
}

QPoint QDesignerDnDItem::hotSpot() const
{
    return m_hot_spot;
}

QWidget *QDesignerDnDItem::widget() const
{
    return m_widget;
}

QDesignerDnDItem::DropType QDesignerDnDItem::type() const
{
    return m_type;
}

QWidget *QDesignerDnDItem::source() const
{
    return m_source;
}

void QDesignerDnDItem::setDomUi(DomUI *dom_ui)
{
    delete m_dom_ui;
    m_dom_ui = dom_ui;
}
