/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.h#17 $
**
** Definition of QTimer class
**
** Created : 931111
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTIMER_H
#define QTIMER_H

#include "qobject.h"


class QTimer : public QObject
{
    Q_OBJECT
public:
    QTimer( QObject *parent=0, const char *name=0 );
   ~QTimer();

    bool	isActive() const;

    int		start( int msec, bool sshot = FALSE );
    void	changeInterval( int msec );
    void	stop();

    static void singleShot( int msec, QObject *receiver, const char *member );

signals:
    void	timeout();

protected:
    bool	event( QEvent * );

private:
    int		id;
    bool	single;

private:	// Disabled copy constructor and operator=
    QTimer( const QTimer & ) {}
    QTimer &operator=( const QTimer & ) { return *this; }
};


inline bool QTimer::isActive() const
{
    return id >= 0;
}


#endif // QTIMER_H
