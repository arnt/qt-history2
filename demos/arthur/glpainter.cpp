/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qgl.h>
#include <qpainter.h>
#include <qlayout.h>

#include "glpainter.h"

class GLWidget : public QGLWidget
{
public:
    GLWidget(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);

private:
    DemoWidget *dw;
    int step;
};

extern void drawPrimitives(DemoWidget *dw, QPainter *p, int count, double distance, int step);
extern void drawShadedCube(DemoWidget *dw, QPainter *p, int iterations, int spread, int step);

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    dw = qt_cast<DemoWidget *>(parent);
    step = 0;
    Q_ASSERT(dw);
}

void GLWidget::timerEvent(QTimerEvent *)
{
    update();
    ++step;
}

void GLWidget::paintEvent(QPaintEvent *)
{
    static int i = 0;
    QPainter p(this);

    p.setBrush(QBrush(QPoint(0,0), QColor(20+60, 60+60, 190+60), QPoint(width(), height()), Qt::white));
    p.drawRect(0, 0, width(), height());

    p.translate(width()/2, height()/2);
    p.rotate(++i % 360);
    p.shear(dw->xfunc(i*0.8), dw->yfunc(i*0.8));
    p.translate(-width()/2, -height()/2);
    dw->fillBackground(&p);
    p.resetXForm();

    drawShadedCube(dw, &p, 2, 5, step);
    drawPrimitives(dw, &p, 150, 0.3, step);
}

GLPainter::GLPainter(QWidget *parent)
    : DemoWidget(parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setMargin(0);
    glwidget = new GLWidget(this);
    layout->addWidget(glwidget);
}

void GLPainter::startAnimation()
{
    animTimer.start(50, glwidget);
}

void GLPainter::stopAnimation()
{
    animTimer.stop();
}
