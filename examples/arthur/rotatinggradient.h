#ifndef ROTATINGGRADIENT_H
#define ROTATINGGRADIENT_H

#include "demowidget.h"

#include <qwmatrix.h>

class RotatingGradient : public DemoWidget
{
public:
    RotatingGradient(QWidget *parent=0);
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *e);
private:
    QWMatrix matrix;
    QPixmap pixmap;
};

#endif // ROTATINGGRADIENT_H
