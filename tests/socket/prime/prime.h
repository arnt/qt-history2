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
