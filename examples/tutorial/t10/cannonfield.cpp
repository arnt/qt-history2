/****************************************************************
**
** Implementation CannonField class, Qt tutorial 10
**
****************************************************************/

#include <QPaintEvent>
#include <QPainter>

#include "cannonfield.h"

CannonField::CannonField(QWidget *parent)
    : QWidget(parent)
{
    ang = 45;
    f = 0;
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
    update(cannonRect());
    emit angleChanged(ang);
}

void CannonField::setForce(int force)
{
    if (force < 0)
        force = 0;
    if (f == force)
        return;
    f = force;
    emit forceChanged(f);
}

void CannonField::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::blue);

    painter.translate(0, height());
    painter.drawPie(QRect(-35, -35, 70, 70), 0, 90 * 16);
    painter.rotate(-ang);
    painter.drawRect(QRect(33, -4, 15, 8));
}

QRect CannonField::cannonRect() const
{
    QRect result(0, 0, 50, 50);
    result.moveBottomLeft(rect().bottomLeft());
    return result;
}
