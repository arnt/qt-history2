#include "qthread.h"
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

QMutex::QMutex()
{
  int ret=pthread_mutex_init(&mymutex,0);
  if(ret) {
    printf("Mutex init failure %s\n",strerror(ret));
  }
}

QMutex::~QMutex()
{
  int ret=pthread_mutex_destroy(&mymutex);
  if(ret) {
    printf("Mutex destroy failure %s\n",strerror(ret));
  }
}

void QMutex::lock()
{
  int ret=pthread_mutex_lock(&mymutex);
  if(ret) {
    printf("Mutex lock failure\n",strerror(ret));
  }
}

void QMutex::unlock()
{
  int ret=pthread_mutex_unlock(&mymutex);
  if(ret) {
    printf("Mutex unlock failure\n",strerror(ret));
  }   
}

int QThread::currentThread()
{
  // A pthread_t is an int
  return pthread_self();
}
