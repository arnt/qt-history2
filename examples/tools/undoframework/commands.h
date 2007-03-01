/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
