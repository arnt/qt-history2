#ifndef QDOCKWINDOWSEPARATOR_P_H
#define QDOCKWINDOWSEPARATOR_P_H

#include <qwidget.h>

class QDockWindow;
class QDockWindowLayout;
class QPainter;

class QDockWindowSeparator : public QWidget
{
    Q_OBJECT

public:
    QDockWindowSeparator(QDockWindowLayout *l, QWidget *parent);

    QRect calcRect(const QPoint &point);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    QDockWindowLayout *layout;

    struct DragState {
	QPoint origin, last;
	QWidget *prevFocus;
    } *state;
};

#endif // QDOCKWINDOWSEPARATOR_P_H
