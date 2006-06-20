/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qchar.h>
#include <qfile.h>
#include <private/qunicodetables_p.h>




//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qchar.h corelib/tools/qchar.cpp

class tst_QChar : public QObject
{
    Q_OBJECT

public:
    tst_QChar();
    ~tst_QChar();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void toUpper();
    void toLower();
    void isUpper();
    void isLower();
    void category();
    void digitValue();
    void decomposition();
    void ligature();
    void lineBreakClass();
    void normalization();
};

tst_QChar::tst_QChar()
{}

tst_QChar::~tst_QChar()
{ }

void tst_QChar::initTestCase()
{ }

void tst_QChar::cleanupTestCase()
{ }

void tst_QChar::init()
{ }

void tst_QChar::cleanup()
{
}

void tst_QChar::toUpper()
{
    QVERIFY(QChar('a').toUpper() == 'A');
    QVERIFY(QChar('A').toUpper() == 'A');
}

void tst_QChar::toLower()
{
    QVERIFY(QChar('A').toLower() == 'a');
    QVERIFY(QChar('a').toLower() == 'a');
}

void tst_QChar::isUpper()
{
    QVERIFY(QChar('A').isUpper());
    QVERIFY(QChar('Z').isUpper());
    QVERIFY(!QChar('a').isUpper());
    QVERIFY(!QChar('z').isUpper());
    QVERIFY(!QChar('?').isUpper());
    QVERIFY(QChar(0xC2).isUpper());   // A with ^
    QVERIFY(!QChar(0xE2).isUpper());  // a with ^
}

void tst_QChar::isLower()
{
    QVERIFY(!QChar('A').isLower());
    QVERIFY(!QChar('Z').isLower());
    QVERIFY(QChar('a').isLower());
    QVERIFY(QChar('z').isLower());
    QVERIFY(!QChar('?').isLower());
    QVERIFY(!QChar(0xC2).isLower());   // A with ^
    QVERIFY(QChar(0xE2).isLower());  // a with ^
}

void tst_QChar::category()
{
    QVERIFY(QChar('a').category() == QChar::Letter_Lowercase);
    QVERIFY(QChar('A').category() == QChar::Letter_Uppercase);

    QVERIFY(QUnicodeTables::category(0xe0100u) == QChar::Mark_NonSpacing);
    QVERIFY(QUnicodeTables::category(0xeffffu) != QChar::Other_PrivateUse);
    QVERIFY(QUnicodeTables::category(0xf0000u) == QChar::Other_PrivateUse);
    QVERIFY(QUnicodeTables::category(0xf0001u) == QChar::Other_PrivateUse);

    QVERIFY(QUnicodeTables::category(0xd900u) == QChar::Other_Surrogate);
    QVERIFY(QUnicodeTables::category(0xdc00u) == QChar::Other_Surrogate);
    QVERIFY(QUnicodeTables::category(0xdc01u) == QChar::Other_Surrogate);
}


void tst_QChar::digitValue()
{
    QVERIFY(QChar('9').digitValue() == 9);
    QVERIFY(QChar('0').digitValue() == 0);
}

void tst_QChar::decomposition()
{
    QVERIFY(QUnicodeTables::decompositionTag(0xa0) == QChar::NoBreak);
    QVERIFY(QUnicodeTables::decompositionTag(0xa8) == QChar::Compat);
    QVERIFY(QUnicodeTables::decompositionTag(0x41) == QChar::NoDecomposition);
    QVERIFY(QUnicodeTables::decomposition(0xa0) == QString(QChar(0x20)));
    QVERIFY(QUnicodeTables::decomposition(0xc0) == (QString(QChar(0x41)) + QString(QChar(0x300))));

    {
        QString str;
        str += QChar( (0x1D157 - 0x10000) / 0x400 + 0xd800 );
        str += QChar( ((0x1D157 - 0x10000) % 0x400) + 0xdc00 );
        str += QChar( (0x1D165 - 0x10000) / 0x400 + 0xd800 );
        str += QChar( ((0x1D165 - 0x10000) % 0x400) + 0xdc00 );
        QVERIFY(QUnicodeTables::decomposition(0x1D15e) == str);
    }

    {
        QString str;
        str += QChar(0x1100);
        str += QChar(0x1161);
        QVERIFY(QUnicodeTables::decomposition(0xac00) == str);
    }
    {
        QString str;
        str += QChar(0x110c);
        str += QChar(0x1165);
        str += QChar(0x11b7);
        QVERIFY(QUnicodeTables::decomposition(0xc810) == str);
    }
}

void tst_QChar::ligature()
{
    QVERIFY(QUnicodeTables::ligature(0x0041, 0x00300) == 0xc0);
    QVERIFY(QUnicodeTables::ligature(0x0049, 0x00308) == 0xcf);
    QVERIFY(QUnicodeTables::ligature(0x0391, 0x00301) == 0x386);
    QVERIFY(QUnicodeTables::ligature(0x0627, 0x00653) == 0x622);

    QVERIFY(QUnicodeTables::ligature(0x1100, 0x1161) == 0xac00);
    QVERIFY(QUnicodeTables::ligature(0xac00, 0x11a8) == 0xac01);
}

void tst_QChar::lineBreakClass()
{
    QVERIFY(QUnicodeTables::lineBreakClass(0x0041u) == QUnicodeTables::LineBreak_AL);
    QVERIFY(QUnicodeTables::lineBreakClass(0x0033u) == QUnicodeTables::LineBreak_NU);
    QVERIFY(QUnicodeTables::lineBreakClass(0xe0164u) == QUnicodeTables::LineBreak_CM);
    QVERIFY(QUnicodeTables::lineBreakClass(0x2f9a4u) == QUnicodeTables::LineBreak_ID);
    QVERIFY(QUnicodeTables::lineBreakClass(0x10000u) == QUnicodeTables::LineBreak_AL);
    QVERIFY(QUnicodeTables::lineBreakClass(0x0fffdu) == QUnicodeTables::LineBreak_AL);
}

void tst_QChar::normalization()
{
    {
        QString composed;
        composed += QChar(0xc0);
        QString decomposed;
        decomposed += QChar(0x41);
        decomposed += QChar(0x300);

        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_D) == decomposed);
        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_C) == composed);
        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_KD) == decomposed);
        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_KC) == composed);
    }
    {
        QString composed;
        composed += QChar(0xa0);
        QString decomposed;
        decomposed += QChar(0x20);

        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_D) == composed);
        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_C) == composed);
        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_KD) == decomposed);
        QVERIFY(QUnicodeTables::normalize(composed, QString::NormalizationForm_KC) == decomposed);
    }

    QFile f("NormalizationTest.txt");
    if (!f.exists()) {
        QSKIP("Couldn't find NormalizationTest.txt", SkipAll);
        return;
    }

    f.open(QIODevice::ReadOnly);

    while (!f.atEnd()) {
        QByteArray line;
        line.resize(1024);
        int len = f.readLine(line.data(), 1024);
        line.resize(len-1);

        int comment = line.indexOf('#');
        if (comment >= 0)
            line = line.left(comment);

        if (line.startsWith("@"))
            continue;

        if (line.isEmpty())
            continue;

        line = line.trimmed();
        if (line.endsWith(';'))
            line.truncate(line.length()-1);

        QList<QByteArray> l = line.split(';');

        Q_ASSERT(l.size() == 5);

        QString columns[5];
        for (int i = 0; i < 5; ++i) {
            QList<QByteArray> c = l.at(i).split(' ');
            Q_ASSERT(!c.isEmpty());

            for (int j = 0; j < c.size(); ++j) {
                bool ok;
                uint uc = c.at(j).toInt(&ok, 16);
                if (uc < 0x10000)
                    columns[i].append(QChar(uc));
                else {
                    // convert to utf16
                    uc -= 0x10000;
                    ushort high = uc/0x400 + 0xd800;
                    ushort low = uc%0x400 + 0xdc00;
                    columns[i].append(QChar(high));
                    columns[i].append(QChar(low));
                }
            }
        }

        // CONFORMANCE:
        // 1. The following invariants must be true for all conformant implementations
        //
        //    NFC
        //      c2 ==  NFC(c1) ==  NFC(c2) ==  NFC(c3)
        //      c4 ==  NFC(c4) ==  NFC(c5)

        QVERIFY(columns[1] == QUnicodeTables::normalize(columns[0], QString::NormalizationForm_C));
        QVERIFY(columns[1] == QUnicodeTables::normalize(columns[1], QString::NormalizationForm_C));
        QVERIFY(columns[1] == QUnicodeTables::normalize(columns[2], QString::NormalizationForm_C));
        QVERIFY(columns[3] == QUnicodeTables::normalize(columns[3], QString::NormalizationForm_C));
        QVERIFY(columns[3] == QUnicodeTables::normalize(columns[4], QString::NormalizationForm_C));

        //    NFD
        //      c3 ==  NFD(c1) ==  NFD(c2) ==  NFD(c3)
        //      c5 ==  NFD(c4) ==  NFD(c5)

        QVERIFY(columns[2] == QUnicodeTables::normalize(columns[0], QString::NormalizationForm_D));
        QVERIFY(columns[2] == QUnicodeTables::normalize(columns[1], QString::NormalizationForm_D));
        QVERIFY(columns[2] == QUnicodeTables::normalize(columns[2], QString::NormalizationForm_D));
        QVERIFY(columns[4] == QUnicodeTables::normalize(columns[3], QString::NormalizationForm_D));
        QVERIFY(columns[4] == QUnicodeTables::normalize(columns[4], QString::NormalizationForm_D));

        //    NFKC
        //      c4 == NFKC(c1) == NFKC(c2) == NFKC(c3) == NFKC(c4) == NFKC(c5)

        QVERIFY(columns[3] == QUnicodeTables::normalize(columns[0], QString::NormalizationForm_KC));
        QVERIFY(columns[3] == QUnicodeTables::normalize(columns[1], QString::NormalizationForm_KC));
        QVERIFY(columns[3] == QUnicodeTables::normalize(columns[2], QString::NormalizationForm_KC));
        QVERIFY(columns[3] == QUnicodeTables::normalize(columns[3], QString::NormalizationForm_KC));
        QVERIFY(columns[3] == QUnicodeTables::normalize(columns[4], QString::NormalizationForm_KC));

        //    NFKD
        //      c5 == NFKD(c1) == NFKD(c2) == NFKD(c3) == NFKD(c4) == NFKD(c5)

        QVERIFY(columns[4] == QUnicodeTables::normalize(columns[0], QString::NormalizationForm_KD));
        QVERIFY(columns[4] == QUnicodeTables::normalize(columns[1], QString::NormalizationForm_KD));
        QVERIFY(columns[4] == QUnicodeTables::normalize(columns[2], QString::NormalizationForm_KD));
        QVERIFY(columns[4] == QUnicodeTables::normalize(columns[3], QString::NormalizationForm_KD));
        QVERIFY(columns[4] == QUnicodeTables::normalize(columns[4], QString::NormalizationForm_KD));

        // 2. For every code point X assigned in this version of Unicode that is not specifically
        //    listed in Part 1, the following invariants must be true for all conformant
        //    implementations:
        //
        //      X == NFC(X) == NFD(X) == NFKC(X) == NFKD(X)

        // #################

    }
}

QTEST_APPLESS_MAIN(tst_QChar)
#include "tst_qchar.moc"
