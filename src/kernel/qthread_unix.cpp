#include "qthread.h"
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <qlist.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qobject.h>

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

class QThreadPrivate : public QObject {

    Q_OBJECT

public:

    bool woken;
    
    QThreadPrivate();
    void wakeGuiThread();
    
    QList<QThreadEvent> myevents;
    QMutex myeventmutex;

public slots:

    void socketActivated(int);

private:

    int wakeupPipe[2];

};

#include "qthread_unix.moc"

QThreadPrivate::QThreadPrivate()
{
    if ( pipe( wakeupPipe) ) {
	qFatal("Couldn't open thread pipe: %s\n",strerror(errno));
    }
    myevents.setAutoDelete(TRUE);
    QSocketNotifier * sn=new QSocketNotifier (wakeupPipe[1],QSocketNotifier::Read,this);
    connect(sn,SIGNAL(activated(int)),this,SLOT(socketActivated(int)));
    woken=false;
}

void QThreadPrivate::socketActivated(int)
{
  char c;
  read(wakeupPipe[0],&c,1);
  QThread::sendPostedEvents();
}

static QThreadPrivate * qthreadprivate=0;

void QThreadPrivate::wakeGuiThread()
{
    char c=1;
    write( wakeupPipe[1],&c,1 );
}

void QThread::postEvent(QObject * o,QEvent * e)
{
  if(!qthreadprivate) {
    qthreadprivate=new QThreadPrivate();
  }
  qthreadprivate->myeventmutex.lock();
  QThreadEvent * qte=new QThreadEvent;
  qte->o=o;
  qte->e=e;
  qthreadprivate->myevents.append(qte);
  qthreadprivate->myeventmutex.unlock();
  if(!qthreadprivate->woken) {
    qthreadprivate->wakeGuiThread();
    qthreadprivate->woken=true;
  }
}

void QThread::sendPostedEvents()
{
  if(!qthreadprivate) {
    qthreadprivate=new QThreadPrivate();
  }
  qthreadprivate->myeventmutex.lock();
  qthreadprivate->woken=false;
  QThreadEvent * qte;
  for(qte=qthreadprivate->myevents.first();qte!=0;qte=qthreadprivate->myevents.next()) {
    qApp->postEvent(qte->o,qte->e);
  }
  qthreadprivate->myevents.clear();
  qApp->sendPostedEvents();
  qthreadprivate->myeventmutex.unlock();
}
