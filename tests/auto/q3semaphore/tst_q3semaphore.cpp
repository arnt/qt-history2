/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>



#include <qthread.h>
#include <q3semaphore.h>






#include "q3semaphore.h"

//TESTED_CLASS=Q3Semaphore
//TESTED_FILES=compat/tools/q3semaphore.h compat/tools/q3semaphore.cpp

class tst_Q3Semaphore : public QObject
{
    Q_OBJECT

public:
    tst_Q3Semaphore();
    virtual ~tst_Q3Semaphore();

public slots:
    void init();
    void cleanup();
private slots:
    void incrementOne();
    void incrementN();
};

Q3Semaphore *semaphore = 0;

tst_Q3Semaphore::tst_Q3Semaphore()
{

}

tst_Q3Semaphore::~tst_Q3Semaphore()
{

}

// init() will be executed immediately before each testfunction is run.
void tst_Q3Semaphore::init()
{
}

// cleanup() will be executed immediately after each testfunction is run.
void tst_Q3Semaphore::cleanup()
{
}

class ThreadOne : public QThread
{
public:
    ThreadOne() {}

protected:
    void run()
    {
	int i = 0;
	while ( i < 100 ) {
	    (*semaphore)++;
	    i++;
	    (*semaphore)--;
	}
    }
};

void tst_Q3Semaphore::incrementOne()
{
    QVERIFY(!semaphore);
    semaphore = new Q3Semaphore(1);

    ThreadOne t1;
    ThreadOne t2;

    t1.start();
    t2.start();

    QVERIFY(t1.wait(4000));
    QVERIFY(t2.wait(4000));

    delete semaphore;
    semaphore = 0;
}

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
	    (*semaphore)+=N;
	    i++;
	    (*semaphore)-=N;
	}
    }
};

void tst_Q3Semaphore::incrementN()
{
    QVERIFY(!semaphore);
    semaphore = new Q3Semaphore(4);

    ThreadN t1(2);
    ThreadN t2(3);

    t1.start();
    t2.start();

    QVERIFY(t1.wait(4000));
    QVERIFY(t2.wait(4000));

    delete semaphore;
    semaphore = 0;
}

QTEST_MAIN(tst_Q3Semaphore)
#include "tst_q3semaphore.moc"
