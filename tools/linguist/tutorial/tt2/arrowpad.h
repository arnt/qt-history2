/****************************************************************
**
** Definition of ArrowPad class, translation tutorial 2
**
****************************************************************/

#ifndef ARROWPAD_H
#define ARROWPAD_H

#include <QGridWidget>

class ArrowPad : public QGridWidget
{
    Q_OBJECT

public:
    ArrowPad(QWidget *parent = 0);

private:
    void skip();
};

#endif
