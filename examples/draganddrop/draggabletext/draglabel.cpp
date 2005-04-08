#include <QtGui>

#include "draglabel.h"

DragLabel::DragLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setFrameShape(QFrame::Panel);
    setFrameShadow(QFrame::Raised);
}

void DragLabel::mousePressEvent(QMouseEvent *event)
{
    QString plainText = text(); // for quoting purposes

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(plainText);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(event->pos() - rect().topLeft());

    Qt::DropAction dropAction = drag->start(Qt::CopyAction | Qt::MoveAction);

    if (dropAction == Qt::MoveAction) {
        close();
        update();
    }
}
