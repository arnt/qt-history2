/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsize.h>

Q_DECLARE_METATYPE(QSizeF)

//TESTED_CLASS=
//TESTED_FILES=gui/painting/qsize.h gui/painting/qsize.cpp

class tst_QSizeF : public QObject {
    Q_OBJECT

public:
    tst_QSizeF();
    virtual ~tst_QSizeF();

public slots:
    void init();
    void cleanup();

private slots:
    void scale();

    void expandedTo();
    void expandedTo_data();

    void boundedTo_data();
    void boundedTo();

    void transpose_data();
    void transpose();
};

#if QT_VERSION < 0x040100
namespace QTest
{
    // Qt < 4.1 didn't do fuzzy comparisons
    template<>
    inline bool compare(QSizeF const &t1, QSizeF const &t2, const char *file, int line)
    {
        char msg[1024];
        msg[0] = '\0';
        bool isOk = true;
        if ((qAbs(t1.width() - t2.width()) > 0.000000000001) || (qAbs(t1.height() - t2.height()) > 0.000000000001)) {
            qt_snprintf(msg, 1024, "Compared values of type QSizeF are not the same (fuzzy compare).\n"
                    "   Actual  : w: %lf h: %lf \n"
                    "   Expected: w: %lf h: %lf", t1.width(), t1.height(), t2.width(), t2.height());
            isOk = false;
        } else {
            qt_snprintf(msg, 1024, "QCOMPARE('%lf %lf', QSizeF)", t1.width(), t1.height());
        }
        return compare_helper(isOk, msg, file, line);
    }
}
#endif

tst_QSizeF::tst_QSizeF() {
}

tst_QSizeF::~tst_QSizeF() {
}

void tst_QSizeF::init() {
}

void tst_QSizeF::cleanup() {
}

void tst_QSizeF::scale() {
    QSizeF t1(10.4, 12.8);
    t1.scale(60.6, 60.6, Qt::IgnoreAspectRatio);
    QCOMPARE(t1, QSizeF(60.6, 60.6));

    QSizeF t2(10.4, 12.8);
    t2.scale(43.52, 43.52, Qt::KeepAspectRatio);
    QCOMPARE(t2, QSizeF(35.36, 43.52));

    QSizeF t3(9.6, 12.48);
    t3.scale(31.68, 31.68, Qt::KeepAspectRatioByExpanding);
    QCOMPARE(t3, QSizeF(31.68, 41.184));

    QSizeF t4(12.8, 10.4);
    t4.scale(43.52, 43.52, Qt::KeepAspectRatio);
    QCOMPARE(t4, QSizeF(43.52, 35.36));

    QSizeF t5(12.48, 9.6);
    t5.scale(31.68, 31.68, Qt::KeepAspectRatioByExpanding);
    QCOMPARE(t5, QSizeF(41.184, 31.68));

}


void tst_QSizeF::expandedTo_data() {
    QTest::addColumn<QSizeF>("input1");
    QTest::addColumn<QSizeF>("input2");
    QTest::addColumn<QSizeF>("expected");

    QTest::newRow("data0") << QSizeF(10.4, 12.8) << QSizeF(6.6, 4.4) << QSizeF(10.4, 12.8);
    QTest::newRow("data1") << QSizeF(0.0, 0.0) << QSizeF(6.6, 4.4) << QSizeF(6.6, 4.4);
    // This should pick the highest of w,h components independently of each other,
    // thus the result dont have to be equal to neither input1 nor input2.
    QTest::newRow("data3") << QSizeF(6.6, 4.4) << QSizeF(4.4, 6.6) << QSizeF(6.6, 6.6);
}

void tst_QSizeF::expandedTo() {
    QFETCH( QSizeF, input1);
    QFETCH( QSizeF, input2);
    QFETCH( QSizeF, expected);

    QCOMPARE( input1.expandedTo(input2), expected);
}

void tst_QSizeF::boundedTo_data() {
    QTest::addColumn<QSizeF>("input1");
    QTest::addColumn<QSizeF>("input2");
    QTest::addColumn<QSizeF>("expected");

    QTest::newRow("data0") << QSizeF(10.4, 12.8) << QSizeF(6.6, 4.4) << QSizeF(6.6, 4.4);
    QTest::newRow("data1") << QSizeF(0.0, 0.0) << QSizeF(6.6, 4.4) << QSizeF(0.0, 0.0);
    // This should pick the lowest of w,h components independently of each other,
    // thus the result dont have to be equal to neither input1 nor input2.
    QTest::newRow("data3") << QSizeF(6.6, 4.4) << QSizeF(4.4, 6.6) << QSizeF(4.4, 4.4);
}

void tst_QSizeF::boundedTo() {
    QFETCH( QSizeF, input1);
    QFETCH( QSizeF, input2);
    QFETCH( QSizeF, expected);

    QCOMPARE( input1.boundedTo(input2), expected);
}

void tst_QSizeF::transpose_data() {
    QTest::addColumn<QSizeF>("input1");
    QTest::addColumn<QSizeF>("expected");

    QTest::newRow("data0") << QSizeF(10.4, 12.8) << QSizeF(12.8, 10.4);
    QTest::newRow("data1") << QSizeF(0.0, 0.0) << QSizeF(0.0, 0.0);
    QTest::newRow("data3") << QSizeF(6.6, 4.4) << QSizeF(4.4, 6.6);
}

void tst_QSizeF::transpose() {
    QFETCH( QSizeF, input1);
    QFETCH( QSizeF, expected);

    // transpose() works only inplace and does not return anything, so we must do the operation itself before the compare.
    input1.transpose();
    QCOMPARE(input1 , expected);
}

QTEST_APPLESS_MAIN(tst_QSizeF)
#include "tst_qsizef.moc"
