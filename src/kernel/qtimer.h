/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.h#26 $
**
** Definition of QTimer class
**
** Created : 931111
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTIMER_H
#define QTIMER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QTimer : public QObject
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
#if defined(Q_DISABLE_COPY)
    QTimer( const QTimer & );
    QTimer &operator=( const QTimer & );
#endif
};


inline bool QTimer::isActive() const
{
    return id >= 0;
}


#endif // QTIMER_H
