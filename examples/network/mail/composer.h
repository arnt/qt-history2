/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
