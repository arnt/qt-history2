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

#include <qwidget.h>


class QLineEdit;
class QMultiLineEdit;
class QLabel;
class QPushButton;


class Composer : public QWidget
{
    Q_OBJECT

public:
    Composer( QWidget *parent = 0 );

private slots:
    void sendMessage();
    void enableSend();

private:
    QLineEdit *from, *to, *subject;
    QMultiLineEdit *message;
    QLabel * sendStatus;
    QPushButton * send;
};


#endif
