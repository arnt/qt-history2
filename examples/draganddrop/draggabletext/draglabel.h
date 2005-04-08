#ifndef DRAGLABEL_H
#define DRAGLABEL_H

#include <QLabel>

class QDragEnterEvent;
class QDragMoveEvent;
class QFrame;

class DragLabel : public QLabel
{
public:
    DragLabel(const QString &text, QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QString labelText;
};

#endif
