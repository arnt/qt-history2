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

#ifndef INTROSCREEN_H
#define INTROSCREEN_H

#include "demowidget.h"

class QTextDocument;
class QAbstractTextDocumentLayout;

class IntroScreen : public DemoWidget
{
public:
    IntroScreen(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    void resizeEvent(QResizeEvent *);

private:
    QString text;
    QPoint oldMousePoint;
    QTextDocument *textDocument;
    bool mouseDown;
    int lastAnimationStep, animationStepDelta;
};

#endif
