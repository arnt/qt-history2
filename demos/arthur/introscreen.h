#ifndef INTROSCREEN_H
#define INTROSCREEN_H

#include "demowidget.h"

class QTextDocument;

class IntroScreen : public DemoWidget
{
public:
    IntroScreen(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

private:
    QString text;
    QPoint oldMousePoint;
    QTextDocument *textDocument;
    bool mouseDown;
};

#endif // INTROSCREEN_H
