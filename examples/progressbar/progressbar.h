/****************************************************************************
** $Id: //depot/qt/main/examples/progressbar/progressbar.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <qvbox.h>
#include <qtimer.h>

class QRadioButton;
class QPushButton;
class QProgressBar;

class ProgressBar : public QVBox
{
    Q_OBJECT

public:
    ProgressBar( QWidget *parent = 0, const char *name = 0 );

protected:
    QRadioButton *slow, *normal, *fast;
    QPushButton *start, *pause, *reset;
    QProgressBar *progress;
    QTimer timer;

protected slots:
    void slotStart();
    void slotReset();
    void slotTimeout();

};

#endif
