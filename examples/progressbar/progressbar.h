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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <qbuttongroup.h>
#include <qtimer.h>

class QRadioButton;
class QPushButton;
class QProgressBar;

class ProgressBar : public QButtonGroup
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
