/****************************************************************************
** $Id: //depot/qt/main/examples/rangecontrols/rangecontrols.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef RANGECONTROLS_H
#define RANGECONTROLS_H

#include <qvbox.h>

class QCheckBox;
class QDial;

class RangeControls : public QVBox
{
    Q_OBJECT

public:
    RangeControls( QWidget *parent = 0, const char *name = 0 );

private slots:
    void toggleShowNotches();
    void toggleWrapping();
    
private:
    QCheckBox *notches, *wrapping;
    QDial *dial;
    
};

#endif
