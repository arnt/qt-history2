#ifndef DRAGICON_H
#define DRAGICON_H

#include <QLabel>

class QDragEnterEvent;
class QDragMoveEvent;
class QFrame;

class DragIcon : public QLabel
{
public:
    DragIcon(QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif
