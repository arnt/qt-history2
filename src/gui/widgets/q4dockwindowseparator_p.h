#ifndef Q4DOCKWINDOWSEPARATOR_P_H
#define Q4DOCKWINDOWSEPARATOR_P_H

#include <qwidget.h>

class Q4DockWindow;
class Q4DockWindowLayout;
class QPainter;

class Q4DockWindowSeparator : public QWidget
{
    Q_OBJECT

public:
    Q4DockWindowSeparator(Q4DockWindowLayout *l, QWidget *parent);

    QRect calcRect(const QPoint &point);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    Q4DockWindowLayout *layout;

    struct DragState {
	QPoint origin, last;
	QWidget *prevFocus;
    } *state;
};

#endif // Q4DOCKWINDOWSEPARATOR_P_H
