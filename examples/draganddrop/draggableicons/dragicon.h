#ifndef DRAGICON_H
#define DRAGICON_H

#include <QLabel>

class DragIcon : public QLabel
{
public:
    DragIcon(QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif
