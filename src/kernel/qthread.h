/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread.h#16 $
**
** Definition of QThread class
**
** Created : 931107
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


#ifndef QTHREAD_H
#define QTHREAD_H

#ifdef QT_THREAD_SUPPORT

#include <qglobal.h>
#include <qobject.h>
#include <qevent.h>
#include <qdatetime.h>

class QMutexPrivate;
class QThreadPrivate;
class QThreadEventPrivate;

typedef unsigned long MUTEX_HANDLE;
typedef unsigned long THREAD_HANDLE;
typedef unsigned long THREADEVENT_HANDLE;

class Q_EXPORT QMutex {

 public:

    QMutex();
    ~QMutex();
    void lock();
    void unlock();
    MUTEX_HANDLE handle();
    bool locked();

private:

    QMutexPrivate * d;

#if defined(Q_DISABLE_COPY)
    QMutex( const QMutex & );
    QMutex &operator=( const QMutex & );
#endif

};

class Q_EXPORT QThread {

public:

    static THREAD_HANDLE currentThread();
    static void postEvent( QObject *,QEvent * );

    static void exit();

    QThread();
    virtual ~QThread();
    void wait();
    void start();
    virtual void run();
    THREAD_HANDLE handle();
    bool running();

    void runWrapper();

private:

    QThreadPrivate * d;

#if defined(Q_DISABLE_COPY)
    QThread( const QThread & );
    QThread &operator=( const QThread & );
#endif

};

class QThreadDataPrivate;

class Q_EXPORT QThreadData {

public:

    QThreadData();
    ~QThreadData();
    void setData(void *);
    void * data();

private:

    QThreadDataPrivate * d;

};

class Q_EXPORT QThreadEvent {

public:

    QThreadEvent();
    ~QThreadEvent();
    void wait();
    void wait(const QTime &);
    void wakeOne();
    void wakeAll();
    THREADEVENT_HANDLE handle();

private:

    QThreadEventPrivate * d;

#if defined(Q_DISABLE_COPY)
    QThreadEvent( const QThreadEvent & );
    QThreadEvent &operator=( const QThreadEvent & );
#endif

};

#endif

#endif




