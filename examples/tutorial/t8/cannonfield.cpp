/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************
**
** Implementation CannonField class, Qt tutorial 8
**
****************************************************************/

#include <QPainter>

#include "cannonfield.h"

CannonField::CannonField(QWidget *parent)
    : QWidget(parent)
{
    ang = 45;
    setPalette(QPalette(QColor(250, 250, 200)));
}

void CannonField::setAngle(int angle)
{
    if (angle < 5)
        angle = 5;
    if (angle > 70)
        angle = 70;
    if (ang == angle)
        return;
    ang = angle;
    update();
    emit angleChanged(ang);
}

void CannonField::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.drawText(200, 200, "Angle = " + QString::number(ang));
}
