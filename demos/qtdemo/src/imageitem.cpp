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

#include "imageitem.h"
#include "colors.h"

ImageItem::ImageItem(const QString &path, int maxWidth, int maxHeight, QGraphicsScene *scene, QGraphicsItem *parent)
    : DemoItem(scene, parent)
{
    this->path = path;
    this->maxWidth = maxWidth;
    this->maxHeight = maxHeight;
}

QImage *ImageItem::createImage(const QMatrix &matrix) const
{
    QImage *original = new QImage();
    if (!original->load(this->path))
        return original; // nothing we can do about it...
    
    QPoint size = matrix.map(QPoint(this->maxWidth, this->maxHeight));
    int w = size.x();
    int h = size.y();

    if (original->height() <= h && original->width() <= w)
        return original;
    
    w = qMin(w, original->width());
    h = qMin(h, original->height());
    
    // Create a new image with correct size, and draw original on it
    QImage *image = new QImage(w, h, QImage::Format_ARGB32_Premultiplied);
    image->fill(QColor(0, 0, 0, 0).rgba());
    QPainter painter(image);
    painter.drawImage(0, 0, *original);

    // Blur out edges
    int blur = 30;
    if (h < original->height()){
        QLinearGradient brush1(0, h - blur, 0, h);
        brush1.setSpread(QGradient::PadSpread);
        brush1.setColorAt(0.0, QColor(0, 0, 0, 0));
        brush1.setColorAt(1.0, Colors::sceneBg1);
        painter.fillRect(0, h - blur, original->width(), h, brush1);
    }
    if (w < original->width()){
        QLinearGradient brush2(w - blur, 0, w, 0);
        brush2.setSpread(QGradient::PadSpread);
        brush2.setColorAt(0.0, QColor(0, 0, 0, 0));
        brush2.setColorAt(1.0, Colors::sceneBg1);
        painter.fillRect(w - blur, 0, w, original->height(), brush2);
    }
  
    delete original;
    return image;
}



