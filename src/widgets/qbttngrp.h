/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbttngrp.h#10 $
**
** Definition of QButtonGroup class
**
** Author  : Eirik Eng
** Created : 950130
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBTTNGRP_H
#define QBTTNGRP_H

#include "qgrpbox.h"


class QButton;
class QButtonList;


class QButtonGroup : public QGroupBox
{
    Q_OBJECT
public:
    QButtonGroup( QWidget *parent=0, const char *name=0 );
    QButtonGroup( const char *title, QWidget *parent=0, const char *name=0 );
   ~QButtonGroup();

    int	 insert( QButton *, int id=-1 );
    void remove( QButton * );

signals:
    void pressed( int id );
    void released( int id );
    void clicked( int id );

protected slots:
    void buttonPressed();
    void buttonReleased();
    void buttonClicked();

private:
    void	init();
    QButtonList *buttons;
};


#endif // QBTTNGRP_H
