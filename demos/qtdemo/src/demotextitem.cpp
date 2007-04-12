/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "demotextitem.h"
#include "colors.h"

DemoTextItem::DemoTextItem(const QString &text, const QFont &font, const QColor &textColor,
                            float textWidth, QGraphicsScene *scene, QGraphicsItem *parent, TYPE type, const QColor &bgColor)
    : DemoItem(scene, parent)
{
    this->type = type;
    this->text = text;
    this->font = font;
    this->textColor = textColor;
    this->bgColor = bgColor;
    this->textWidth = textWidth;
}

void DemoTextItem::setText(const QString &text)
{
    this->text = text;
    this->update();
}

QImage *DemoTextItem::createImage(const QMatrix &matrix) const
{
    if (this->type == DYNAMIC_TEXT)
        return 0;
    
    QGraphicsTextItem textItem(0, 0);
    textItem.setHtml(this->text);
    textItem.setTextWidth(this->textWidth);
    textItem.setFont(this->font);
    textItem.setDefaultTextColor(this->textColor);
    QRect scaledRect = matrix.mapRect(textItem.boundingRect().toRect());
    QImage *image = new QImage(scaledRect.width(), scaledRect.height(), QImage::Format_ARGB32_Premultiplied);
    image->fill(QColor(0, 0, 0, 0).rgba());
    QPainter painter(image);
    painter.scale(matrix.m11(), matrix.m22());
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    QStyleOptionGraphicsItem style;
    if (this->bgColor.isValid()){
        painter.setBrush(this->bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawRoundRect(scaledRect, 10, 10);
    }
    textItem.paint(&painter, &style, 0);
    return image;
}

QRectF DemoTextItem::boundingRect() const
{
    if (this->type == STATIC_TEXT)
        return DemoItem::boundingRect();
    return QRectF(0, 0, 50, 20); // Sorry for using magic number
}

void DemoTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (this->type == STATIC_TEXT)
        return DemoItem::paint(painter, option, widget);

    painter->setPen(this->textColor);
    painter->drawText(0, 0, this->text);
}


