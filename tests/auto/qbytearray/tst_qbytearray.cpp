/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qbytearray.h>
#include <qfile.h>
#include <limits.h>
#include <private/qtools_p.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qbytearray.h corelib/tools/qbytearray.cpp

class tst_QByteArray : public QObject
{
    Q_OBJECT

public:
    tst_QByteArray();
    virtual ~tst_QByteArray();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void qCompress_data();
    void qCompress();
    void qUncompress_data();
    void qUncompress();
    void constByteArray();
    void leftJustified();
    void rightJustified();
    void startsWith_data();
    void startsWith();
    void startsWith_char();
    void endsWith_data();
    void endsWith();
    void endsWith_char();
    void split_data();
    void split();
    void base64_data();
    void base64();
    void qvsnprintf();
    void qstrlen();
    void qstrnlen();
    void qstrcpy();
    void qstrncpy();
    void qstricmp_data();
    void qstricmp();
    void qstricmp_singularities();
    void qstrnicmp_singularities();
    void chop_data();
    void chop();
    void prepend();
    void append();
    void insert();
    void remove_data();
    void remove();
    void replace_data();
    void replace();
    void indexOf_data();
    void indexOf();
    void lastIndexOf_data();
    void lastIndexOf();
    void toULong_data();
    void toULong();
    void toULongLong_data();
    void toULongLong();

    void number();
    void toInt_data();
    void toInt();
    void qAllocMore();

    void resizeAfterFromRawData();
};

tst_QByteArray::tst_QByteArray()
{
    qRegisterMetaType<qulonglong>("qulonglong");
}

tst_QByteArray::~tst_QByteArray()
{
}

void tst_QByteArray::initTestCase()
{
}

void tst_QByteArray::cleanupTestCase()
{
}

void tst_QByteArray::init()
{
}

void tst_QByteArray::cleanup()
{
}

void tst_QByteArray::qCompress_data()
{
    QTest::addColumn<QByteArray>("ba");

    const int size1 = 1024*1024;
    QByteArray ba1( size1, 0 );

    QTest::newRow( "00" ) << QByteArray();

    int i;
    for ( i=0; i<size1; i++ )
	ba1[i] = (char)( i / 1024 );
    QTest::newRow( "01" ) << ba1;

    for ( i=0; i<size1; i++ )
	ba1[i] = (char)( i % 256 );
    QTest::newRow( "02" ) << ba1;

    ba1.fill( 'A' );
    QTest::newRow( "03" ) << ba1;

    QFile file( "rfc3252.txt" );
    QVERIFY( file.open(QIODevice::ReadOnly) );
    QTest::newRow( "04" ) << file.readAll();
}

void tst_QByteArray::qCompress()
{
    QFETCH( QByteArray, ba );
    QByteArray compressed = ::qCompress( ba );
    //qDebug( "size uncompressed: %d    size compressed: %d", ba.size(), compressed.size() );
    QTEST( ::qUncompress( compressed ), "ba" );
}

/*
    Just making sure it doesn't crash on invalid data.
*/
void tst_QByteArray::qUncompress_data()
{
    QTest::addColumn<QByteArray>("in");
    QTest::addColumn<QByteArray>("out");

    QTest::newRow("0x00000000") << QByteArray("\x00\x00\x00\x00") << QByteArray();
    QTest::newRow("0x000000ff") << QByteArray("\x00\x00\x00\xff") << QByteArray();
    QTest::newRow("0x3f000000") << QByteArray("\x3f\x00\x00\x00") << QByteArray();
    QTest::newRow("0x3fffffff") << QByteArray("\x3f\xff\xff\xff") << QByteArray();
    QTest::newRow("0x7fffff00") << QByteArray("\x7f\xff\xff\x00") << QByteArray();
    QTest::newRow("0x7fffffff") << QByteArray("\x7f\xff\xff\xff") << QByteArray();
    QTest::newRow("0x80000000") << QByteArray("\x80\x00\x00\x00") << QByteArray();
    QTest::newRow("0x800000ff") << QByteArray("\x80\x00\x00\xff") << QByteArray();
    QTest::newRow("0xcf000000") << QByteArray("\xcf\x00\x00\x00") << QByteArray();
    QTest::newRow("0xcfffffff") << QByteArray("\xcf\xff\xff\xff") << QByteArray();
    QTest::newRow("0xffffff00") << QByteArray("\xff\xff\xff\x00") << QByteArray();
    QTest::newRow("0xffffffff") << QByteArray("\xff\xff\xff\xff") << QByteArray();
}

void tst_QByteArray::qUncompress()
{
    QFETCH(QByteArray, in);
    QTEST(::qUncompress(in), "out");
    QTEST(::qUncompress(in + "blah"), "out");
}

void tst_QByteArray::constByteArray()
{
    const char *ptr = "abc";
    QByteArray cba = QByteArray::fromRawData(ptr, 3);
    QVERIFY(cba.constData() == ptr);
    cba.squeeze();
    QVERIFY(cba.constData() == ptr);
    cba.detach();
    QVERIFY(cba.size() == 3);
    QVERIFY(cba.capacity() == 3);
    QVERIFY(cba.constData() != ptr);
    QVERIFY(cba.constData()[0] == 'a');
    QVERIFY(cba.constData()[1] == 'b');
    QVERIFY(cba.constData()[2] == 'c');
    QVERIFY(cba.constData()[3] == '\0');
}

void tst_QByteArray::leftJustified()
{
    QByteArray a;
    a = "ABC";
    QCOMPARE(a.leftJustified(5,'-'), QByteArray("ABC--"));
    QCOMPARE(a.leftJustified(4,'-'), QByteArray("ABC-"));
    QCOMPARE(a.leftJustified(4), QByteArray("ABC "));
    QCOMPARE(a.leftJustified(3), QByteArray("ABC"));
    QCOMPARE(a.leftJustified(2), QByteArray("ABC"));
    QCOMPARE(a.leftJustified(1), QByteArray("ABC"));
    QCOMPARE(a.leftJustified(0), QByteArray("ABC"));

    QByteArray n;
    QVERIFY(!n.leftJustified(3).isNull());    // I expected true
    QCOMPARE(a.leftJustified(4,' ',true), QByteArray("ABC "));
    QCOMPARE(a.leftJustified(3,' ',true), QByteArray("ABC"));
    QCOMPARE(a.leftJustified(2,' ',true), QByteArray("AB"));
    QCOMPARE(a.leftJustified(1,' ',true), QByteArray("A"));
    QCOMPARE(a.leftJustified(0,' ',true), QByteArray(""));
}

void tst_QByteArray::rightJustified()
{
    QByteArray a;
    a="ABC";
    QCOMPARE(a.rightJustified(5,'-'),QByteArray("--ABC"));
    QCOMPARE(a.rightJustified(4,'-'),QByteArray("-ABC"));
    QCOMPARE(a.rightJustified(4),QByteArray(" ABC"));
    QCOMPARE(a.rightJustified(3),QByteArray("ABC"));
    QCOMPARE(a.rightJustified(2),QByteArray("ABC"));
    QCOMPARE(a.rightJustified(1),QByteArray("ABC"));
    QCOMPARE(a.rightJustified(0),QByteArray("ABC"));

    QByteArray n;
    QVERIFY(!n.rightJustified(3).isNull());  // I expected true
    QCOMPARE(a.rightJustified(4,'-',true),QByteArray("-ABC"));
    QCOMPARE(a.rightJustified(4,' ',true),QByteArray(" ABC"));
    QCOMPARE(a.rightJustified(3,' ',true),QByteArray("ABC"));
    QCOMPARE(a.rightJustified(2,' ',true),QByteArray("AB"));
    QCOMPARE(a.rightJustified(1,' ',true),QByteArray("A"));
    QCOMPARE(a.rightJustified(0,' ',true),QByteArray(""));
    QCOMPARE(a,QByteArray("ABC"));
}

void tst_QByteArray::startsWith_data()
{
    QTest::addColumn<QByteArray>("ba");
    QTest::addColumn<QByteArray>("sw");
    QTest::addColumn<bool>("result");

    QTest::newRow("01") << QByteArray() << QByteArray() << true;
    QTest::newRow("02") << QByteArray() << QByteArray("") << true;
    QTest::newRow("03") << QByteArray() << QByteArray("hallo") << false;

    QTest::newRow("04") << QByteArray("") << QByteArray() << true;
    QTest::newRow("05") << QByteArray("") << QByteArray("") << true;
    QTest::newRow("06") << QByteArray("") << QByteArray("h") << false;

    QTest::newRow("07") << QByteArray("hallo") << QByteArray("h") << true;
    QTest::newRow("08") << QByteArray("hallo") << QByteArray("hallo") << true;
    QTest::newRow("09") << QByteArray("hallo") << QByteArray("") << true;
    QTest::newRow("10") << QByteArray("hallo") << QByteArray("hallohallo") << false;
    QTest::newRow("11") << QByteArray("hallo") << QByteArray() << true;
}

void tst_QByteArray::startsWith()
{
    QFETCH(QByteArray, ba);
    QFETCH(QByteArray, sw);
    QFETCH(bool, result);

    QVERIFY(ba.startsWith(sw) == result);

    if (sw.isNull()) {
        QVERIFY(ba.startsWith((char*)0) == result);
    } else {
        QVERIFY(ba.startsWith(sw.data()) == result);
    }
}

void tst_QByteArray::startsWith_char()
{
    QVERIFY(QByteArray("hallo").startsWith('h'));
    QVERIFY(!QByteArray("hallo").startsWith('\0'));
    QVERIFY(!QByteArray("hallo").startsWith('o'));
    QVERIFY(QByteArray("h").startsWith('h'));
    QVERIFY(!QByteArray("h").startsWith('\0'));
    QVERIFY(!QByteArray("h").startsWith('o'));
    QVERIFY(!QByteArray("hallo").startsWith('l'));
    QVERIFY(!QByteArray("").startsWith('\0'));
    QVERIFY(!QByteArray("").startsWith('a'));
    QVERIFY(!QByteArray().startsWith('a'));
    QVERIFY(!QByteArray().startsWith('\0'));
}

void tst_QByteArray::endsWith_data()
{
    QTest::addColumn<QByteArray>("ba");
    QTest::addColumn<QByteArray>("sw");
    QTest::addColumn<bool>("result");

    QTest::newRow("01") << QByteArray() << QByteArray() << true;
    QTest::newRow("02") << QByteArray() << QByteArray("") << true;
    QTest::newRow("03") << QByteArray() << QByteArray("hallo") << false;

    QTest::newRow("04") << QByteArray("") << QByteArray() << true;
    QTest::newRow("05") << QByteArray("") << QByteArray("") << true;
    QTest::newRow("06") << QByteArray("") << QByteArray("h") << false;

    QTest::newRow("07") << QByteArray("hallo") << QByteArray("o") << true;
    QTest::newRow("08") << QByteArray("hallo") << QByteArray("hallo") << true;
    QTest::newRow("09") << QByteArray("hallo") << QByteArray("") << true;
    QTest::newRow("10") << QByteArray("hallo") << QByteArray("hallohallo") << false;
    QTest::newRow("11") << QByteArray("hallo") << QByteArray() << true;
}

void tst_QByteArray::endsWith()
{
    QFETCH(QByteArray, ba);
    QFETCH(QByteArray, sw);
    QFETCH(bool, result);

    QVERIFY(ba.endsWith(sw) == result);

    if (sw.isNull()) {
        QVERIFY(ba.endsWith((char*)0) == result);
    } else {
        QVERIFY(ba.endsWith(sw.data()) == result);
    }
}

void tst_QByteArray::endsWith_char()
{
    QVERIFY(QByteArray("hallo").endsWith('o'));
    QVERIFY(!QByteArray("hallo").endsWith('\0'));
    QVERIFY(!QByteArray("hallo").endsWith('h'));
    QVERIFY(QByteArray("h").endsWith('h'));
    QVERIFY(!QByteArray("h").endsWith('\0'));
    QVERIFY(!QByteArray("h").endsWith('o'));
    QVERIFY(!QByteArray("hallo").endsWith('l'));
    QVERIFY(!QByteArray("").endsWith('\0'));
    QVERIFY(!QByteArray("").endsWith('a'));
    QVERIFY(!QByteArray().endsWith('a'));
    QVERIFY(!QByteArray().endsWith('\0'));
}

void tst_QByteArray::split_data()
{
    QTest::addColumn<QByteArray>("sample");
    QTest::addColumn<int>("size");

    QTest::newRow("1") << QByteArray("-rw-r--r--  1 0  0  519240 Jul  9  2002 bigfile")
		    << 14;
    QTest::newRow("2") << QByteArray("abcde") << 1;
    QTest::newRow("one empty") << QByteArray("") << 1;
    QTest::newRow("two empty") << QByteArray(" ") << 2;
    QTest::newRow("three empty") << QByteArray("  ") << 3;

}

void tst_QByteArray::split()
{
    QFETCH(QByteArray, sample);
    QFETCH(int, size);

    QList<QByteArray> list = sample.split(' ');
    QCOMPARE(list.count(), size);
}

void tst_QByteArray::base64_data()
{
    QTest::addColumn<QByteArray>("rawdata");
    QTest::addColumn<QByteArray>("base64");

    QTest::newRow("1") << QByteArray("") << QByteArray("");
    QTest::newRow("2") << QByteArray("1") << QByteArray("MQ==");
    QTest::newRow("3") << QByteArray("12") << QByteArray("MTI=");
    QTest::newRow("4") << QByteArray("123") << QByteArray("MTIz");
    QTest::newRow("5") << QByteArray("1234") << QByteArray("MTIzNA==");
    QTest::newRow("6") << QByteArray("\n") << QByteArray("Cg==");
    QTest::newRow("7") << QByteArray("a\n") << QByteArray("YQo=");
    QTest::newRow("8") << QByteArray("ab\n") << QByteArray("YWIK");
    QTest::newRow("9") << QByteArray("abc\n") << QByteArray("YWJjCg==");
    QTest::newRow("a") << QByteArray("abcd\n") << QByteArray("YWJjZAo=");
    QTest::newRow("b") << QByteArray("abcde\n") << QByteArray("YWJjZGUK");
    QTest::newRow("c") << QByteArray("abcdef\n") << QByteArray("YWJjZGVmCg==");
    QTest::newRow("d") << QByteArray("abcdefg\n") << QByteArray("YWJjZGVmZwo=");
    QTest::newRow("e") << QByteArray("abcdefgh\n") << QByteArray("YWJjZGVmZ2gK");

    QByteArray ba;
    ba.resize(256);
    for (int i = 0; i < 256; ++i)
        ba[i] = i;
    QTest::newRow("f") << ba << QByteArray("AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==");
}


void tst_QByteArray::base64()
{
    QFETCH(QByteArray, rawdata);
    QFETCH(QByteArray, base64);

    QByteArray arr = QByteArray::fromBase64(base64);
    QCOMPARE(arr, rawdata);

    QByteArray arr64 = rawdata.toBase64();
    QCOMPARE(arr64, base64);
}

void tst_QByteArray::qvsnprintf()
{
    char buf[20];
    qMemSet(buf, 42, sizeof(buf));

    QCOMPARE(::qsnprintf(buf, 10, "%s", "bubu"), 4);
    QCOMPARE(static_cast<const char *>(buf), "bubu");
    QCOMPARE(buf[5], char(42));

    qMemSet(buf, 42, sizeof(buf));
    QCOMPARE(::qsnprintf(buf, 5, "%s", "bubu"), 4);
    QCOMPARE(static_cast<const char *>(buf), "bubu");
    QCOMPARE(buf[5], char(42));

    qMemSet(buf, 42, sizeof(buf));
#ifdef Q_OS_WIN
    // VS 2005 uses the Qt implementation of vsnprintf.
# if defined(_MSC_VER) && _MSC_VER >= 1400
    QCOMPARE(::qsnprintf(buf, 3, "%s", "bubu"), -1);
    QCOMPARE(static_cast<const char*>(buf), "bu");
# else
    // windows has to do everything different, of course.
    QCOMPARE(::qsnprintf(buf, 3, "%s", "bubu"), -1);
    buf[19] = '\0';
    QCOMPARE(static_cast<const char *>(buf), "bub****************");
# endif
#else
#ifdef Q_OS_IRIX
    // Irix reports back the amount of characters written without the \0
    QCOMPARE(::qsnprintf(buf, 3, "%s", "bubu"), 2);
#else
    // Every other system in this world reports the amount of data that could have been written
    QCOMPARE(::qsnprintf(buf, 3, "%s", "bubu"), 4);
#endif
    QCOMPARE(static_cast<const char*>(buf), "bu");
#endif
    QCOMPARE(buf[4], char(42));

#ifndef Q_OS_WIN
    qMemSet(buf, 42, sizeof(buf));
    QCOMPARE(::qsnprintf(buf, 10, ""), 0);
#endif
}


void tst_QByteArray::qstrlen()
{
    const char *src = "Something about ... \0 a string.";
    QCOMPARE(::qstrlen((char*)0), (uint)0);
    QCOMPARE(::qstrlen(src), (uint)20);
}

void tst_QByteArray::qstrnlen()
{
    const char *src = "Something about ... \0 a string.";
    QCOMPARE(::qstrnlen((char*)0, 1), (uint)0);
    QCOMPARE(::qstrnlen(src, 31), (uint)20);
    QCOMPARE(::qstrnlen(src, 19), (uint)19);
    QCOMPARE(::qstrnlen(src, 21), (uint)20);
    QCOMPARE(::qstrnlen(src, 20), (uint)20);
}

void tst_QByteArray::qstrcpy()
{
    const char *src = "Something about ... \0 a string.";
    const char *expected = "Something about ... ";
    char dst[128];

    QCOMPARE(::qstrcpy(0, 0), (char*)0);
    QCOMPARE(::qstrcpy(dst, 0), (char*)0);

    QCOMPARE(::qstrcpy(dst ,src), (char *)dst);
    QCOMPARE((char *)dst, const_cast<char *>(expected));
}

void tst_QByteArray::qstrncpy()
{
    QByteArray src(1024, 'a'), dst(1024, 'b');

    // singularities
    QCOMPARE(::qstrncpy(0, 0,0), (char*)0);
    QCOMPARE(::qstrncpy(dst.data(), 0, 0), (char*)0);

    // normal copy
    QCOMPARE(::qstrncpy(dst.data(), src.data(), src.size()), dst.data());

    src = QByteArray( "Tumdelidum" );
    QCOMPARE(QByteArray(::qstrncpy(dst.data(), src.data(), src.size())),
            QByteArray("Tumdelidu"));

    // normal copy with length is longer than neccessary
    src = QByteArray( "Tumdelidum\0foo" );
    dst.resize(128*1024);
    QCOMPARE(QByteArray(::qstrncpy(dst.data(), src.data(), dst.size())),
            QByteArray("Tumdelidum"));
}

void tst_QByteArray::qstricmp_data()
{
    QTest::addColumn<QString>("str1");
    QTest::addColumn<QString>("str2");

    QTest::newRow("equal   1") << "abcEdb"  << "abcEdb";
    QTest::newRow("equal   2") << "abcEdb"  << "ABCeDB";
    QTest::newRow("equal   3") << "ABCEDB"  << "abcedb";
    QTest::newRow("less    1") << "abcdef"  << "abcdefg";
    QTest::newRow("less    2") << "abcdeF"  << "abcdef";
    QTest::newRow("greater 1") << "abcdef"  << "abcdeF";
    QTest::newRow("greater 2") << "abcdefg" << "abcdef";
}

void tst_QByteArray::qstricmp()
{
    QFETCH(QString, str1);
    QFETCH(QString, str2);

    int expected = strcmp(str1.toUpper().toLatin1(),
                          str2.toUpper().toLatin1());
    if ( expected != 0 ) {
        expected = (expected < 0 ? -1 : 1);
    }
    int actual = ::qstricmp(str1.toLatin1(), str2.toLatin1());
    if ( actual != 0 ) {
        actual = (actual < 0 ? -1 : 1);
    }
    QCOMPARE(expected, actual);
}

void tst_QByteArray::qstricmp_singularities()
{
    QCOMPARE(::qstricmp(0, 0), 0);
    QVERIFY(::qstricmp(0, "a") != 0);
    QVERIFY(::qstricmp("a", 0) != 0);
    QCOMPARE(::qstricmp("", ""), 0);
}

void tst_QByteArray::qstrnicmp_singularities()
{
    QCOMPARE(::qstrnicmp(0, 0, 123), 0);
    QVERIFY(::qstrnicmp(0, "a", 123) != 0);
    QVERIFY(::qstrnicmp("a", 0, 123) != 0);
    QCOMPARE(::qstrnicmp("", "", 123), 0);
    QCOMPARE(::qstrnicmp("a", "B", 0), 0);
}

void tst_QByteArray::chop_data()
{
    QTest::addColumn<QByteArray>("src");
    QTest::addColumn<int>("choplength");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("1") << QByteArray("short1") << 128 << QByteArray();
    QTest::newRow("2") << QByteArray("short2") << int(strlen("short2"))
                    << QByteArray();
    QTest::newRow("3") << QByteArray("abcdef\0foo", 10) << 2
                    << QByteArray("abcdef\0f", 8);
    QTest::newRow("4") << QByteArray("STARTTLS\r\n") << 2
                    << QByteArray("STARTTLS");
    QTest::newRow("5") << QByteArray("") << 1 << QByteArray();
    QTest::newRow("6") << QByteArray("foo") << 0 << QByteArray("foo");
    QTest::newRow("7") << QByteArray(0) << 28 << QByteArray();
}

void tst_QByteArray::chop()
{
    QFETCH(QByteArray, src);
    QFETCH(int, choplength);
    QFETCH(QByteArray, expected);

    src.chop(choplength);
    QCOMPARE(src, expected);
}

void tst_QByteArray::prepend()
{
    QByteArray ba("foo");
    QCOMPARE(ba.prepend((char*)0), QByteArray("foo"));
    QCOMPARE(ba.prepend(QByteArray()), QByteArray("foo"));
    QCOMPARE(ba.prepend("1"), QByteArray("1foo"));
    QCOMPARE(ba.prepend(QByteArray("2")), QByteArray("21foo"));
    QCOMPARE(ba.prepend('3'), QByteArray("321foo"));
}

void tst_QByteArray::append()
{
    QByteArray ba("foo");
    QCOMPARE(ba.append((char*)0), QByteArray("foo"));
    QCOMPARE(ba.append(QByteArray()), QByteArray("foo"));
    QCOMPARE(ba.append("1"), QByteArray("foo1"));
    QCOMPARE(ba.append(QByteArray("2")), QByteArray("foo12"));
    QCOMPARE(ba.append('3'), QByteArray("foo123"));
}

void tst_QByteArray::insert()
{
    QByteArray ba("Meal");
    QCOMPARE(ba.insert(1, QByteArray("ontr")), QByteArray("Montreal"));
    QCOMPARE(ba.insert(ba.size(), "foo"), QByteArray("Montrealfoo"));

    ba = QByteArray("13");
    QCOMPARE(ba.insert(1, QByteArray("2")), QByteArray("123"));

    ba = "ac";
    QCOMPARE(ba.insert(1, 'b'), QByteArray("abc"));
    QCOMPARE(ba.size(), 3);

    ba = "ikl";
    QCOMPARE(ba.insert(1, "j"), QByteArray("ijkl"));
    QCOMPARE(ba.size(), 4);
}

void tst_QByteArray::remove_data()
{
    QTest::addColumn<QByteArray>("src");
    QTest::addColumn<int>("position");
    QTest::addColumn<int>("length");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("1") << QByteArray("Montreal") << 1 << 4
                    << QByteArray("Meal");
    QTest::newRow("2") << QByteArray() << 10 << 10 << QByteArray();
    QTest::newRow("3") << QByteArray("hi") << 0 << 10 << QByteArray();
    QTest::newRow("4") << QByteArray("Montreal") << 4 << 100
                    << QByteArray("Mont");

    // index out of range
    QTest::newRow("5") << QByteArray("Montreal") << 8 << 1
                    << QByteArray("Montreal");
    QTest::newRow("6") << QByteArray("Montreal") << 18 << 4
                    << QByteArray("Montreal");
}

void tst_QByteArray::remove()
{
    QFETCH(QByteArray, src);
    QFETCH(int, position);
    QFETCH(int, length);
    QFETCH(QByteArray, expected);
    QCOMPARE(src.remove(position, length), expected);
}

void tst_QByteArray::replace_data()
{
    QTest::addColumn<QByteArray>("src");
    QTest::addColumn<int>("pos");
    QTest::addColumn<int>("len");
    QTest::addColumn<QByteArray>("after");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("1") << QByteArray("Say yes!") << 4 << 3
                    << QByteArray("no") << QByteArray("Say no!");
    QTest::newRow("2") << QByteArray("rock and roll") << 5 << 3
                    << QByteArray("&") << QByteArray("rock & roll");
    QTest::newRow("3") << QByteArray("foo") << 3 << 0 << QByteArray("bar")
                    << QByteArray("foobar");
    QTest::newRow("4") << QByteArray() << 0 << 0 << QByteArray() << QByteArray();
    // index out of range
    QTest::newRow("5") << QByteArray() << 3 << 0 << QByteArray("hi")
                    << QByteArray("   hi");
}

void tst_QByteArray::replace()
{
    QFETCH(QByteArray, src);
    QFETCH(int, pos);
    QFETCH(int, len);
    QFETCH(QByteArray, after);
    QFETCH(QByteArray, expected);

    QByteArray str1 = src;
    QByteArray str2 = src;

    QCOMPARE(str1.replace(pos, len, after).constData(), expected.constData());
    QCOMPARE(str2.replace(pos, len, after.data()), expected);
}

void tst_QByteArray::indexOf_data()
{
    QTest::addColumn<QByteArray>("haystack");
    QTest::addColumn<QByteArray>("needle");
    QTest::addColumn<int>("startpos");
    QTest::addColumn<int>("expected");

    QTest::newRow( "1" ) << QByteArray("abc") << QByteArray("a") << 0 << 0;
    QTest::newRow( "2" ) << QByteArray("abc") << QByteArray("A") << 0 << -1;
    QTest::newRow( "3" ) << QByteArray("abc") << QByteArray("a") << 1 << -1;
    QTest::newRow( "4" ) << QByteArray("abc") << QByteArray("A") << 1 << -1;
    QTest::newRow( "5" ) << QByteArray("abc") << QByteArray("b") << 0 << 1;
    QTest::newRow( "6" ) << QByteArray("abc") << QByteArray("B") << 0 << -1;
    QTest::newRow( "7" ) << QByteArray("abc") << QByteArray("b") << 1 << 1;
    QTest::newRow( "8" ) << QByteArray("abc") << QByteArray("B") << 1 << -1;
    QTest::newRow( "9" ) << QByteArray("abc") << QByteArray("b") << 2 << -1;
    QTest::newRow( "10" ) << QByteArray("abc") << QByteArray("c") << 0 << 2;
    QTest::newRow( "11" ) << QByteArray("abc") << QByteArray("C") << 0 << -1;
    QTest::newRow( "12" ) << QByteArray("abc") << QByteArray("c") << 1 << 2;
    QTest::newRow( "13" ) << QByteArray("abc") << QByteArray("C") << 1 << -1;
    QTest::newRow( "14" ) << QByteArray("abc") << QByteArray("c") << 2 << 2;
    QTest::newRow( "15" ) << QByteArray("aBc") << QByteArray("bc") << 0 << -1;
    QTest::newRow( "16" ) << QByteArray("aBc") << QByteArray("Bc") << 0 << 1;
    QTest::newRow( "17" ) << QByteArray("aBc") << QByteArray("bC") << 0 << -1;
    QTest::newRow( "18" ) << QByteArray("aBc") << QByteArray("BC") << 0 << -1;

    QTest::newRow( "empty" ) << QByteArray("") << QByteArray("x") << 0 << -1;
    QTest::newRow( "null" ) << QByteArray() << QByteArray("x") << 0 << -1;
    QTest::newRow( "null-in-null") << QByteArray() << QByteArray() << 0 << 0;
    QTest::newRow( "empty-in-null") << QByteArray() << QByteArray("") << 0 << 0;
    QTest::newRow( "null-in-empty") << QByteArray("") << QByteArray() << 0 << 0;
    QTest::newRow( "empty-in-empty") << QByteArray("") << QByteArray("") << 0 << 0;
}

void tst_QByteArray::indexOf()
{
    QFETCH( QByteArray, haystack );
    QFETCH( QByteArray, needle );
    QFETCH( int, startpos );
    QFETCH( int, expected );

    QCOMPARE( haystack.indexOf(needle, startpos), expected );
    QCOMPARE( haystack.indexOf(needle.data(), startpos), expected );
    if (needle.size() == 1)
	QCOMPARE( haystack.indexOf(needle.at(0), startpos), expected );

    if (startpos == 0) {
	QCOMPARE( haystack.indexOf(needle), expected );
	QCOMPARE( haystack.indexOf(needle.data()), expected );
	if (needle.size() == 1)
	    QCOMPARE( haystack.indexOf(needle.at(0)), expected );
    }
}

void tst_QByteArray::lastIndexOf_data()
{
    QTest::addColumn<QByteArray>("haystack");
    QTest::addColumn<QByteArray>("needle");
    QTest::addColumn<int>("startpos");
    QTest::addColumn<int>("expected");

    QTest::newRow( "1" ) << QByteArray("abc") << QByteArray("a") << 0 << 0;
    QTest::newRow( "2" ) << QByteArray("abc") << QByteArray("A") << 0 << -1;
    QTest::newRow( "3" ) << QByteArray("abc") << QByteArray("a") << 1 << 0;
    QTest::newRow( "4" ) << QByteArray("abc") << QByteArray("A") << 1 << -1;
    QTest::newRow( "5" ) << QByteArray("abc") << QByteArray("a") << -1 << 0;
    QTest::newRow( "6" ) << QByteArray("abc") << QByteArray("b") << 0 << -1;
    QTest::newRow( "7" ) << QByteArray("abc") << QByteArray("B") << 0 << -1;
    QTest::newRow( "8" ) << QByteArray("abc") << QByteArray("b") << 1 << 1;
    QTest::newRow( "9" ) << QByteArray("abc") << QByteArray("B") << 1 << -1;
    QTest::newRow( "10" ) << QByteArray("abc") << QByteArray("b") << 2 << 1;
    QTest::newRow( "11" ) << QByteArray("abc") << QByteArray("b") << -1 << 1;
    QTest::newRow( "12" ) << QByteArray("abc") << QByteArray("c") << 0 << -1;
    QTest::newRow( "13" ) << QByteArray("abc") << QByteArray("C") << 0 << -1;
    QTest::newRow( "14" ) << QByteArray("abc") << QByteArray("c") << 1 << -1;
    QTest::newRow( "15" ) << QByteArray("abc") << QByteArray("C") << 1 << -1;
    QTest::newRow( "16" ) << QByteArray("abc") << QByteArray("c") << 2 << 2;
    QTest::newRow( "17" ) << QByteArray("abc") << QByteArray("c") << -1 << 2;
    QTest::newRow( "18" ) << QByteArray("aBc") << QByteArray("bc") << 0 << -1;
    QTest::newRow( "19" ) << QByteArray("aBc") << QByteArray("Bc") << 0 << -1;
    QTest::newRow( "20" ) << QByteArray("aBc") << QByteArray("Bc") << 2 << 1;
    QTest::newRow( "21" ) << QByteArray("aBc") << QByteArray("Bc") << 1 << 1;
    QTest::newRow( "22" ) << QByteArray("aBc") << QByteArray("Bc") << -1 << 1;
    QTest::newRow( "23" ) << QByteArray("aBc") << QByteArray("bC") << 0 << -1;
    QTest::newRow( "23" ) << QByteArray("aBc") << QByteArray("BC") << 0 << -1;

    QTest::newRow( "empty" ) << QByteArray("") << QByteArray("x") << -1 << -1;
    QTest::newRow( "null" ) << QByteArray() << QByteArray("x") << -1 << -1;
    QTest::newRow( "null-in-null") << QByteArray() << QByteArray() << -1 << 0;
    QTest::newRow( "empty-in-null") << QByteArray() << QByteArray("") << -1 << 0;
    QTest::newRow( "null-in-empty") << QByteArray("") << QByteArray() << -1 << 0;
    QTest::newRow( "empty-in-empty") << QByteArray("") << QByteArray("") << -1 << 0;
}

void tst_QByteArray::lastIndexOf()
{
    QFETCH( QByteArray, haystack );
    QFETCH( QByteArray, needle );
    QFETCH( int, startpos );
    QFETCH( int, expected );

    QCOMPARE( haystack.lastIndexOf(needle, startpos), expected );
    QCOMPARE( haystack.lastIndexOf(needle.data(), startpos), expected );
    if (needle.size() == 1)
	QCOMPARE( haystack.lastIndexOf(needle.at(0), startpos), expected );

    if (startpos == -1) {
	QCOMPARE( haystack.lastIndexOf(needle), expected );
	QCOMPARE( haystack.lastIndexOf(needle.data()), expected );
	if (needle.size() == 1)
	    QCOMPARE( haystack.lastIndexOf(needle.at(0)), expected );
    }
}

void tst_QByteArray::number()
{
    QCOMPARE(QString(QByteArray::number((quint64) 0)),
	    QString(QByteArray("0")));
    QCOMPARE(QString(QByteArray::number(Q_UINT64_C(0xFFFFFFFFFFFFFFFF))),
	    QString(QByteArray("18446744073709551615")));
    QCOMPARE(QString(QByteArray::number(Q_INT64_C(0xFFFFFFFFFFFFFFFF))),
	    QString(QByteArray("-1")));
    QCOMPARE(QString(QByteArray::number(qint64(0))),
	    QString(QByteArray("0")));
    QCOMPARE(QString(QByteArray::number(Q_INT64_C(0x7FFFFFFFFFFFFFFF))),
	    QString(QByteArray("9223372036854775807")));
    QCOMPARE(QString(QByteArray::number(Q_INT64_C(0x8000000000000000))),
	    QString(QByteArray("-9223372036854775808")));
}

void tst_QByteArray::toInt_data()
{
    QTest::addColumn<QByteArray>("string");
    QTest::addColumn<int>("base");
    QTest::addColumn<int>("expectednumber");
    QTest::addColumn<bool>("expectedok");

    QTest::newRow("base 10") << QByteArray("100") << 10 << int(100) << true;
    QTest::newRow("base 16") << QByteArray("100") << 16 << int(256) << true;
    QTest::newRow("base 16") << QByteArray("0400") << 16 << int(1024) << true;
    QTest::newRow("base 2") << QByteArray("1111") << 2 << int(15) << true;
    QTest::newRow("base 8") << QByteArray("100") << 8 << int(64) << true;
    QTest::newRow("base 0") << QByteArray("0x10") << 0 << int(16) << true;
    QTest::newRow("base 0") << QByteArray("10") << 0 << int(10) << true;
    QTest::newRow("base 0") << QByteArray("010") << 0 << int(8) << true;
    QTest::newRow("empty") << QByteArray() << 0 << int(0) << false;
}

void tst_QByteArray::toInt()
{
   QFETCH( QByteArray, string );
   QFETCH( int, base );
   QFETCH( int, expectednumber );
   QFETCH( bool, expectedok );

    bool ok;
    int number = string.toInt(&ok, base);

    QCOMPARE( ok, expectedok );
    QCOMPARE( number, expectednumber );
}

Q_DECLARE_METATYPE(qulonglong);
void tst_QByteArray::toULong_data()
{
    QTest::addColumn<QByteArray>("str");
    QTest::addColumn<int>("base");
    QTest::addColumn<ulong>("result");
    QTest::addColumn<bool>("ok");

    ulong LongMaxPlusOne = (ulong)LONG_MAX + 1;
    QTest::newRow("LONG_MAX+1") << QString::number(LongMaxPlusOne).toLatin1() << 10 << LongMaxPlusOne << true;
    QTest::newRow("default") << QByteArray() << 10 << 0UL << FALSE;
    QTest::newRow("empty") << QByteArray("") << 10 << 0UL << FALSE;
    QTest::newRow("ulong1") << QByteArray("3234567890") << 10 << 3234567890UL << true;
    QTest::newRow("ulong2") << QByteArray("fFFfFfFf") << 16 << 0xFFFFFFFFUL << true;
}

void tst_QByteArray::toULong()
{
    QFETCH(QByteArray, str);
    QFETCH(int, base);
    QFETCH(ulong, result);
    QFETCH(bool, ok);

    bool b;
    QCOMPARE(str.toULong(0, base), result);
    QCOMPARE(str.toULong(&b, base), result);
    QCOMPARE(b, ok);
}

void tst_QByteArray::toULongLong_data()
{
    QTest::addColumn<QByteArray>("str");
    QTest::addColumn<int>("base");
    QTest::addColumn<qulonglong>("result");
    QTest::addColumn<bool>("ok");

    QTest::newRow("default") << QByteArray() << 10 << (qulonglong)0 << FALSE;
    QTest::newRow("out of base bound") << QByteArray("c") << 10 << (qulonglong)0 << FALSE;

}

void tst_QByteArray::toULongLong()
{
    QFETCH(QByteArray, str);
    QFETCH(int, base);
    QFETCH(qulonglong, result);
    QFETCH(bool, ok);

    bool b;
    QCOMPARE(str.toULongLong(0, base), result);
    QCOMPARE(str.toULongLong(&b, base), result);
    QCOMPARE(b, ok);
}

// global function defined in qbytearray.cpp
void tst_QByteArray::qAllocMore()
{
    const int N = 15;
    const int t[N] = {
        INT_MIN, INT_MIN + 1, -1234567, -66000, -1025,
        -3, -1, 0, +1, +3, +1025, +66000, +1234567, INT_MAX - 1, INT_MAX
    };

    // make sure qAllocMore() doesn't loop infinitely on any input
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            ::qAllocMore(t[i], t[j]);
        }
    }
}

void tst_QByteArray::resizeAfterFromRawData()
{
    QByteArray buffer("hello world");

    QByteArray array = QByteArray::fromRawData(buffer.constData(), buffer.size());
    QVERIFY(array.constData() == buffer.constData());
    array.resize(5);
    QVERIFY(array.constData() == buffer.constData());
}

QTEST_APPLESS_MAIN(tst_QByteArray)
#include "tst_qbytearray.moc"
