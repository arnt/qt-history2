/****************************************************************************
** $Id: //depot/qt/main/examples/lineedits/lineedits.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef LINEDITS_H
#define LINEDITS_H

#include <qvbox.h>

class QLineEdit;
class QComboBox;

class LineEdits : public QVBox
{
    Q_OBJECT

public:
    LineEdits( QWidget *parent = 0, const char *name = 0 );

protected:
    QLineEdit *lined1, *lined2;
    QComboBox *combo1, *combo2;

protected slots:
    void slotEchoChanged( int );
    void slotValidatorChanged( int );

};

#endif
