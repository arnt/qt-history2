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

#ifndef DEMOWIDGET_H
#define DEMOWIDGET_H

#include "attributes.h"

#include <qwidget.h>
#include <qbasictimer.h>
#include <qdatetime.h>

class DemoWidget : public QWidget
{
    Q_OBJECT
public:
    DemoWidget(QWidget *w=0);

    virtual void startAnimation();
    virtual void stopAnimation();

    void updateBackground();
    virtual void resetState() { update(); }

    void setAttributes(Attributes *attr) { attributes = attr; }

    void timerEvent(QTimerEvent *e);
    QSize sizeHint() const;

    void drawBackground(QPainter *p);

    double xfunc(double t);
    double yfunc(double t);

    inline Attributes *attribs() { return attributes; }

protected:
    QPixmap backgroundPixmap;
    int timeoutRate;
    int animationStep() const
    { return step.elapsed() / timeoutRate; }

    Attributes *attributes;

    double a, b, c, d;

    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void resizeEvent(QResizeEvent *)
    {
        backgroundPixmap = QPixmap(size());
        updateBackground();
    }

private:
    void fillBackground(QPainter *p);
    QBasicTimer animationTimer;
    QTime step;
};

#endif
