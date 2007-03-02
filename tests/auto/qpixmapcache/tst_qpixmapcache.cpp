/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qpixmapcache.h>





//TESTED_CLASS=
//TESTED_FILES=gui/image/qpixmapcache.h gui/image/qpixmapcache.cpp

class tst_QPixmapCache : public QObject
{
    Q_OBJECT

public:
    tst_QPixmapCache();
    virtual ~tst_QPixmapCache();


public slots:
    void init();
private slots:
    void cacheLimit();
    void setCacheLimit();
    void find();
    void insert();
    void remove();
    void clear();
};


static int originalCacheLimit;

tst_QPixmapCache::tst_QPixmapCache()
{
    originalCacheLimit = QPixmapCache::cacheLimit();
}

tst_QPixmapCache::~tst_QPixmapCache()
{
}

void tst_QPixmapCache::init()
{
    QPixmapCache::setCacheLimit(originalCacheLimit);
    QPixmapCache::clear();
}

void tst_QPixmapCache::cacheLimit()
{
    // make sure the default is reasonable; it was 1024 last time I looked at it
    QVERIFY(originalCacheLimit >= 128 && originalCacheLimit <= 8192);

    QPixmapCache::setCacheLimit(100);
    QCOMPARE(QPixmapCache::cacheLimit(), 100);

    QPixmapCache::setCacheLimit(-50);
    QCOMPARE(QPixmapCache::cacheLimit(), -50);
}

void tst_QPixmapCache::setCacheLimit()
{
    QPixmap *p1 = new QPixmap(2, 3);
    QPixmapCache::insert("P1", *p1);
    QVERIFY(QPixmapCache::find("P1") != 0);
    delete p1;

    QPixmapCache::setCacheLimit(0);
    QVERIFY(QPixmapCache::find("P1") == 0);

    p1 = new QPixmap(2, 3);
    QPixmapCache::setCacheLimit(1000);
    QPixmapCache::insert("P1", *p1);
    QVERIFY(QPixmapCache::find("P1") != 0);

    delete p1;
}

void tst_QPixmapCache::find()
{
    QPixmap p1(10, 10);
    p1.fill(Qt::red);
    QPixmapCache::insert("P1", p1);

    QPixmap p2;
    QVERIFY(QPixmapCache::find("P1", p2));
    QCOMPARE(p2.width(), 10);
    QCOMPARE(p2.height(), 10);
    QCOMPARE(p1, p2);

    // obsolete
    QPixmap *p3 = QPixmapCache::find("P1");
    QVERIFY(p3);
    QCOMPARE(p1, *p3);
}

void tst_QPixmapCache::insert()
{
    QPixmap p1(10, 10);
    p1.fill(Qt::red);

    QPixmap p2(10, 10);
    p2.fill(Qt::yellow);

    // make sure it doesn't explode
    for (int i = 0; i < 20000; ++i)
        QPixmapCache::insert("0", p1);

    // ditto
    for (int j = 0; j < 20000; ++j)
        QPixmapCache::insert(QString::number(j), p1);

    int num = 0;
    for (int k = 0; k < 20000; ++k) {
        if (QPixmapCache::find(QString::number(k)))
            ++num;
    }

    int estimatedNum = (1024 * QPixmapCache::cacheLimit())
                       / ((p1.width() * p1.height() * p1.depth()) / 8);
    QVERIFY(estimatedNum - 1 <= num <= estimatedNum + 1);
}

void tst_QPixmapCache::remove()
{
    QPixmap p1(10, 10);
    p1.fill(Qt::red);

    QPixmapCache::insert("red", p1);
    p1.fill(Qt::yellow);

    QPixmap p2;
    QVERIFY(QPixmapCache::find("red", p2));
    QVERIFY(p1.toImage() != p2.toImage());
    QVERIFY(p1.toImage() == p1.toImage()); // sanity check

    QPixmapCache::remove("red");
    QVERIFY(QPixmapCache::find("red") == 0);
    QPixmapCache::remove("red");
    QVERIFY(QPixmapCache::find("red") == 0);

    QPixmapCache::remove("green");
    QVERIFY(QPixmapCache::find("green") == 0);
}

void tst_QPixmapCache::clear()
{
    QPixmap p1(10, 10);
    p1.fill(Qt::red);

    for (int i = 0; i < 20000; ++i) {
        QVERIFY(QPixmapCache::find("x" + QString::number(i)) == 0);
    }
    for (int j = 0; j < 20000; ++j) {
        QPixmapCache::insert(QString::number(j), p1);
    }

    int num = 0;
    for (int k = 0; k < 20000; ++k) {
        if (QPixmapCache::find(QString::number(k), p1))
            ++num;
    }
    QVERIFY(num > 0);

    QPixmapCache::clear();

    for (int k = 0; k < 20000; ++k) {
        QVERIFY(QPixmapCache::find(QString::number(k)) == 0);
    }
}

QTEST_MAIN(tst_QPixmapCache)
#include "tst_qpixmapcache.moc"
