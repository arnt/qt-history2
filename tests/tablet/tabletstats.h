/****************************************************************************
** $Id: $
**
** Copyright ( C ) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef _TABLET_STATS_
#define _TABLET_STATS_

#include <qmainwindow.h>
#include "canvas.h"

class QLabel;

class StatsCanvas : public Canvas
{
    Q_OBJECT
public:
    StatsCanvas( QWidget *parent = 0, const char* name = 0 ) : Canvas( parent, name )
    {};
    ~StatsCanvas() {};
signals:
    void signalNewPressure( int newPressure );
    void signaleNewTilt( int newTX, int newTY );
    void signalNewDev( int newDev );
    void signalNewLoc( int newX, int newY );

protected:
    void tabletEvent( QTabletEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
};

class TabletStats : public QWidget
{
    Q_OBJECT
public:
    TabletStats( QWidget *parent, const char* name );
    ~TabletStats();

private slots:
    void slotPressureChanged( int newPress );
    void slotTiltChanged( int newTiltX, int newTiltY );
    void slotDevChanged( int newDev );
    void slotLocationChanged( int newX, int newY );

protected:
    QLabel *lblXTilt,
	   *lblYTilt,
	   *lblPressure,
	   *lblDev;
    StatsCanvas *statCan;
};

#endif
