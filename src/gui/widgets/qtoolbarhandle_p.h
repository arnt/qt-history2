#ifndef QTOOLBARHANDLE_P_H
#define QTOOLBARHANDLE_P_H

#include <qwidget.h>

class QToolBar;

class QToolBarHandle : public QWidget
{
public:
    QToolBarHandle(QToolBar *parent);

    Qt::Orientation orientation();

    QSize sizeHint() const;

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    struct DragState {
	QPoint offset;
	bool canDrop;
    };
    DragState *state;
};

#endif // QTOOLBARHANDLE_P_H
