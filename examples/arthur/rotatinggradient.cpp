#include "rotatinggradient.h"

#include <qpainter.h>

RotatingGradient::RotatingGradient(QWidget *parent)
    : DemoWidget(parent)
{
    timeoutRate = 25;
    animationStep = 0;
}

void RotatingGradient::timerEvent(QTimerEvent *e)
{
    matrix.rotate(1);
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

    // Define a value that will move from 0 to 255 then down to 0
    // again. fade in, fade out
    int fade = animationStep % 512;
    if (fade > 255)
        fade = 511 - fade;

    // Create two points that rotate around origo at radius w/4 and
    // move them to the center of the widget. We also add the fade
    // factor to the widgets to tilt them away from the static circle
    // motion.
    QPoint p1 = matrix*QPoint(-w/4, 0) + QPoint(fade/2 + width()/2, height()/2);
    QPoint p2 = matrix*QPoint(w/4, 0)  + QPoint(width()/2, fade/2+height()/2);

    int alpha1 = 255;
    int alpha2 = attributes->alpha ? fade : 255;

    // Define the gradient brush. The colors will fade from blue and green to
    // red and blue
    QBrush gradient = QBrush(p1, QColor(fade, 0, 255-fade, alpha1),
                             p2, QColor(0, 255-fade, fade, alpha2));
    p.setBrush(gradient);

    p.drawRect(0, 0, width(), height());
}
