/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qstatusbar.h#3 $
**
** Definition of QStatusBar class
**
** Created : 980316
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class QStatusBarPrivate;


class QStatusBar: public QWidget
{
    Q_OBJECT
public:
    QStatusBar( QWidget * parent = 0, const char * name = 0 );
    ~QStatusBar();

    void addWidget( QWidget *, int, bool = FALSE );
    void removeWidget( QWidget * );

public slots:
    void message( const char * );
    void message( const char *, int );
    void clear();

protected:
    void paintEvent( QPaintEvent * );

    void reformat();
    void hideOrShow();

private:
    QStatusBarPrivate * d;
};


#endif
