#include <QtGui>

#include "commands.h"
#include "diagramitem.h"

MoveCommand::MoveCommand(DiagramItem *diagramItem, const QPointF &oldPos,
		         QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myDiagramItem = diagramItem;
    newPos = diagramItem->pos();
    myOldPos = oldPos;
    setText(QObject::tr("Move %1") 
	    .arg(createCommandString(myDiagramItem, oldPos)));
}

bool MoveCommand::mergeWith(const QUndoCommand *command)
{
    if (command->id() != Id || 
        static_cast<const MoveCommand *>(command)->myDiagramItem
        != myDiagramItem)
	return false;	
    
    return true;
}

void MoveCommand::undo()
{
    myDiagramItem->setPos(myOldPos);
    myDiagramItem->scene()->update();
}

void MoveCommand::redo()
{
    myDiagramItem->setPos(newPos);
}

DeleteCommand::DeleteCommand(QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    myGraphicsScene = scene;
    QList<QGraphicsItem *> list = myGraphicsScene->selectedItems();
    Q_ASSERT(list.size() == 1);
    list.first()->setSelected(false);
    myDiagramItem = static_cast<DiagramItem *>(list.first());
    setText(QObject::tr("Delete %1")
	    .arg(createCommandString(myDiagramItem, myDiagramItem->pos())));
}

void DeleteCommand::undo()
{
    myGraphicsScene->addItem(myDiagramItem);
    myGraphicsScene->update();
}

void DeleteCommand::redo()
{
    myGraphicsScene->removeItem(myDiagramItem);
}

AddCommand::AddCommand(DiagramItem::DiagramType addType,
                       QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    static int itemCount = 0;

    myGraphicsScene = scene;
    myDiagramItem = new DiagramItem(addType);
    initialPosition = QPointF((itemCount * 15) % int(scene->width()), 
			      (itemCount * 15) % int(scene->height())); 
    scene->update();
    ++itemCount;
    setText(QObject::tr("Add %1")
	    .arg(createCommandString(myDiagramItem, initialPosition)));
}

void AddCommand::undo()
{
    myGraphicsScene->removeItem(myDiagramItem);
    myGraphicsScene->update();
}

void AddCommand::redo()
{
    myGraphicsScene->addItem(myDiagramItem);
    myDiagramItem->setPos(initialPosition);
    myGraphicsScene->clearSelection();
    myGraphicsScene->update();
}

QString createCommandString(DiagramItem *item, const QPointF &pos)
{
    return QObject::tr("%1 at (%2, %3)") 
	    .arg(item->diagramType() == DiagramItem::Box ? "Box" : "Triangle")
	    .arg(pos.x()).arg(pos.y());
}
