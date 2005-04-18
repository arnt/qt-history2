#include <QtGui>

#include "dragicon.h"
#include "dragwidget.h"

DragWidget::DragWidget(QWidget *parent)
    : QFrame(parent)
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);

    DragIcon *boatIcon = new DragIcon(this);
    boatIcon->setPixmap(QPixmap(":/images/boat.png"));
    boatIcon->move(20, 20);
    boatIcon->show();

    DragIcon *carIcon = new DragIcon(this);
    carIcon->setPixmap(QPixmap(":/images/car.png"));
    carIcon->move(120, 20);
    carIcon->show();

    DragIcon *houseIcon = new DragIcon(this);
    houseIcon->setPixmap(QPixmap(":/images/house.png"));
    houseIcon->move(20, 120);
    houseIcon->show();
}

void DragWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (children().contains(event->source())) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void DragWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        
        QPixmap pixmap;
        QPoint offset;
        dataStream >> pixmap >> offset;

        DragIcon *newIcon = new DragIcon(this);
        newIcon->setPixmap(pixmap);
        newIcon->move(event->pos() - offset);
        newIcon->show();

        if (children().contains(event->source())) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}
