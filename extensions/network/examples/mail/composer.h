/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef COMPOSER_H
#define COMPOSER_H

#include <qvbox.h>

class QLineEdit;
class QMultiLineEdit;

class Composer : public QVBox
{
    Q_OBJECT
    
public:
    Composer( QWidget *parent = 0 );

private slots:
    void sendMessage();
    void enableAll();
    
private:
    QLineEdit *from, *to, *cc, *bcc, *server, *port, *subject;
    QMultiLineEdit *message;
    
};

#endif
