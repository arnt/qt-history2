/****************************************************************************
** $Id: //depot/qt/main/tests/socket/prime/prime.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef PRIME_H
#define PRIME_H

class QLineEdit;
class QTextStream;
class QLabel;

#include <qwidget.h>
#include <qsocket.h>


class Prime : public QWidget
{
    Q_OBJECT
public:
    Prime( QWidget * parent = 0, const char * name = 0 );
    ~Prime();

private slots:
    void dataArrived();
    void timeToUpdate();
    void makeConnection();

private:
    QLineEdit * input;
    QLabel * output;
    QSocket * c;
    QTextStream * s;
};

#endif
