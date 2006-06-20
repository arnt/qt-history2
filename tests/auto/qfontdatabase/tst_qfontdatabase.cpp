/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qfontdatabase.h>





//TESTED_CLASS=
//TESTED_FILES=gui/text/qfontdatabase.h gui/text/qfontdatabase.cpp

class tst_QFontDatabase : public QObject
{
Q_OBJECT

public:
    tst_QFontDatabase();
    virtual ~tst_QFontDatabase();

public slots:
    void init();
    void cleanup();
private slots:
    void styles_data();
    void styles();

    void fixedPitch_data();
    void fixedPitch();

    void widthTwoTimes_data();
    void widthTwoTimes();
};

tst_QFontDatabase::tst_QFontDatabase()
{
}

tst_QFontDatabase::~tst_QFontDatabase()
{

}

void tst_QFontDatabase::init()
{
// TODO: Add initialization code here.
// This will be executed immediately before each test is run.
}

void tst_QFontDatabase::cleanup()
{
// TODO: Add cleanup code here.
// This will be executed immediately after each test is run.
}

void tst_QFontDatabase::styles_data()
{
    QTest::addColumn<QString>("font");

    QTest::newRow( "data0" ) << QString( "Times New Roman" );
}

void tst_QFontDatabase::styles()
{
    QFETCH( QString, font );

    QFontDatabase fdb;
    QStringList styles = fdb.styles( font );
    QStringList::Iterator it = styles.begin();
    while ( it != styles.end() ) {
	QString style = *it;
	QString trimmed = style.trimmed();
	++it;

	QCOMPARE( style, trimmed );
    }
}

void tst_QFontDatabase::fixedPitch_data()
{
    QTest::addColumn<QString>("font");
    QTest::addColumn<bool>("fixedPitch");

    QTest::newRow( "Times New Roman" ) << QString( "Times New Roman" ) << false;
    QTest::newRow( "Arial" ) << QString( "Arial" ) << false;
    QTest::newRow( "Script" ) << QString( "Script" ) << false;
    QTest::newRow( "Courier" ) << QString( "Courier" ) << true;
    QTest::newRow( "Courier New" ) << QString( "Courier New" ) << true;
    QTest::newRow( "Lucida Console" ) << QString( "Lucida Console" ) << true;
}

void tst_QFontDatabase::fixedPitch()
{
    QFETCH(QString, font);
    QFETCH(bool, fixedPitch);

    QFontDatabase fdb;
    if (!fdb.families().contains(font))
	QSKIP( "Font not installed", SkipSingle);

#if QT_VERSION >= 0x030200
    QCOMPARE(fdb.isFixedPitch(font), fixedPitch);
#endif

    QFont qfont(font);
    QFontInfo fi(qfont);
    QCOMPARE(fi.fixedPitch(), fixedPitch);
}

void tst_QFontDatabase::widthTwoTimes_data()
{
    QTest::addColumn<QString>("font");
    QTest::addColumn<int>("pixelSize");
    QTest::addColumn<QString>("text");

    QTest::newRow("Arial") << QString("Arial") << 1000 << QString("Some text");
}

void tst_QFontDatabase::widthTwoTimes()
{
    QFETCH(QString, font);
    QFETCH(int, pixelSize);
    QFETCH(QString, text);

    QFont f;
    f.setFamily(font);
    f.setPixelSize(pixelSize);

    QFontMetrics fm(f);
    int w1 = fm.charWidth(text, 0);
    int w2 = fm.charWidth(text, 0);

    QCOMPARE(w1, w2);
}

QTEST_MAIN(tst_QFontDatabase)
#include "tst_qfontdatabase.moc"
