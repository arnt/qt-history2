#ifndef INTROSCREEN_H
#define INTROSCREEN_H

#include "demowidget.h"

#include <qtextcursor.h>

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
    QPixmap topGradient;
    QPixmap bottomGradient;
    QPoint oldMousePoint;
    QTextDocument *textDocument;
    QTextCursor textCursor;
};

#endif // INTROSCREEN_H
