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

#include "orderindicator.h"
#include "formwindow.h"

#include <qpainter.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qevent.h>

OrderIndicator::OrderIndicator(int i, QWidget* w, FormWindow *fw)
    : QWidget(fw), formWindow(fw)
{
    setAttribute(Qt::WA_MouseNoMask);
    setObjectName("order_indicator");
    order = -1;
    m_widget = w;
    setAttribute(Qt::WA_NoBackground);
    setAttribute(Qt::WA_MouseNoMask);
    setAutoMask(true);
    setOrder(i, w);
}


OrderIndicator::~OrderIndicator()
{
}


void OrderIndicator::setOrder(int i, QWidget* wid)
{
    if (m_widget != wid)
        return;
    if (!wid->isVisibleTo(formWindow)) {
        hide();
        return;
    }

    if (order == i) {
        show();
        raise();
        return;
    }
    order = i;
    int w = fontMetrics().width(QString::number(i)) + 10;
    int h = fontMetrics().lineSpacing() * 3 / 2;
    QFont f(font());
    f.setBold(true);
    setFont(f);
    resize(qMax(w, h), h);
    update(); // in case the size didn't change
    reposition();
    show();
    raise();
}

void OrderIndicator::reposition()
{
    QPoint p =parentWidget()->mapFromGlobal(m_widget->mapToGlobal(m_widget->rect().topLeft()));
    move(p - QPoint(width()/3, height()/3));
}


void OrderIndicator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(Qt::white);
    p.setBrush(Qt::blue);
    p.drawEllipse(rect());
    p.drawText(rect(), Qt::AlignCenter, QString::number(order));
}


void OrderIndicator::updateMask()
{
    QBitmap bm(size());
    bm.fill(Qt::color0);
    {
        QPainter p(&bm);
        p.setPen(Qt::color1);
        p.setBrush(Qt::color1);
        p.drawEllipse(rect());
    }
    setMask(bm);
}

void OrderIndicator::mousePressEvent(QMouseEvent *e)
{
    QApplication::sendEvent(m_widget, e);
}
