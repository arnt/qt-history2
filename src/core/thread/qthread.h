/****************************************************************************
**
** Definition of QThread class.
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

#ifndef QTHREAD_H
#define QTHREAD_H

#ifndef QT_H
#include <qnamespace.h>
#endif // QT_H

#include <limits.h>

struct QThreadInstance;
class QObject;
class QEvent;

class Q_CORE_EXPORT QThread
{
public:
#ifdef QT_COMPAT
    static QT_COMPAT void postEvent( QObject *,QEvent * );
#endif

    static Qt::HANDLE currentThread();

    static void initialize();
    static void cleanup();

    static void exit();

    QThread( unsigned int stackSize = 0 );
    virtual ~QThread();

    // default argument causes thread to block indefinately
    bool wait( unsigned long time = ULONG_MAX );

    enum Priority {
	IdlePriority,

	LowestPriority,
	LowPriority,
	NormalPriority,
	HighPriority,
	HighestPriority,

	TimeCriticalPriority,

	InheritPriority
    };

    void start( Priority = InheritPriority );
    void terminate();

    bool finished() const;
    bool running() const;

protected:
    virtual void run() = 0;

    static void sleep( unsigned long );
    static void msleep( unsigned long );
    static void usleep( unsigned long );

private:
    QThreadInstance * d;
    friend struct QThreadInstance;

#if defined(Q_DISABLE_COPY)
    QThread( const QThread & );
    QThread &operator=( const QThread & );
#endif // Q_DISABLE_COPY
};

#endif // QTHREAD_H
