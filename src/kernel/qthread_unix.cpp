#include "qthread.h"
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <qlist.h>
#include <qapplication.h>

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
    printf("Mutex lock failure %s\n",strerror(ret));
  }
}

void QMutex::unlock()
{
  int ret=pthread_mutex_unlock(&mymutex);
  if(ret) {
    printf("Mutex unlock failure %s\n",strerror(ret));
  }   
}

int QThread::currentThread()
{
  // A pthread_t is an int
  return pthread_self();
}

class QThreadEvent {

public:

  QObject * o;
  QEvent * e;

};

QList<QThreadEvent> * myevents=0;
QMutex * myeventmutex=0;

void QThread::postEvent(QObject * o,QEvent * e)
{

  if(!myeventmutex) {
    myeventmutex=new QMutex();
  }
  myeventmutex->lock();
  if(!myevents) {
    myevents=new QList<QThreadEvent>;
    myevents->setAutoDelete(TRUE);
  }
  QThreadEvent * qte=new QThreadEvent;
  qte->o=o;
  qte->e=e;
  myevents->append(qte);
  myeventmutex->unlock();
}

void QThread::sendPostedEvents()
{
  if(!myeventmutex) {
    myeventmutex=new QMutex();
  }
  myeventmutex->lock();
  if(!myevents)
    return;
  QThreadEvent * qte;
  for(qte=myevents->first();qte!=0;qte=myevents->next()) {
    qApp->postEvent(qte->o,qte->e);
  }
  myevents->clear();
  myeventmutex->unlock();
}
