#ifndef PIECESLIST_H
#define PIECESLIST_H

#include <QListWidget>

class PiecesList : public QListWidget
{
    Q_OBJECT

public:
    PiecesList(QWidget *parent = 0);
    void addPiece(QPixmap pixmap, QPoint location);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void startDrag(Qt::DropActions supportedActions);
};

#endif
