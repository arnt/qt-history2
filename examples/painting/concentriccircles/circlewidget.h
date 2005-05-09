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

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include <QWidget>

class CircleWidget : public QWidget
{
    Q_OBJECT

public:
    CircleWidget(QWidget *parent = 0);

    void setFloatBased(bool floatBased);
    void setAntialiased(bool antialiased);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void nextAnimationFrame();

protected:
    void paintEvent(QPaintEvent *event);

private:
    bool floatBased;
    bool antialiased;
    int frameNo;
};

#endif
