/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread_win.cpp#1 $
**
** QThread class for windows
**
** Created : 931107
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/


#include "qthread.h"
#include <process.h>

class QMutexPrivate {
};

// Stub implementation

QMutex::QMutex()
{
}

QMutex::~QMutex()
{
}

void QMutex::lock()
{
}

void QMutex::unlock()
{
}

int QThread::currentThread()
{
  return 0;
}

void QThread::postEvent(QObject *,QEvent *)
{
}

static void start_thread(QThread * t)
{
  t->run();
}

QThread::QThread()
{
}

QThread::~QThread()
{
}

void QThread::start()
{
  // Error checking would be good
  _beginthread(start_thread,0,(void *)this);
}

void QThread::run()
{
    // Default implementation does nothing
}

