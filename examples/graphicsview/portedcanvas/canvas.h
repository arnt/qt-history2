#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <q3popupmenu.h>
#include <q3mainwindow.h>
#include <q3intdict.h>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>

class BouncyLogo : public QGraphicsPixmapItem {
    void initSpeed();
public:
    BouncyLogo();
    void advance(int);
    int type() const;

    QPainterPath shape() const;

    void initPos();
private:
    qreal xvel;
    qreal yvel;
};


class FigureEditor : public QGraphicsView {
    Q_OBJECT

public:
    FigureEditor(QGraphicsScene&, QWidget* parent=0, const char* name=0, Qt::WindowFlags f=0);
    void clear();

signals:
    void status(const QString&);
};

class Main : public Q3MainWindow {
    Q_OBJECT

public:
    Main(QGraphicsScene&, QWidget* parent=0, const char* name=0, Qt::WindowFlags f=0);
    ~Main();

public slots:
    void help();

private slots:
    void aboutQt();
    void newView();
    void clear();
    void init();

    void addSprite();
    void addCircle();
    void addHexagon();
    void addPolygon();
    void addSpline();
    void addText();
    void addLine();
    void addRectangle();
    void addMesh();
    void addLogo();
    void addButterfly();

    void enlarge();
    void shrink();
    void rotateClockwise();
    void rotateCounterClockwise();
    void zoomIn();
    void zoomOut();
    void mirror();
    void moveL();
    void moveR();
    void moveU();
    void moveD();

    void print();

private:
    QGraphicsScene& canvas;
    FigureEditor *editor;

    Q3PopupMenu* options;
    QPrinter* printer;
    int dbf_id;
};

#endif
