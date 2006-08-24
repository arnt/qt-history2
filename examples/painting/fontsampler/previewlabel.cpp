/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include "previewlabel.h"

PreviewLabel::PreviewLabel(QWidget *parent)
    : QWidget(parent)
{
}

void PreviewLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.fillRect(event->rect(), QColor(224,224,224));
    painter.drawPixmap(0, 0, pixmap);
    painter.end();
}

void PreviewLabel::setPixmap(const QPixmap &pixmap)
{
    this->pixmap = pixmap;
}
