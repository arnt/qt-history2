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

#include "colorbutton.h"

#include <qapplication.h>
#include <qevent.h>
#include <qcolordialog.h>
#include <qpainter.h>
#include <qdragobject.h>
#include <qstyle.h>
#include <qstyleoption.h>

ColorButton::ColorButton(QWidget *parent)
    : QAbstractButton(parent), mousepressed(FALSE)
{
    setAcceptDrops(TRUE);
    col = Qt::black;
    connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}


ColorButton::ColorButton(const QColor &c, QWidget *parent)
    : QAbstractButton(parent)
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
    QStyleOptionButton buttonOptions(0);
    buttonOptions.init(this);
    buttonOptions.features = QStyleOptionButton::None;
    buttonOptions.rect = rect();
    buttonOptions.palette = palette();
    buttonOptions.state = (isDown() ? QStyle::Style_Down : QStyle::Style_Raised);
    style().drawPrimitive(QStyle::PE_ButtonBevel, &buttonOptions, p, this);

    drawButtonLabel(p);

    QStyleOptionFocusRect frectOptions(0);
    frectOptions.init(this);
    frectOptions.rect = style().subRect(QStyle::SR_PushButtonFocusRect, &buttonOptions, this);
    if (hasFocus())
        style().drawPrimitive(QStyle::PE_FocusRect, &frectOptions, p, this);
}


void ColorButton::drawButtonLabel(QPainter *p)
{
    QPalette::ColorGroup cg =
        (isEnabled() ? (hasFocus() ? QPalette::Active : QPalette::Inactive) : QPalette::Disabled);

    p->setPen(palette().color(cg, QPalette::ButtonText));
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

void ColorButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawButton(&p);
}


