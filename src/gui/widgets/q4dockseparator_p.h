#ifndef Q4DOCKSEPARATOR_P_H
#define Q4DOCKSEPARATOR_P_H

#include <qwidget.h>

class Q4DockWindowLayout;

class Q4DockSeparator : public QWidget
{
public:
    Q4DockSeparator(Q4DockWindowLayout *dock, QWidget *parent);

    void setDock(Q4DockWindowLayout *d);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    Q4DockWindowLayout *dock;
    Orientation orientation;

    struct DragState {
	QPoint origin;
	QWidget *prevFocus;
    } *state;
};

#endif // Q4DOCKSEPARATOR_P_H
