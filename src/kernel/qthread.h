/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread.h#16 $
**
** Definition of QThread class
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


#ifndef QTHREAD_H
#define QTHREAD_H

#include <qglobal.h>
#include <qobject.h>
#include <qevent.h>

class QMutexPrivate;

class QMutex {

  QMutexPrivate * d;

public:

  QMutex();
  ~QMutex();
  void lock();
  void unlock();

};

class QThread {

public:

  static int currentThread();
  static void postEvent(QObject *,QEvent *);

};

#endif





