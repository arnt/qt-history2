/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <qdockwindow.h>

class QAction;
class QActionGroup;
class QMenu;

class ColorSwatch : public QDockWindow
{
    Q_OBJECT

    QAction *closableAction;
    QAction *movableAction;
    QAction *floatableAction;
    QAction *topLevelAction;

    QActionGroup *allowedAreasActions;
    QAction *allowLeftAction;
    QAction *allowRightAction;
    QAction *allowTopAction;
    QAction *allowBottomAction;

    QActionGroup *areaActions;
    QAction *leftAction;
    QAction *rightAction;
    QAction *topAction;
    QAction *bottomAction;

public:
    ColorSwatch(const QString &colorName, QWidget *parent = 0, Qt::WFlags flags = 0);

    QMenu *menu;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void polishEvent(QEvent *);

private:
    void allow(Qt::DockWindowArea area, bool allow);
    void place(Qt::DockWindowArea area, bool place);

private slots:
    void changeClosable(bool on);
    void changeMovable(bool on);
    void changeFloatable(bool on);

    void changeTopLevel(bool topLevel);

    void allowLeft(bool a);
    void allowRight(bool a);
    void allowTop(bool a);
    void allowBottom(bool a);

    void placeLeft(bool p);
    void placeRight(bool p);
    void placeTop(bool p);
    void placeBottom(bool p);
};

#endif
