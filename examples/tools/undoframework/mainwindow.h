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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QToolBar;
class QMenu;
class QUndoStack;
class QUndoView;
class DiagramScene;
class DiagramItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void itemMoved(DiagramItem *movedDiagram, const QPointF &moveStartPosition);

private slots:
    void deleteItem();
    void addBox();
    void addTriangle();
    void about();
    void itemMenuAboutToShow();
    void itemMenuAboutToHide();

private:
    void createActions();
    void createMenus();
    void createUndoView();

    QAction *deleteAction;
    QAction *addBoxAction;
    QAction *addTriangleAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *exitAction;
    QAction *aboutAction; 

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *itemMenu;
    QMenu *helpMenu;
 
    DiagramScene *diagramScene;
    QUndoStack *undoStack;
    QUndoView *undoView;
};

#endif
