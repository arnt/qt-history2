/****************************************************************************
** Definition of GameBoard class, Qt tutorial 13
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef GAMEBRD_H
#define GAMEBRD_H

#include <qpushbt.h>
#include <qfont.h>

#include "lcdrange.h"
#include "cannon.h"


class GameBoard : public QWidget
{
    Q_OBJECT
public:
    GameBoard( QWidget *parent=0, const char *name=0 );
protected slots:
    void shootClicked();
    void hit();
    void missed();
    void newGame();
private:
    QPushButton *quit;
    QPushButton *shoot;
    QPushButton *reStart;
    LCDRange    *angle;
    LCDRange    *force;
    QLCDNumber  *hits;
    QLCDNumber  *shotsLeft;
    CannonField *cannon;
};

#endif // GAMEBRD_H
