/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.h#1 $
**
** Definition of QTimer class
**
** Author  : Haavard Nord
** Created : 931111
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QTIMER_H
#define QTIMER_H

#include "qobject.h"


class QTimer : public QObject			// timer class
{
    Q_OBJECT
public:
    QTimer();
   ~QTimer();

    bool isActive() const { return id >= 0; }	// timer is active

    int	 start( long msec, bool sshot = FALSE );// start timer
    void changeInterval( long msec );		// change timer interval
    void stop();				// stop timer

signals:
    void timeout( int );			// timer expires

private:
    bool event( QEvent * );
    int	 id;
    bool single;
};


#endif // QTIMER_H
