#ifndef DEMOWIDGET_H
#define DEMOWIDGET_H

#include "attributes.h"

#include <qwidget.h>

class DemoWidget : public QWidget
{
public:
    DemoWidget(QWidget *w=0);

    virtual QString description() const { return QString(); }

    virtual void startAnimation();
    virtual void stopAnimation();

    void setAttributes(Attributes *attr) { attributes = attr; }

    void timerEvent(QTimerEvent *e);
    QSize sizeHint() const;

    void fillBackground(QPainter *p);

    double xfunc(double t);
    double yfunc(double t);

protected:
    int timeoutRate;
    int animationStep;
    int animationLoopStep;

    Attributes *attributes;

    double a, b, c, d;

private:
    int timerId;
};

#endif // DEMOWIDGET_H
