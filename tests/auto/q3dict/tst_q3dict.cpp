/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <q3dict.h>

//TESTED_CLASS=
//TESTED_FILES=compat/tools/q3dict.h

class tst_Q3Dict : public QObject
{
Q_OBJECT

public:
    tst_Q3Dict();
    virtual ~tst_Q3Dict();


public slots:
    void init();
    void cleanup();
private slots:
    void resize();
    void acc_01_data();
    void acc_01();
};

tst_Q3Dict::tst_Q3Dict()
{
}

tst_Q3Dict::~tst_Q3Dict()
{
}

void tst_Q3Dict::init()
{
// TODO: Add initialization code here.
// This will be executed immediately before each test is run.
}

void tst_Q3Dict::cleanup()
{
// TODO: Add cleanup code here.
// This will be executed immediately after each test is run.
}

#include <qstring.h>
#include <qdatetime.h>
#include <stdlib.h>

QString keyFor( int i )
{
    QString key;
    key.sprintf("KEY%05d",i);
    return key;
}
#include <qapplication.h>

void tst_Q3Dict::acc_01_data()
{
    QTest::addColumn<int>("nins");

    //next we fill it with data
    QTest::newRow( "data0" )  << 5;
}

void tst_Q3Dict::acc_01()
{
    QFETCH(int,nins);

    Q3Dict<int> dict(7);
    dict.setAutoDelete( TRUE );

    for ( int i=0; i<nins; i++ ) {
	int* d = new int;
	*d = i;
	dict.insert(keyFor(i),d);
    }

    QTime timer;
    int start = nins/500;
    if (start == 0)
	start = 1;
    for ( int j=start; j<1000000; j+=1+j/10 ) {// don't want to use 0 here because that crashes

	timer.start();
	dict.resize( j );
//	int ms_r = timer.elapsed();

	int n=0;
	timer.start();
	for ( Q3DictIterator<int> it(dict); it.current(); ++it ) {
	    n++;
	    QVERIFY( keyFor( *it.current() ) == it.currentKey() ); // Wrong key if it isn't
	}
	QVERIFY( !(n != nins) ); //qFatal("Too few");
//	int ms_i = timer.elapsed();

	timer.start();
	for ( int i = 0; i<nins; i++ ) {
	    dict.find( keyFor(i) );
	}
//	int ms_f = timer.elapsed();
//	qDebug("resize(%d) took %dms, iteration took %dms, find took %0.1f\265s", j, ms_r, ms_i, 1000.0*ms_f/nins);
    }

    dict.resize( 10 );
}

void tst_Q3Dict::resize()
{
    Q3Dict<int> dict(7);
    QVERIFY( dict.size() == 7 );

    int i;
    for ( i=0; i<7; i++ ) {
	int* d = new int(i);
	dict.insert(keyFor(i),d);
    }

    QVERIFY(dict.size() == 7);

    for (i = 0; i < 7; ++i)
        delete dict.take(keyFor(i));
}

QTEST_APPLESS_MAIN(tst_Q3Dict)
#include "tst_q3dict.moc"
