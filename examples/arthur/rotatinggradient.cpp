#include "rotatinggradient.h"

#include <qpainter.h>

RotatingGradient::RotatingGradient(QWidget *parent)
    : DemoWidget(parent)
{
    timeoutRate = 25;
    animationStep = 0;
    animationLoopStep = 511;
}

void RotatingGradient::timerEvent(QTimerEvent *e)
{
    m.rotate(1);
    DemoWidget::timerEvent(e);
}

void RotatingGradient::paintEvent(QPaintEvent *)
{
    if (!attributes)
        return;

    QPainter p(this);

    // We paint the whole widget so don't paint background if we are not transparent.
    if (attributes->alpha)
        fillBackground(&p);

    QPoint p1(-width()/4, 0);
    QPoint p2(+width()/4, 0);

    int cb = animationStep;
    if (cb > 255)
        cb = 511 - cb;

    p1 = m*p1 + QPoint(cb/2 + width()/2, height()/2);
    p2 = m*p2 + QPoint(width()/2, cb/2+height()/2);

    int alpha1 = 255;
    int alpha2 = attributes->alpha ? cb : 255;

    QBrush gradient = QBrush(p1, QColor(cb, 0, 255-cb, alpha1),
                             p2, QColor(0, 255-cb, cb, alpha2));

    p.setBrush(gradient);
    p.drawRect(0, 0, width(), height());

    p.setPen(Qt::black);
    p.drawLine(p1, p2);
}


QString RotatingGradient::description() const
{
    return "Description of rotating gradient...";
}
