/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qeventloop.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmutex.h>
#include <qpointer.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qwidget.h>

#include <stdio.h>
#include <stdlib.h>


static const int totalSize = 1000000;
static const int increment = totalSize / 100;


static const char dataSource[4] = { 'A', 'C', 'G', 'T' };
static int stats[4] = { 0, 0, 0, 0 };

static QString statsString()
{
    QString s("Statistics:");
    for (int x = 0; x < sizeof(dataSource); ++x)
	s += QString(" %1=%2")
	     .arg(dataSource[x])
	     .arg(stats[x] ? QString::number(stats[x]) : QString("Unavailable"));
    return s;
}

static inline void resetStats()
{
    for (int x = 0; x < sizeof(dataSource); ++x)
	stats[x] = 0;
}


class Producer : public QObject
{
    Q_OBJECT

    int total;
    int maximum;

public:
    inline Producer(int max)
	: total(0), maximum(max)
    { }

    inline bool isFinished() const
    { return total == maximum; }

    inline int maximumSize() const { return maximum; }

    inline void produce(QByteArray *data)
    {
	data->resize(increment);
	char *d = data->data();
	for (int i = 0; i < data->size(); ++i)
	    d[i] = dataSource[random() % sizeof(dataSource)];

	total += data->size();
	emit produced(total);

	if (isFinished()) {
	    emit finished();
	}
    }

signals:
    void produced(int size);
    void finished();
};


class Consumer : public QObject
{
    Q_OBJECT

    int total;

    Producer *_producer;

public:
    inline Consumer(Producer *producer)
	: total(0), _producer(producer)
    { }

    inline void consume(QByteArray *data)
    {
	const char * const d = data->constData();
	for (int i = 0; i < data->size(); ++i) {
	    for (int x = 0; x < sizeof(dataSource); ++x) {
		if (d[i] == dataSource[x])
		    ++stats[x];
	    }
	}

	total += data->size();
	data->resize(0);
	emit consumed(total);

	if (total == _producer->maximumSize()) {
	    emit finished();
	    emit finished(statsString());
	}
    }

signals:
    void consumed(int size);
    void finished();
    void finished(const QString &string);
};


class ProducerThread : public QThread
{
 public:
    Producer *producer;

    QByteArray *data;

    QMutex *mutex;
    QWaitCondition *condition;
    QWaitCondition startup;

    volatile bool stopped;

    inline ProducerThread()
	: producer(0), data(0), mutex(0), condition(0), stopped(false)
    { }

    inline void stop()
    { stopped = true; }

    void run() {
	QEventLoop eventloop;
	producer = new Producer(totalSize);

	{
	    // startup checkpoint
	    QMutexLocker locker(mutex);
	    stopped = false;
	    startup.wakeOne();
	    startup.wait(mutex);
	}

	do {
	    QMutexLocker locker(mutex);

	    while (!stopped && !data->isEmpty()) {
		condition->wakeOne();
		condition->wait(mutex);
	    }
	    if (stopped) break;

	    producer->produce(data);
	    if (producer->isFinished())
		break;
	} while (!stopped);

	if (!stopped) {
	    // wait for consumer to finish
	    QMutexLocker locker(mutex);
	    condition->wakeOne();
	    condition->wait(mutex);
	}

	delete producer;
    }
};

class ConsumerThread : public QThread
{
 public:
    QPointer<Producer> producer;
    Consumer *consumer;

    QByteArray *data;

    QMutex *mutex;
    QWaitCondition *condition;
    QWaitCondition startup;

    volatile bool stopped;

    inline ConsumerThread()
	: producer(0), consumer(0), data(0), mutex(0), condition(0), stopped(false)
    { }

    inline void stop()
    { stopped = true; }

    void run() {
	QEventLoop eventloop;
	consumer = new Consumer(producer);

	{
	    // startup checkpoint
	    QMutexLocker locker(mutex);
	    stopped = false;
	    startup.wakeOne();
	    startup.wait(mutex);
	}

	do {
	    QMutexLocker locker(mutex);

	    while (!stopped && data->isEmpty()) {
		condition->wakeOne();
		condition->wait(mutex);
	    }
	    if (stopped) break;

	    consumer->consume(data);
	    if (producer->isFinished())
		break;
	} while (!stopped);

	if (!stopped) {
	    // tell the producer we are finished
	    QMutexLocker locker(mutex);
	    condition->wakeOne();
	}

	delete consumer;
    }
};


class ProdCons : public QWidget
{
    Q_OBJECT

public:
    ProdCons();
    ~ProdCons();

public slots:
    void go();
    void stop();

    void finished();

private:
    ProducerThread prod;
    ConsumerThread cons;

    QPushButton *startButton, *stopButton;
    QCheckBox *loopCheckBox;
    QProgressBar *prodBar, *consBar;
    QLabel *statsLabel;

    bool stopped;
    bool redraw;
};

ProdCons::ProdCons()
    : QWidget(0, "producer consumer widget"), stopped(false), redraw(true)
{
    setWindowTitle("Qt Example - Producer/Consumer");

    startButton = new QPushButton("&Start", this);
    connect(startButton, SIGNAL(clicked()), SLOT(go()));

    stopButton = new QPushButton("S&top", this);
    connect(stopButton, SIGNAL(clicked()), SLOT(stop()));
    stopButton->setEnabled(false);

    loopCheckBox = new QCheckBox("Loop", this);
    loopCheckBox->setChecked(false);

    prodBar = new QProgressBar(totalSize, this);
    consBar = new QProgressBar(totalSize, this);

    statsLabel = new QLabel(statsString(), this);

    QVBoxLayout *vbox = new QVBoxLayout(this, 8, 8);
    vbox->addWidget(new QLabel(QString("Producer/Consumer using %1kb byte buffer").
			       arg(totalSize/1000), this));
    QFrame *line;

    vbox->addWidget(startButton);
    vbox->addWidget(stopButton);
    vbox->addWidget(loopCheckBox);

    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    vbox->addWidget(line);

    vbox->addWidget(new QLabel("Producer progress:", this));
    vbox->addWidget(prodBar);
    vbox->addWidget(new QLabel("Consumer progress:", this));
    vbox->addWidget(consBar);

    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    vbox->addWidget(line);

    vbox->addWidget(statsLabel);

    prod.data = cons.data = new QByteArray();
    prod.mutex = cons.mutex = new QMutex;
    prod.condition = cons.condition = new QWaitCondition;
}

ProdCons::~ProdCons()
{
    stop();

    delete cons.data;
    delete cons.mutex;
    delete cons.condition;
    prod.data = cons.data = 0;
    prod.mutex = cons.mutex = 0;
    prod.condition = cons.condition = 0;
}

void ProdCons::go()
{
    stopped = false;

    if ( redraw ) {
	stopButton->setEnabled(true);
	stopButton->setFocus();
	startButton->setEnabled(false);
    }

    resetStats();

    {
	QMutexLocker locker(prod.mutex);

	// start the producer
	prod.start();
	prod.startup.wait(locker.mutex());

	cons.producer = prod.producer;
	connect(prod.producer, SIGNAL(produced(int)), prodBar, SLOT(setProgress(int)));

	prod.startup.wakeOne();
    }

    {
	QMutexLocker locker(cons.mutex);
	// start the consumer and wait for it to signal that is has started

	cons.start();
	cons.startup.wait(locker.mutex());

	connect(cons.consumer, SIGNAL(consumed(int)), consBar, SLOT(setProgress(int)));
	connect(cons.consumer, SIGNAL(finished()), SLOT(finished()));
	connect(cons.consumer, SIGNAL(finished(const QString &)),
		statsLabel, SLOT(setText(const QString &)));

	cons.startup.wakeOne();
    }
}

void ProdCons::stop()
{
    {
	QMutexLocker locker(prod.mutex);
	prod.stop();
	cons.stop();
	prod.condition->wakeAll();
    }

    if (prod.running())
	prod.wait();

    if (cons.running())
	cons.wait();

    if ( redraw ) {
	// no point in repainting these Buttons so many times is we are looping...
	startButton->setEnabled(true);
	startButton->setFocus();
	stopButton->setEnabled(false);

	prodBar->reset();
	consBar->reset();
    }

    stopped = true;
}

void ProdCons::finished()
{
    bool loop = (loopCheckBox->isChecked() && !stopped);
    bool save_redraw = redraw;
    redraw = !loop;

    stop();
    if (loop)
	go();

    redraw = save_redraw;
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
