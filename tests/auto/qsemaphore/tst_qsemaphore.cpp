/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qcoreapplication.h>
#include <qthread.h>
#include <qsemaphore.h>

//TESTED_CLASS=QSemaphore
//TESTED_FILES=corelib/thread/qsemaphore.h corelib/thread/qsemaphore.cpp

class tst_QSemaphore : public QObject
{
    Q_OBJECT

public:
    tst_QSemaphore();
    ~tst_QSemaphore();

private slots:
    void acquire();
    void tryAcquire();
    void tryAcquireWithTimeout_data();
    void tryAcquireWithTimeout();
    void release();
    void available();
    void producerConsumer();
};

static QSemaphore *semaphore = 0;

tst_QSemaphore::tst_QSemaphore()
{ }

tst_QSemaphore::~tst_QSemaphore()
{ }

class ThreadOne : public QThread
{
public:
    ThreadOne() {}

protected:
    void run()
    {
	int i = 0;
	while ( i < 100 ) {
            semaphore->acquire();
	    i++;
            semaphore->release();
	}
    }
};

class ThreadN : public QThread
{
    int N;

public:
    ThreadN(int n) :N(n) { }

protected:
    void run()
    {
	int i = 0;
	while ( i < 100 ) {
            semaphore->acquire(N);
	    i++;
            semaphore->release(N);
	}
    }
};

void tst_QSemaphore::acquire()
{
    {
        // old incrementOne() test
        QVERIFY(!semaphore);
        semaphore = new QSemaphore;
        // make some "thing" available
        semaphore->release();

        ThreadOne t1;
        ThreadOne t2;

        t1.start();
        t2.start();

        QVERIFY(t1.wait(4000));
        QVERIFY(t2.wait(4000));

        delete semaphore;
        semaphore = 0;
    }

    // old incrementN() test
    {
        QVERIFY(!semaphore);
        semaphore = new QSemaphore;
        // make 4 "things" available
        semaphore->release(4);

        ThreadN t1(2);
        ThreadN t2(3);

        t1.start();
        t2.start();

        QVERIFY(t1.wait(4000));
        QVERIFY(t2.wait(4000));

        delete semaphore;
        semaphore = 0;
    }

    QSemaphore semaphore;

    QCOMPARE(semaphore.available(), 0);
    semaphore.release();
    QCOMPARE(semaphore.available(), 1);
    semaphore.release();
    QCOMPARE(semaphore.available(), 2);
    semaphore.release(10);
    QCOMPARE(semaphore.available(), 12);
    semaphore.release(10);
    QCOMPARE(semaphore.available(), 22);

    semaphore.acquire();
    QCOMPARE(semaphore.available(), 21);
    semaphore.acquire();
    QCOMPARE(semaphore.available(), 20);
    semaphore.acquire(10);
    QCOMPARE(semaphore.available(), 10);
    semaphore.acquire(10);
    QCOMPARE(semaphore.available(), 0);
}

void tst_QSemaphore::tryAcquire()
{
    QSemaphore semaphore;

    QCOMPARE(semaphore.available(), 0);

    semaphore.release();
    QCOMPARE(semaphore.available(), 1);
    QVERIFY(!semaphore.tryAcquire(2));
    QCOMPARE(semaphore.available(), 1);

    semaphore.release();
    QCOMPARE(semaphore.available(), 2);
    QVERIFY(!semaphore.tryAcquire(3));
    QCOMPARE(semaphore.available(), 2);

    semaphore.release(10);
    QCOMPARE(semaphore.available(), 12);
    QVERIFY(!semaphore.tryAcquire(100));
    QCOMPARE(semaphore.available(), 12);

    semaphore.release(10);
    QCOMPARE(semaphore.available(), 22);
    QVERIFY(!semaphore.tryAcquire(100));
    QCOMPARE(semaphore.available(), 22);

    QVERIFY(semaphore.tryAcquire());
    QCOMPARE(semaphore.available(), 21);

    QVERIFY(semaphore.tryAcquire());
    QCOMPARE(semaphore.available(), 20);

    QVERIFY(semaphore.tryAcquire(10));
    QCOMPARE(semaphore.available(), 10);

    QVERIFY(semaphore.tryAcquire(10));
    QCOMPARE(semaphore.available(), 0);

    // should not be able to acquire more
    QVERIFY(!semaphore.tryAcquire());
    QCOMPARE(semaphore.available(), 0);

    QVERIFY(!semaphore.tryAcquire());
    QCOMPARE(semaphore.available(), 0);

    QVERIFY(!semaphore.tryAcquire(10));
    QCOMPARE(semaphore.available(), 0);

    QVERIFY(!semaphore.tryAcquire(10));
    QCOMPARE(semaphore.available(), 0);
}

void tst_QSemaphore::tryAcquireWithTimeout_data()
{
    QTest::addColumn<int>("timeout");

    QTest::newRow("") << 1000;
    QTest::newRow("") << 10000;
}

void tst_QSemaphore::tryAcquireWithTimeout()
{
    QFETCH(int, timeout);

    QSemaphore semaphore;
    QTime time;


    QCOMPARE(semaphore.available(), 0);

    semaphore.release();
    QCOMPARE(semaphore.available(), 1);
    time.start();
    QVERIFY(!semaphore.tryAcquire(2, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 1);

    semaphore.release();
    QCOMPARE(semaphore.available(), 2);
    time.start();
    QVERIFY(!semaphore.tryAcquire(3, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 2);

    semaphore.release(10);
    QCOMPARE(semaphore.available(), 12);
    time.start();
    QVERIFY(!semaphore.tryAcquire(100, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 12);

    semaphore.release(10);
    QCOMPARE(semaphore.available(), 22);
    time.start();
    QVERIFY(!semaphore.tryAcquire(100, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 22);

    time.start();
    QVERIFY(semaphore.tryAcquire(1, timeout));
    QVERIFY(time.elapsed() <= timeout);
    QCOMPARE(semaphore.available(), 21);

    time.start();
    QVERIFY(semaphore.tryAcquire(1, timeout));
    QVERIFY(time.elapsed() <= timeout);
    QCOMPARE(semaphore.available(), 20);

    time.start();
    QVERIFY(semaphore.tryAcquire(10, timeout));
    QVERIFY(time.elapsed() <= timeout);
    QCOMPARE(semaphore.available(), 10);

    time.start();
    QVERIFY(semaphore.tryAcquire(10, timeout));
    QVERIFY(time.elapsed() <= timeout);
    QCOMPARE(semaphore.available(), 0);

    // should not be able to acquire more
    time.start();
    QVERIFY(!semaphore.tryAcquire(1, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 0);

    time.start();
    QVERIFY(!semaphore.tryAcquire(1, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 0);

    time.start();
    QVERIFY(!semaphore.tryAcquire(10, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 0);

    time.start();
    QVERIFY(!semaphore.tryAcquire(10, timeout));
    QVERIFY(time.elapsed() >= timeout);
    QCOMPARE(semaphore.available(), 0);
}

void tst_QSemaphore::release()
{ DEPENDS_ON("acquire"); }

void tst_QSemaphore::available()
{ DEPENDS_ON("acquire"); }

const char alphabet[] = "ACGTH";
const int AlphabetSize = sizeof(alphabet) - 1;

const int BufferSize = 4096; // GCD of BufferSize and alphabet size must be 1
char buffer[BufferSize];

const int ProducerChunkSize = 3;
const int ConsumerChunkSize = 7;

// note: the code depends on the fact that DataSize is a multiple of
// ProducerChunkSize, ConsumerChunkSize, and BufferSize
const int DataSize = ProducerChunkSize * ConsumerChunkSize * BufferSize * 10;

QSemaphore freeSpace(BufferSize);
QSemaphore usedSpace;

class Producer : public QThread
{
public:
    void run();
};

void Producer::run()
{
    for (int i = 0; i < DataSize; ++i) {
        freeSpace.acquire();
        buffer[i % BufferSize] = alphabet[i % AlphabetSize];
        usedSpace.release();
    }
    for (int i = 0; i < DataSize; ++i) {
        if ((i % ProducerChunkSize) == 0)
            freeSpace.acquire(ProducerChunkSize);
        buffer[i % BufferSize] = alphabet[i % AlphabetSize];
        if ((i % ProducerChunkSize) == (ProducerChunkSize - 1))
            usedSpace.release(ProducerChunkSize);
    }
}

class Consumer : public QThread
{
public:
    void run();
};

void Consumer::run()
{
    for (int i = 0; i < DataSize; ++i) {
        usedSpace.acquire();
        QCOMPARE(buffer[i % BufferSize], alphabet[i % AlphabetSize]);
        freeSpace.release();
    }
    for (int i = 0; i < DataSize; ++i) {
        if ((i % ConsumerChunkSize) == 0)
            usedSpace.acquire(ConsumerChunkSize);
        QCOMPARE(buffer[i % BufferSize], alphabet[i % AlphabetSize]);
        if ((i % ConsumerChunkSize) == (ConsumerChunkSize - 1))
            freeSpace.release(ConsumerChunkSize);
    }
}

void tst_QSemaphore::producerConsumer()
{
    Producer producer;
    Consumer consumer;
    producer.start();
    consumer.start();
    producer.wait();
    consumer.wait();
}

QTEST_MAIN(tst_QSemaphore)
#include "tst_qsemaphore.moc"
