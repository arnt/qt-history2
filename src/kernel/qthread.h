#ifndef QTHREAD_H
#define QTHREAD_H

#include <qglobal.h>
#include <qobject.h>
#include <qevent.h>

#ifdef UNIX
#include <pthread.h>
#endif

class QMutex {

#ifdef UNIX
  pthread_mutex_t mymutex;
#else
#warning Eek,no mutex!
#endif

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
  static void sendPostedEvents();

};

#endif





