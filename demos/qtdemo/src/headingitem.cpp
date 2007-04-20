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

#include "headingitem.h"
#include "colors.h"

HeadingItem::HeadingItem(const QString &text, QGraphicsScene *scene, QGraphicsItem *parent)
    : DemoItem(scene, parent)
{
    this->text = text;
    this->noSubPixeling = true;
}

QImage *HeadingItem::createImage(const QMatrix &matrix) const
{
    float sx = qMin(matrix.m11(), matrix.m22());
    float sy = matrix.m22() < sx ? sx : matrix.m22();
    QFontMetrics fm(Colors::headingFont());
    
    float w = fm.width(this->text) + 1;
    float h = fm.height();
    int xShadow = 3.0f;
    int yShadow = 3.0f;

    QImage *image = new QImage(int((w + xShadow) * sx), int((h + yShadow) * sy), QImage::Format_ARGB32_Premultiplied);
    image->fill(QColor(0, 0, 0, 0).rgba());
    QPainter painter(image);
    painter.setFont(Colors::headingFont());
    painter.scale(sx, sy);   
    
    //draw shadow
    QLinearGradient brush_shadow(xShadow, yShadow, w, yShadow);        
    brush_shadow.setSpread(QLinearGradient::PadSpread);
    if (Colors::useEightBitPalette)
        brush_shadow.setColorAt(0.0f, QColor(0, 0, 0));
    else
        brush_shadow.setColorAt(0.0f, QColor(0, 0, 0, 100));
    QPen pen_shadow; 
    pen_shadow.setBrush(brush_shadow);
    painter.setPen(pen_shadow);
    painter.drawText(xShadow, yShadow, w, h, Qt::AlignLeft, this->text);

    // draw text
    QLinearGradient brush_text(0, 0, w, w);        
    brush_text.setSpread(QLinearGradient::PadSpread);
    brush_text.setColorAt(0.0f, QColor(255, 255, 255));
    brush_text.setColorAt(0.2f, QColor(255, 255, 255));
    brush_text.setColorAt(0.5f, QColor(190, 190, 190));
    QPen pen_text; 
    pen_text.setBrush(brush_text);
    painter.setPen(pen_text);
    painter.drawText(0, 0, w, h, Qt::AlignLeft, this->text);
    return image;
}


void HeadingItem::animationStarted(int)
{
    this->noSubPixeling = false;
}


void HeadingItem::animationStopped(int)
{
    this->noSubPixeling = true;
}
