#ifndef QTHREAD_H
#define QTHREAD_H

#include <qglobal.h>

#ifdef _OS_UNIX_
#include <pthread.h>
#endif

class QMutex {

#ifdef _OS_UNIX_
pthread_mutex_t mymutex;
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

};

#endif
