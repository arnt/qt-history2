#ifndef QDOCKSEPARATOR_P_H
#define QDOCKSEPARATOR_P_H

#include <qwidget.h>

class QDockWindowLayout;

class QDockSeparator : public QWidget
{
public:
    QDockSeparator(QDockWindowLayout *dock, QWidget *parent);

    void setDock(QDockWindowLayout *d);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    QDockWindowLayout *dock;
    Orientation orientation;

    struct DragState {
	QPoint origin;
	QWidget *prevFocus;
    } *state;
};

#endif // QDOCKSEPARATOR_P_H
