/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qapplication.h>
#include <qthread.h>

#include <qmutex.h>



#include <q3deepcopy.h>





//TESTED_CLASS=
//TESTED_FILES=compat/tools/q3deepcopy.h 

class tst_Q3DeepCopy : public QObject
{
Q_OBJECT

public:
    tst_Q3DeepCopy();
    virtual ~tst_Q3DeepCopy();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void deepcopy_data();
    void deepcopy();
};

// this is the thread object that stress tests Q3DeepCopy
class Thread : public QThread
{
public:
    QMutex *mutex;
    Thread *other;
    Q3DeepCopy<QString> string;
    int iterations;

    Thread() : mutex( 0 ), other( 0 ) { }

    void run()
    {
        QString local;
        QStringList list;

        mutex->lock();
        local = string;
        mutex->unlock();

	while ( iterations-- ) {
            mutex->lock();
            local = string;
	    mutex->unlock();

            list.clear();
            list << local << local << local << local << local;
            list << local << local << local << local << local;
            list << local << local << local << local << local;
            list << local << local << local << local << local;

            local = QString::null;
            local = QString::null;
            local = QString::null;
            local = QString::null;

            mutex->lock();
            local = other->string;
            mutex->unlock();

            local = local + local;
            local = local + local;
            local = local + local;
            local = local + local;

            local = QString::null;
            local = QString::null;
            local = QString::null;
            local = QString::null;
        }
    }
};


tst_Q3DeepCopy::tst_Q3DeepCopy()
{
}

tst_Q3DeepCopy::~tst_Q3DeepCopy()
{

}

// initTestCase will be executed once before the first testfunction is executed.
void tst_Q3DeepCopy::initTestCase()
{
}

// cleanupTestCase will be executed once after the last testfunction is executed.
void tst_Q3DeepCopy::cleanupTestCase()
{
}

// init() will be executed immediately before each testfunction is run.
void tst_Q3DeepCopy::init()
{
}

// cleanup() will be executed immediately after each testfunction is run.
void tst_Q3DeepCopy::cleanup()
{
}

void tst_Q3DeepCopy::deepcopy_data()
{
    // define the test elements we're going to use
    QTest::addColumn<QString>("string0");
    QTest::addColumn<int>("iterations0");
    QTest::addColumn<QString>("string1");
    QTest::addColumn<int>("iterations1");
    QTest::addColumn<QString>("string2");
    QTest::addColumn<int>("iterations2");
    QTest::addColumn<QString>("string3");
    QTest::addColumn<int>("iterations3");

    // create a first testdata instance and fill it with data
    QTest::newRow( "data0" )
	<< QString( "one.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" ) << 1234
	<< QString( "two.bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" ) << 2345
	<< QString( "three.ccccccccccccccccccccccccccccccccccccccccc" ) << 3456
	<< QString( "four.dddddddddddddddddddddddddddddddddddddddddd" ) << 4567;
    QTest::newRow( "data1" )
	<< QString( "one.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" ) << 12345
	<< QString( "two.bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" ) << 23456
	<< QString( "three.ccccccccccccccccccccccccccccccccccccccccc" ) << 34567
	<< QString( "four.dddddddddddddddddddddddddddddddddddddddddd" ) << 45678;
    QTest::newRow( "data2" )
	<< QString( "one.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" ) << 45678
	<< QString( "two.bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" ) << 34567
	<< QString( "three.ccccccccccccccccccccccccccccccccccccccccc" ) << 23456
	<< QString( "four.dddddddddddddddddddddddddddddddddddddddddd" ) << 12345;
    QTest::newRow( "data3" )
	<< QString( "one.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" ) << 4567
	<< QString( "two.bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" ) << 3456
	<< QString( "three.ccccccccccccccccccccccccccccccccccccccccc" ) << 2345
	<< QString( "four.dddddddddddddddddddddddddddddddddddddddddd" ) << 1234;
}

void tst_Q3DeepCopy::deepcopy()
{
    QFETCH( QString, string0 );
    QFETCH( int, iterations0 );
    QFETCH( QString, string1 );
    QFETCH( int, iterations1);
    QFETCH( QString, string2 );
    QFETCH( int, iterations2);
    QFETCH( QString, string3 );
    QFETCH( int, iterations3);

    QMutex mutex1, mutex2;
    Thread one, two, three, four;

    one.string = string0;
    one.mutex = &mutex1;
    one.other = &two;
    one.iterations = iterations0;

    two.string = string1;
    two.mutex = &mutex1;
    two.other = &one;
    two.iterations = iterations1;

    three.string = string2;
    three.mutex = &mutex2;
    three.other = &four;
    three.iterations = iterations2;

    four.string = string3;
    four.mutex = &mutex2;
    four.other = &three;
    four.iterations = iterations3;

    one.start();
    two.start();
    three.start();
    four.start();

    // wait for the threads for a maximum of 5 minutes.  anything long, we assume
    // that the test has deadlocked.
    QVERIFY( one.wait( 300000 ) );
    QVERIFY( two.wait( 300000 ) );
    QVERIFY( three.wait( 300000 ) );
    QVERIFY( four.wait( 300000 ) );
}

QTEST_MAIN(tst_Q3DeepCopy)
#include "tst_qdeepcopy.moc"
