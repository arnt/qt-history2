#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <qpopupmenu.h>
#include <qmainwindow.h>
#include <qintdict.h>
#include <qcanvas.h>

class SpaceShip : public QCanvasSprite {
    void initSpeed();
public:
    SpaceShip();
    void advance(int);
    int rtti() const;
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

public:
    Main();

private slots:
    void addSprite();
    void addCircle();
    void addHexagon();
    void addPolygon();
    void addRectangle();
    void toggleDoubleBuffer();

private:
    QCanvas canvas;
    FigureEditor *editor;

    QPopupMenu* options;
    int dbf_id;
};

#endif
