#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class RenderArea;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private:
    RenderArea *rectArea;
    RenderArea *roundRectArea;
    RenderArea *ellipseArea;
    RenderArea *pieArea;
    RenderArea *polylineArea;
    RenderArea *polygonArea;
    RenderArea *textArea;
    RenderArea *bezierArea;
    RenderArea *compositionArea;
};

#endif
