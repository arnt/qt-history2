/****************************************************************************
**
** Definition of QTimer class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTIMER_H
#define QTIMER_H

#ifndef QT_H
#include "qbasictimer.h" // conceptual inheritance
#include "qobject.h"
#endif // QT_H


class Q_CORE_EXPORT QTimer : public QObject
{
    Q_OBJECT

public:
    QTimer( QObject *parent=0, const char *name=0 );
    ~QTimer();

    static void singleShot( int msec, QObject *receiver, const char *member );

    bool	isActive() const;
    int		timerId() const	{ return id; }

public slots:
    int		start( int msec, bool sshot = FALSE );
    void	changeInterval( int msec );
    void	stop();

signals:
    void	timeout();

protected:
    bool	event( QEvent * );

private:
    int id;
    uint single : 1;
    uint nulltimer : 1;

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
