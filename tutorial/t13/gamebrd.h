/****************************************************************
**
** Definition of GameBoard class, Qt tutorial 13
**
****************************************************************/

#ifndef GAMEBRD_H
#define GAMEBRD_H

#include <qwidget.h>

class QPushButton;
class LCDRange;
class QLCDNumber;
class CannonField;

#include "lcdrange.h"
#include "cannon.h"


class GameBoard : public QWidget
{
    Q_OBJECT
public:
    GameBoard( QWidget *parent=0, const char *name=0 );
protected slots:
    void fire();
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
