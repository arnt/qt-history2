#include <qthread.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlayout.h>
#include <qevent.h>
#include <qlabel.h>
#include <qarray.h>
#include <qtextstream.h>
#include <qfile.h>

#include <stdio.h>

// 20kb buffer
#define BUFSIZE (20*1024)
#define PRGSTEP (BUFSIZE / 100)
#define BLKSIZE (2)
QByteArray bytearray;


class ProdEvent : public QCustomEvent
{
public:
    ProdEvent(long s, bool d)
	: QCustomEvent(QEvent::User + 100), sz(s), dn(d)
    { ; }

    long size() const { return sz; }
    bool done() const { return dn; }


private:
    long sz;
    bool dn;
};


class ProdThread : public QThread
{
public:
    ProdThread(QObject *r, QMutex *m, QWaitCondition *c);

    void stop();
    void run();


private:
    QObject *receiver;
    QMutex *mutex;
    QWaitCondition *condition;

    bool done;
};


ProdThread::ProdThread(QObject *r, QMutex *m, QWaitCondition *c)
    : receiver(r), mutex(m), condition(c), done(FALSE)
{
}


void ProdThread::stop()
{
    mutex->lock();
    done = TRUE;
    mutex->unlock();
}


void ProdThread::run()
{
    bool stop = FALSE;
    done = FALSE;

    uchar *buffer = new uchar[BUFSIZE];
    int pos = 0, oldpos = 0;
    int loop = 1;

    ProdEvent *pe = new ProdEvent(pos, done);
    QThread::postEvent(receiver, pe);

    while (! stop) {
	oldpos = pos;
	int i;
	for (i = 0; i < BLKSIZE && pos < BUFSIZE; i++) {
	    buffer[pos++] = (loop % 2) ? 'o' : 'e';
	}

	mutex->lock();

	if (pos == BUFSIZE) {
	    done = TRUE;
	}

	while (! bytearray.isNull() && ! done) {
            condition->wakeOne();
            condition->wait(mutex);
	}

	stop = done;
    bytearray.duplicate((const char *) (buffer + oldpos), pos - oldpos);
    condition->wakeOne();

	mutex->unlock();

	ProdEvent *pe = new ProdEvent(pos, stop);
	QThread::postEvent(receiver, pe);

	loop++;
    }

    condition->wakeOne();

    delete [] buffer;
}


class ConsEvent : public QCustomEvent
{
public:
    ConsEvent(long s)
	: QCustomEvent(QEvent::User + 101), sz(s)
    { ; }

    long size() const { return sz; }


private:
    long sz;
};


class ConsThread : public QThread
{
public:
    ConsThread(QObject *r, QMutex *m, QWaitCondition *c);

    void stop();
    void run();


private:
    QObject *receiver;
    QMutex *mutex;
    QWaitCondition *condition;

    bool done;
};


ConsThread::ConsThread(QObject *r, QMutex *m, QWaitCondition *c)
    : receiver(r), mutex(m), condition(c), done(FALSE)
{
}


void ConsThread::stop()
{
    mutex->lock();
    done = TRUE;
    mutex->unlock();
}


void ConsThread::run()
{
    done = FALSE;

    QFile file("prodcons.out");
    file.open(IO_WriteOnly);

    long size = 0;

    ConsEvent *ce = new ConsEvent(size);
    QThread::postEvent(receiver, ce);

    while (! done) {
	mutex->lock();

	while (bytearray.isNull() && ! done) {
            condition->wakeOne();
            condition->wait(mutex);
	}

    if (! done) {
	    file.writeBlock(bytearray.data(), bytearray.size());
	    size += bytearray.size();
	    bytearray.resize(0);
    }

	mutex->unlock();

	ConsEvent *ce = new ConsEvent(size);
	QThread::postEvent(receiver, ce);
    }

    file.flush();
    file.close();
}


class ProdCons : public QWidget
{
    Q_OBJECT

public:
    ProdCons();
    ~ProdCons();

    void customEvent(QCustomEvent *);


public slots:
    void go();
    void stop();


private:
    QMutex mutex;
    QWaitCondition condition;

    ProdThread *prod;
    ConsThread *cons;

    QPushButton *startbutton, *stopbutton;
    QProgressBar *prodbar, *consbar;
};


ProdCons::ProdCons()
    : QWidget(0, "producer consumer widget"), prod(0), cons(0)
{
    startbutton = new QPushButton("&Start", this);
    connect(startbutton, SIGNAL(clicked()), SLOT(go()));

    stopbutton = new QPushButton("S&top", this);
    connect(stopbutton, SIGNAL(clicked()), SLOT(stop()));
    stopbutton->setEnabled(FALSE);

    prodbar = new QProgressBar(BUFSIZE, this);
    consbar = new QProgressBar(BUFSIZE, this);

    QVBoxLayout *vbox = new QVBoxLayout(this, 8, 8);
    vbox->addWidget(new QLabel(QString("Producer/Consumer using %1 byte buffer").
			       arg(BUFSIZE), this));
    vbox->addWidget(startbutton);
    vbox->addWidget(stopbutton);
    vbox->addWidget(new QLabel("Producer progress:", this));
    vbox->addWidget(prodbar);
    vbox->addWidget(new QLabel("Consumer progress:", this));
    vbox->addWidget(consbar);
}


ProdCons::~ProdCons()
{
    stop();

    if (prod) {
	delete prod;
	prod = 0;
    }

    if (cons) {
	delete cons;
	cons = 0;
    }
}


void ProdCons::go()
{
    mutex.lock();
    startbutton->setEnabled(FALSE);
    stopbutton->setEnabled(TRUE);

    // start the consumer first
    if (! cons)
        cons = new ConsThread(this, &mutex, &condition);
    cons->start();

    // wait for consumer to signal that it has started
    condition.wait(&mutex);

    if (! prod)
        prod = new ProdThread(this, &mutex, &condition);
    prod->start();
    mutex.unlock();
}


void ProdCons::stop()
{
    if (prod && prod->running()) {
	prod->stop();
        condition.wakeAll();
	prod->wait();
    }

    if (cons && cons->running()) {
    cons->stop();
        condition.wakeAll();
	cons->wait();
    }

    startbutton->setEnabled(TRUE);
    stopbutton->setEnabled(FALSE);
}


void ProdCons::customEvent(QCustomEvent *e)
{
    switch (e->type()) {
    case QEvent::User + 100:
	{
	    // ProdEvent
	    ProdEvent *pe = (ProdEvent *) e;

	    if (pe->size() == 0 ||
		pe->size() == BUFSIZE ||
		pe->size() - prodbar->progress() >= PRGSTEP)
		prodbar->setProgress(pe->size());

	    // reap the threads
        if (pe->done())
        stop();

	    break;
	}

    case QEvent::User + 101:
	{
	    // ConsEvent
	    ConsEvent *ce = (ConsEvent *) e;

	    if (ce->size() == 0 ||
		ce->size() == BUFSIZE ||
		ce->size() - consbar->progress() >= PRGSTEP)
		consbar->setProgress(ce->size());

	    /*
	      if (ce->size() == BUFSIZE)
	      stop();
	    */
	    break;
	}

    default:
	{
	    ;
	}
    }
}


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    ProdCons prodcons;
    app.setMainWidget(&prodcons);
    prodcons.show();
    return app.exec();
}


#include "prodcons.moc"
