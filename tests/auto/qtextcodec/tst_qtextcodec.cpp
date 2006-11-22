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
    QTest::addColumn<bool>("eightBit");

    QTest::newRow("data1") << "ISO-8859-1" << true;
    QTest::newRow("data2") << "ISO-8859-2" << true;
    QTest::newRow("data3") << "ISO-8859-3" << true;
    QTest::newRow("data4") << "ISO-8859-4" << true;
    QTest::newRow("data5") << "ISO-8859-5" << true;
    QTest::newRow("data6") << "ISO-8859-6" << true;
    QTest::newRow("data7") << "ISO-8859-7" << true;
    QTest::newRow("data8") << "ISO-8859-8" << true;
    QTest::newRow("data9") << "ISO-8859-9" << true;
    QTest::newRow("data10") << "ISO-8859-10" << true;
    QTest::newRow("data13") << "ISO-8859-13" << true;
    QTest::newRow("data14") << "ISO-8859-14" << true;
    QTest::newRow("data15") << "ISO-8859-15" << true;
    QTest::newRow("data16") << "ISO-8859-16" << true;

    QTest::newRow("data18") << "IBM850" << true;
    QTest::newRow("data19") << "IBM874" << true;
    QTest::newRow("data20") << "IBM866" << true;

    QTest::newRow("data21") << "windows-1250" << true;
    QTest::newRow("data22") << "windows-1251" << true;
    QTest::newRow("data23") << "windows-1252" << true;
    QTest::newRow("data24") << "windows-1253" << true;
    QTest::newRow("data25") << "windows-1254" << true;
    QTest::newRow("data26") << "windows-1255" << true;
    QTest::newRow("data27") << "windows-1256" << true;
    QTest::newRow("data28") << "windows-1257" << true;
    QTest::newRow("data28") << "windows-1258" << true;

    QTest::newRow("data29") << "Apple Roman" << true;
    QTest::newRow("data29") << "WINSAMI2" << true;
    QTest::newRow("data30") << "TIS-620" << true;
    QTest::newRow("data31") << "roman8" << true;

    QTest::newRow("data32") << "SJIS" << false;
    QTest::newRow("data33") << "EUC-KR" << false;
}

void tst_QTextCodec::fromUnicode()
{
    QFETCH(QString, codecName);
    QFETCH(bool, eightBit);

    QTextCodec *codec = QTextCodec::codecForName(codecName.toLatin1());
    QVERIFY(codec != 0);

    // Check if the reverse lookup is what we expect
    if (eightBit) {
        char chars[128];
        for (int i = 0; i < 128; ++i)
            chars[i] = i + 128;
        QString s = codec->toUnicode(chars, 128);
        QByteArray c = codec->fromUnicode(s);
        
        int numberOfQuestionMarks = 0;
        for (int i = 0; i < 128; ++i) {
            if (c.at(i) == '?')
                ++numberOfQuestionMarks;
            else
                QCOMPARE(c.at(i), char(i + 128));
        }
        QVERIFY(numberOfQuestionMarks != 128);
    }

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
