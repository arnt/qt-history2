/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LIFEDLG_H
#define LIFEDLG_H

#include <qtimer.h>
#include <qwidget.h>

class QSlider;
class QPushButton;
class QLabel;
class QComboBox;

#include "life.h"


class LifeTimer : public QTimer
{
    Q_OBJECT
public:
    LifeTimer( QWidget *parent );
    enum { MAXSPEED = 1000 };

public slots:
    void	setSpeed( int speed );
    void	pause( bool );

private:
    int		interval;
};


class LifeDialog : public QWidget
{
    Q_OBJECT
public:
    LifeDialog( int scale = 10, QWidget *parent = 0, const char *name = 0 );
public slots:
    void	getPattern( int );

protected:
    virtual void resizeEvent( QResizeEvent * e );

private:
    enum { TOPBORDER = 70, SIDEBORDER = 10 };

    LifeWidget	*life;
    QPushButton *qb;
    LifeTimer	*timer;
    QPushButton *pb;
    QComboBox	*cb;
    QLabel	*sp;
    QSlider	*scroll;
};


#endif // LIFEDLG_H
