/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbttngrp.h#1 $
**
** Definition of QButtonGroup class
**
** Author  : Eirik Eng
** Created : 950130
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBUTTGRP_H
#define QBUTTGRP_H

#include "qwidget.h"

class QListM_QButtonItem;
class QButton;

class QButtonGroup : public QWidget
{
    Q_OBJECT
public:
    QButtonGroup( QWidget *parent=0, const char *name=0 );
    ~QButtonGroup();
    void insert( QButton *, int id=-1, int index=-1 );

signals:
    void selected ( int id );

slots:
    void buttonClicked();

private:
    void init();

    QListM_QButtonItem *buttons;
};


#endif // QBUTTGRP_H
