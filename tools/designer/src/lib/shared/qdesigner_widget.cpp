/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_widget_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtGui/QPainter>
#include <QtGui/qevent.h>

namespace qdesigner_internal {

void paintGrid(QWidget *widget, QDesignerFormWindowInterface *formWindow, QPaintEvent *e, bool needFrame)
{
    paintGrid(widget, formWindow->grid(), e, needFrame);
}

void paintGrid(QWidget *widget, const QPoint &grid, QPaintEvent *e, bool needFrame)
{
    QPainter p(widget);

    p.fillRect(e->rect(), widget->palette().brush(widget->backgroundRole()));

    p.setPen(widget->palette().dark().color());

    const int gridDeltaX = grid.x();
    const int gridDeltaY = grid.y();

    if ( gridDeltaX > 1 && gridDeltaY > 1) { // turn off grid?
        const int xstart = (e->rect().x() / gridDeltaX) * gridDeltaX;
        const int ystart = (e->rect().y() / gridDeltaY) * gridDeltaY;

        const int xend = (e->rect().right()  / gridDeltaX) * gridDeltaX;
        const int yend = (e->rect().bottom() / gridDeltaY) * gridDeltaY;

        int pointCount = ((xend - xstart) / gridDeltaX) * ((yend - ystart) * gridDeltaY);

        static const int BUF_SIZE = 4096;
        QPoint points[BUF_SIZE];

        int i = 0;
        int x = xstart;
        int y = ystart;
        while (pointCount > 0) {
            while (i < pointCount && i < BUF_SIZE) {
                points[i] = QPoint(x, y);
                ++i;
                x += gridDeltaX;
                if (x > xend) {
                    x = xstart;
                    y += gridDeltaY;
                    if (y > yend) // probably never reached..
                        break;
                }
            }
            p.drawPoints(points, i);
            pointCount -= i;
            i = 0;
        }
    }
    if (needFrame) {
        p.setPen(widget->palette().dark().color());
        p.drawRect(e->rect());
    }
}
}

void QDesignerDialog::paintEvent(QPaintEvent *e)
{
    if (m_formWindow && m_formWindow->currentTool() == 0 && m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature)) {
        qdesigner_internal::paintGrid(this, m_formWindow, e);
    } else {
        QPainter p(this);
        p.fillRect(e->rect(), palette().brush(QPalette::Window));
    }
}

void QDesignerLabel::updateBuddy()
{
    if (myBuddy.isEmpty()) {
        QLabel::setBuddy(0);
        return;
    }

    const QList<QWidget *> widgets = qFindChildren<QWidget*>(topLevelWidget(), QString::fromUtf8(myBuddy));
    QListIterator<QWidget *> it(widgets);
    while (it.hasNext()) {
        QWidget *widget = it.next();
        if (widget && !widget->isHidden()) {
            QLabel::setBuddy(widget);
            return;
        }
    }
    QLabel::setBuddy(0);
}

QDesignerWidget::QDesignerWidget(QDesignerFormWindowInterface* formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow)
{
    setBackgroundRole(QPalette::Window);
}

QDesignerWidget::~QDesignerWidget()
{
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    if (m_formWindow && m_formWindow->currentTool() == 0 && m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature))
        qdesigner_internal::paintGrid(this, m_formWindow, e);
    else
        QWidget::paintEvent(e);
}

void QDesignerWidget::dragEnterEvent(QDragEnterEvent *)
{
//    e->setAccepted(QTextDrag::canDecode(e));
}

QDesignerLabel::QDesignerLabel(QWidget *parent)
    : QLabel(parent)
{
}

void QDesignerLabel::setBuddy(QWidget *widget)
{
    QLabel::setBuddy(widget);
}
