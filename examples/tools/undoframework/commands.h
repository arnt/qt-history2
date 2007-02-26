#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>

#include "diagramitem.h"

class MoveCommand : public QUndoCommand
{
public:
    enum { Id = 1234 };

    MoveCommand(DiagramItem *diagramItem, const QPointF &oldPos,
		QUndoCommand *parent = 0);

    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *command);
    int id() const { return Id; }

private:
    DiagramItem *myDiagramItem;
    QPointF myOldPos;
    QPointF newPos;
};

class DeleteCommand : public QUndoCommand
{
public:
    DeleteCommand(QGraphicsScene *graphicsScene, QUndoCommand *parent = 0);

    void undo();
    void redo();

private:
    DiagramItem *myDiagramItem;
    QGraphicsScene *myGraphicsScene;
};

class AddCommand : public QUndoCommand
{
public:
    AddCommand(DiagramItem::DiagramType addType, QGraphicsScene *graphicsScene,
	       QUndoCommand *parent = 0);

    void undo();
    void redo();

private:
    DiagramItem *myDiagramItem;
    QGraphicsScene *myGraphicsScene;
    QPointF initialPosition;
};

QString createCommandString(DiagramItem *item, const QPointF &point);

#endif
