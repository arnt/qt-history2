#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <qpopupmenu.h>
#include <qmainwindow.h>
#include <qintdict.h>
#include <qcanvas.h>

class SpaceShip : public QCanvasSprite {
public:
    SpaceShip();
    void forward();
};


class FigureEditor : public QCanvasView {
    Q_OBJECT

public:
    FigureEditor(QCanvas&, QWidget* parent=0, const char* name=0, WFlags f=0);

protected:
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);

signals:
    void status(const QString&);

private:
    QCanvasItem* moving;
    QPoint moving_start;
};

class Main : public QMainWindow {
    Q_OBJECT

    QCanvas canvas;
    FigureEditor *editor;

public:
    Main();

private slots:
    void addSprite();
    void addCircle();
    void addHexagon();
    void addRectangle();
};

#endif
