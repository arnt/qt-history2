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

#include "qthread_unix.moc"

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

QThreadPrivate::QThreadPrivate()
{
    if ( pipe( wakeupPipe) ) {
	qFatal("Couldn't open thread pipe: %s\n",strerror(errno));
    }
    QSocketNotifier * sn=new QSocketNotifier (wakeupPipe[1],QSocketNotifer::Read,this);
    connect(sn,SIGNAL(activated(int)),this,SLOT(socketActivated(int)));
    woken=false;
}

void QThreadPrivate::socketActivated(int)
{
  char c;
  read(wakeupPipe[0],&c,1);
  sendPostedEvents();
}

static QThreadPrivate * qthreadprivate=0;

void QThreadPrivate::wakeGuiThread()
{
    char c=1;
    write( wakeupPipe[1],&c,1 );
}

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
  if(!woken) {
    qt_wake_gui_thread();
    woken=true;
  }
}

void QThread::sendPostedEvents()
{
  if(!myeventmutex) {
    myeventmutex=new QMutex();
  }
  if(!myevents)
    return;
  myeventmutex->lock();
  woken=false;
  QThreadEvent * qte;
  for(qte=myevents->first();qte!=0;qte=myevents->next()) {
    qApp->postEvent(qte->o,qte->e);
  }
  myevents->clear();
  qApp->sendPostedEvents();
  myeventmutex->unlock();
}
