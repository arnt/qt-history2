/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread.h#16 $
**
** Definition of QThread class
**
** Created : 931107
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTHREAD_H
#define QTHREAD_H

#include <qwindowdefs.h>

#if defined(QT_THREAD_SUPPORT)

class QMutexPrivate;

const int Q_MUTEX_NORMAL = 0;
const int Q_MUTEX_RECURSIVE = 1;


class Q_EXPORT QMutex : public Qt
{
    friend class QCondition;

public:
    QMutex(bool recursive = FALSE);
    virtual ~QMutex();

    void lock();
    void unlock();
    bool locked();

private:
    QMutexPrivate * d;

#if defined(Q_DISABLE_COPY)
    QMutex( const QMutex & );
    QMutex &operator=( const QMutex & );
#endif
};

class QThreadPrivate;

class Q_EXPORT QThread : public Qt
{
    friend class QThreadPrivate;
public:
    static HANDLE currentThread();
    static void postEvent( QObject *,QEvent * );

    static void exit();

    QThread();
    virtual ~QThread();

    // default argument causes thread to block indefinately
    bool wait( unsigned long time = ULONG_MAX );

    void start();

    bool finished() const;
    bool running() const;


protected:
    virtual void run() = 0;

    static void sleep( unsigned long );
    static void msleep( unsigned long );
    static void usleep( unsigned long );

private:
    QThreadPrivate * d;

#if defined(Q_DISABLE_COPY)
    QThread( const QThread & );
    QThread &operator=( const QThread & );
#endif
};

class QConditionPrivate;

class Q_EXPORT QCondition : public Qt
{
public:
    QCondition();
    virtual ~QCondition();

    // default argument causes thread to block indefinately
    bool wait( unsigned long time = ULONG_MAX );
    bool wait( QMutex *mutex, unsigned long time = ULONG_MAX );

    void wakeOne();
    void wakeAll();

private:
    QConditionPrivate * d;

#if defined(Q_DISABLE_COPY)
    QCondition( const QCondition & );
    QCondition &operator=( const QCondition & );
#endif
};

class QSemaphorePrivate;

class Q_EXPORT QSemaphore : public Qt
{
public:
    QSemaphore( int );
    virtual ~QSemaphore();

    int available() const;
    int total() const;

    // postfix operators
    int operator++(int);
    int operator--(int);

    int operator+=(int);
    int operator-=(int);


private:
    QSemaphorePrivate *d;

#if defined(Q_DISABLE_COPY)
    QSemaphore(const QSemaphore &);
    QSemaphore &operator=(const QSemaphore &);
#endif
};

#endif

#endif
