/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Configuration.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "colorbutton.h"

#include <qapplication.h>
#include <qevent.h>
#include <qcolordialog.h>
#include <qpainter.h>
#include <qdragobject.h>
#include <qstyle.h>


ColorButton::ColorButton(QWidget *parent, const char *name)
    : QAbstractButton(parent, name), mousepressed(FALSE)
{
    setAcceptDrops(TRUE);
    col = black;
    connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}


ColorButton::ColorButton(const QColor &c, QWidget *parent, const char *name)
    : QAbstractButton(parent, name)
{
    setAcceptDrops(TRUE);
    col = c;
    connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}


void ColorButton::setColor(const QColor &c)
{
    col = c;
    update();
}


void ColorButton::changeColor()
{
    QColor c = QColorDialog::getColor(col, qApp->activeWindow());

    if (c.isValid()) {
        setColor(c);
        emit colorChanged(color());
    }
}


QSize ColorButton::sizeHint() const
{
    return QSize(40, 25);
}


QSize ColorButton::minimumSizeHint() const
{
    return QSize(40, 25);
}


void ColorButton::drawButton(QPainter *p)
{
    style().drawPrimitive(QStyle::PE_ButtonBevel, p, rect(), palette(),
                          isDown() ? QStyle::Style_Down : QStyle::Style_Raised);
    drawButtonLabel(p);

    if (hasFocus())
        style().drawPrimitive(QStyle::PE_FocusRect, p,
                              style().subRect(QStyle::SR_PushButtonFocusRect, this),
                              palette(), QStyle::Style_Default);
}


void ColorButton::drawButtonLabel(QPainter *p)
{
    QColor pen = (isEnabled() ?
                  (hasFocus() ? palette().active().buttonText() :
                   palette().inactive().buttonText())
                  : palette().disabled().buttonText());
    p->setPen( pen );
    p->setBrush(col);
    p->drawRect(width() / 4, height() / 4, width() / 2, height() / 2);
}


void ColorButton::dragEnterEvent(QDragEnterEvent *e)
{
    if (! QColorDrag::canDecode(e)) {
        e->ignore();
        return;
    }
}


void ColorButton::dragMoveEvent(QDragMoveEvent *e)
{
    if (! QColorDrag::canDecode(e)) {
        e->ignore();
        return;
    }

    e->accept();
}


void ColorButton::dropEvent(QDropEvent *e)
{
    if (! QColorDrag::canDecode(e)) {
        e->ignore();
        return;
    }

    QColor c;
    QColorDrag::decode(e, c);
    setColor(c);
    emit colorChanged(color());
}


void ColorButton::mousePressEvent(QMouseEvent *e)
{
    presspos = e->pos();
    mousepressed = TRUE;
    QAbstractButton::mousePressEvent(e);
}


void ColorButton::mouseReleaseEvent(QMouseEvent *e)
{
    mousepressed = FALSE;
    QAbstractButton::mouseReleaseEvent(e);
}


void ColorButton::mouseMoveEvent(QMouseEvent *e)
{
    if (! mousepressed)
        return;

    if ((presspos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        mousepressed = FALSE;
        setDown(FALSE);

        QColorDrag *cd = new QColorDrag(color(), this);
        cd->dragCopy();
    }
}

void ColorButton::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    drawButton(&p);
}


