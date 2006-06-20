/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qregexp.h>
#include <qstringlist.h>





//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qstringlist.h corelib/tools/qstringlist.cpp

class tst_QStringList : public QObject
{
    Q_OBJECT

public:
    tst_QStringList();
    virtual ~tst_QStringList();

public slots:
    void init();
    void cleanup();
private slots:
    void filter();
    void replaceInStrings();
    void contains();
    void indexOf();
    void lastIndexOf();

    void indexOf_regExp();
    void lastIndexOf_regExp();

    void streamingOperator();
};

extern const char email[];

tst_QStringList::tst_QStringList()
{
}

tst_QStringList::~tst_QStringList()
{
}

void tst_QStringList::init()
{
}

void tst_QStringList::cleanup()
{
}

void tst_QStringList::indexOf_regExp()
{
    QStringList list;
    list << "harald" << "trond" << "vohi" << "harald";

    QRegExp re(".*o.*");

    QCOMPARE(list.indexOf(re), 1);
    QCOMPARE(list.indexOf(re, 2), 2);
    QCOMPARE(list.indexOf(re, 3), -1);

    QCOMPARE(list.indexOf(QRegExp(".*x.*")), -1);
    QCOMPARE(list.indexOf(re, -1), -1);
    QCOMPARE(list.indexOf(re, -3), 1);
    QCOMPARE(list.indexOf(re, -9999), 1);
    QCOMPARE(list.indexOf(re, 9999), -1);
}

void tst_QStringList::lastIndexOf_regExp()
{
    QStringList list;
    list << "harald" << "trond" << "vohi" << "harald";

    QRegExp re(".*o.*");

    QCOMPARE(list.lastIndexOf(re), 2);
    QCOMPARE(list.lastIndexOf(re, 2), 2);
    QCOMPARE(list.lastIndexOf(re, 1), 1);

    QCOMPARE(list.lastIndexOf(QRegExp(".*x.*")), -1);
    QCOMPARE(list.lastIndexOf(re, -1), 2);
    QCOMPARE(list.lastIndexOf(re, -3), 1);
    QCOMPARE(list.lastIndexOf(re, -9999), -1);
    QCOMPARE(list.lastIndexOf(re, 9999), 2);
}

void tst_QStringList::indexOf()
{
    QStringList list;
    list << "harald" << "trond" << "vohi" << "harald";

    QCOMPARE(list.indexOf("harald"), 0);
    QCOMPARE(list.indexOf("trond"), 1);
    QCOMPARE(list.indexOf("vohi"), 2);
    QCOMPARE(list.indexOf("harald", 1), 3);

    QCOMPARE(list.indexOf("hans"), -1);
    QCOMPARE(list.indexOf("trond", 2), -1);
    QCOMPARE(list.indexOf("harald", -1), 3);
    QCOMPARE(list.indexOf("vohi", -3), 2);
}

void tst_QStringList::lastIndexOf()
{
    QStringList list;
    list << "harald" << "trond" << "vohi" << "harald";

    QCOMPARE(list.lastIndexOf("harald"), 3);
    QCOMPARE(list.lastIndexOf("trond"), 1);
    QCOMPARE(list.lastIndexOf("vohi"), 2);
    QCOMPARE(list.lastIndexOf("harald", 2), 0);

    QCOMPARE(list.lastIndexOf("hans"), -1);
    QCOMPARE(list.lastIndexOf("vohi", 1), -1);
    QCOMPARE(list.lastIndexOf("vohi", -1), 2);
    QCOMPARE(list.lastIndexOf("vohi", -3), -1);
}

void tst_QStringList::filter()
{
    QStringList list1, list2;
    list1 << "Bill Gates" << "Joe Blow" << "Bill Clinton";
    list1 = list1.filter( "Bill" );
    list2 << "Bill Gates" << "Bill Clinton";
    QCOMPARE( list1, list2 );

    QStringList list3, list4;
    list3 << "Bill Gates" << "Joe Blow" << "Bill Clinton";
    list3 = list3.filter( QRegExp("[i]ll") );
    list4 << "Bill Gates" << "Bill Clinton";
    QCOMPARE( list3, list4 );
}

void tst_QStringList::replaceInStrings()
{
    QStringList list1, list2;
    list1 << "alpha" << "beta" << "gamma" << "epsilon";
    list1.replaceInStrings( "a", "o" );
    list2 << "olpho" << "beto" << "gommo" << "epsilon";
    QCOMPARE( list1, list2 );

    QStringList list3, list4;
    list3 << "alpha" << "beta" << "gamma" << "epsilon";
    list3.replaceInStrings( QRegExp("^a"), "o" );
    list4 << "olpha" << "beta" << "gamma" << "epsilon";
    QCOMPARE( list3, list4 );

    QStringList list5, list6;
    list5 << "Bill Clinton" << "Gates, Bill";
    list6 << "Bill Clinton" << "Bill Gates";
    list5.replaceInStrings( QRegExp("^(.*), (.*)$"), "\\2 \\1" );
    QCOMPARE( list5, list6 );
}

void tst_QStringList::contains()
{
    QStringList list;
    list << "arthur" << "Arthur" << "arthuR" << "ARTHUR" << "Dent" << "Hans Dent";

    QVERIFY(list.contains("arthur"));
    QVERIFY(!list.contains("ArthuR"));
    QVERIFY(!list.contains("Hans"));
    QVERIFY(list.contains("arthur", Qt::CaseInsensitive));
    QVERIFY(list.contains("ArthuR", Qt::CaseInsensitive));
    QVERIFY(list.contains("ARTHUR", Qt::CaseInsensitive));
    QVERIFY(list.contains("dent", Qt::CaseInsensitive));
    QVERIFY(!list.contains("hans", Qt::CaseInsensitive));
}

void tst_QStringList::streamingOperator()
{
    QStringList list;
    list << "hei";
    list << list << "hopp" << list;

    QCOMPARE(list, QStringList()
            << "hei" << "hei" << "hopp"
            << "hei" << "hei" << "hopp");

    QStringList list2;
    list2 << "adam";

    QStringList list3;
    list3 << "eva";

    QCOMPARE(list2 << list3, QStringList() << "adam" << "eva");
}

QTEST_APPLESS_MAIN(tst_QStringList)
#include "tst_qstringlist.moc"
