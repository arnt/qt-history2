/****************************************************************************
** $Id: //depot/qt/main/examples/buttons_groups/buttons_groups.h#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef BUTTONS_GROUPS_H
#define BUTTONS_GROUPS_H

#include <qvbox.h>

class QCheckBox;
class QRadioButton;

class ButtonsGroups : public QVBox
{
    Q_OBJECT

public:
    ButtonsGroups( QWidget *parent = 0, const char *name = 0 );

protected:
    QCheckBox *state;
    QRadioButton *rb21, *rb22, *rb23;

protected slots:    
    void slotChangeGrp3State();

};

#endif
