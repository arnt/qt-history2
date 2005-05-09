/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************
**
** Definition of GameBoard class, Qt tutorial 14
**
****************************************************************/

#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QWidget>

class QLCDNumber;
class CannonField;

class GameBoard : public QWidget
{
    Q_OBJECT

public:
    GameBoard(QWidget *parent = 0);

protected slots:
    void fire();
    void hit();
    void missed();
    void newGame();

private:
    QLCDNumber *hits;
    QLCDNumber *shotsLeft;
    CannonField *cannonField;
};

#endif // GAMEBOARD_H
