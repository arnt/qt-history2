/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.h#10 $
**
** Definition of QTimer class
**
** Author  : Haavard Nord
** Created : 931111
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
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

signals:
    void	timeout();

protected:
    bool	event( QEvent * );

private:
    int		id;
    bool	single;
};


inline bool QTimer::isActive() const
{
    return id >= 0;
}


#endif // QTIMER_H
