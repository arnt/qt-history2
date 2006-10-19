/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qtextcodec.h>
#include <qfile.h>
#include <qtextdocument.h>





//TESTED_CLASS=
//TESTED_FILES=corelib/codecs/qtextcodec.h corelib/codecs/qtextcodec.cpp

class tst_QTextCodec : public QObject
{
    Q_OBJECT

public:
    tst_QTextCodec();
    ~tst_QTextCodec();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void toUnicode_data();
    void toUnicode();
    void codecForName_data();
    void codecForName();
    void fromUnicode_data();
    void fromUnicode();
    void toUnicode_codecForHtml();
    void codecForLocale();
};


tst_QTextCodec::tst_QTextCodec()
{
}

tst_QTextCodec::~tst_QTextCodec()
{
}

void tst_QTextCodec::initTestCase()
{
}

void tst_QTextCodec::cleanupTestCase()
{
}

void tst_QTextCodec::init()
{
}

void tst_QTextCodec::cleanup()
{
}

void tst_QTextCodec::toUnicode_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("codecName");

    QTest::newRow( "korean-eucKR" ) << "korean.txt" << "eucKR";
}

void tst_QTextCodec::toUnicode()
{
    QFETCH( QString, fileName );
    QFETCH( QString, codecName );

    QFile file( fileName );

    if ( file.open( QIODevice::ReadOnly ) ) {
	QByteArray ba = file.readAll();
	QTextCodec *c = QTextCodec::codecForName( codecName.toLatin1() );
        QVERIFY(c != 0);
	QString uniString = c->toUnicode( ba );
	QCOMPARE( ba, c->fromUnicode( uniString ) );
        char ch = '\0';
        QVERIFY(c->toUnicode(&ch, 1).isEmpty());
        QVERIFY(c->toUnicode(&ch, 1).isNull());
    } else {
        QFAIL("File could not be opened");
    }
}

void tst_QTextCodec::codecForName_data()
{
    QTest::addColumn<QString>("hint");
    QTest::addColumn<QString>("actualCodecName");

    QTest::newRow("data1") << "iso88591" << "ISO-8859-1";
    QTest::newRow("data2") << "iso88592" << "ISO-8859-2";
    QTest::newRow("data3") << " IsO(8)8/5*9-2 " << "ISO-8859-2";
    QTest::newRow("data4") << " IsO(8)8/5*2-9 " << "";
    QTest::newRow("data5") << "latin2" << "ISO-8859-2";
}

void tst_QTextCodec::codecForName()
{
    QFETCH(QString, hint);
    QFETCH(QString, actualCodecName);

    QTextCodec *codec = QTextCodec::codecForName(hint.toLatin1());
    if (actualCodecName.isEmpty()) {
        QVERIFY(codec == 0);
    } else {
        QVERIFY(codec != 0);
        QCOMPARE(QString(codec->name()), actualCodecName);
    }
}

void tst_QTextCodec::fromUnicode_data()
{
    QTest::addColumn<QString>("codecName");

    QTest::newRow("data1") << "ISO-8859-1";
    QTest::newRow("data2") << "ISO-8859-2";
    QTest::newRow("data3") << "SJIS";
    QTest::newRow("data4") << "EUC-KR";
}

void tst_QTextCodec::fromUnicode()
{
    QFETCH(QString, codecName);

    QTextCodec *codec = QTextCodec::codecForName(codecName.toLatin1());
    QVERIFY(codec != 0);

    /*
        If the encoding is a superset of ASCII, test that the byte
        array is correct (no off by one, no trailing '\0').
    */
    QByteArray result = codec->fromUnicode(QString("abc"));
    if (result.startsWith("a")) {
        QCOMPARE(result.size(), 3);
        QCOMPARE(result, QByteArray("abc"));
    } else {
        QVERIFY(true);
    }
}

void tst_QTextCodec::toUnicode_codecForHtml()
{
    QFile file(QString("QT4-crashtest.txt"));
    QVERIFY(file.open(QFile::ReadOnly));

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    codec->toUnicode(data); // this line crashes
}

void tst_QTextCodec::codecForLocale()
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    QVERIFY(codec != 0);

#ifdef Q_OS_UNIX
    // get a time string that is locale-encoded
    QByteArray originalLocaleEncodedTimeString;
    originalLocaleEncodedTimeString.resize(1024);
    time_t t;
    time(&t);
    int r = strftime(originalLocaleEncodedTimeString.data(),
                     originalLocaleEncodedTimeString.size(),
                     "%A%a%B%b%Z",
                     localtime(&t));
    if (r == 0)
        QSKIP("strftime() failed", SkipAll);
    originalLocaleEncodedTimeString.resize(r);

    QString unicodeTimeString = codec->toUnicode(originalLocaleEncodedTimeString);
    QByteArray localeEncodedTimeString = codec->fromUnicode(unicodeTimeString);
    QCOMPARE(localeEncodedTimeString, originalLocaleEncodedTimeString);
#else
    QSKIP("This test is not implemented on Windows", SkipAll);
#endif
}

QTEST_MAIN(tst_QTextCodec)
#include "tst_qtextcodec.moc"
