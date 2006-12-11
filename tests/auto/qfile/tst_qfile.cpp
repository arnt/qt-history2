/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <QAbstractFileEngine>
#include <QFSFileEngine>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostInfo>
#include <QProcess>
#ifndef Q_OS_WIN
# include <sys/types.h>
# include <unistd.h>
#endif
#ifdef Q_OS_MAC
# include <sys/mount.h>
#elif defined(Q_OS_LINUX)
# include <sys/vfs.h>
#elif defined(Q_OS_FREEBSD)
# include <sys/param.h>
# include <sys/mount.h>
#elif defined(Q_OS_IRIX)
# include <sys/statfs.h>
#endif

#include <stdio.h>

#if QT_VERSION < 0x040200
#define symLinkTarget readLink
#endif

Q_DECLARE_METATYPE(QFile::FileError)

//TESTED_CLASS=
//TESTED_FILES=corelib/io/qfile.h corelib/io/qfile.cpp

class tst_QFile : public QObject
{
    Q_OBJECT

public:
    tst_QFile();
    virtual ~tst_QFile();


public slots:
    void init();
    void cleanup();
private slots:
    void initTestCase();
    void cleanupTestCase();
    void exists();
    void open_data();
    void open();
    void size_data();
    void size();
    void seek();
    void setSize();
    void setSizeSeek();
    void atEnd();
    void readLine();
    void readLine2();
    void readAllStdin();
    void readLineStdin();
    void text();
    void missingEndOfLine();
    void readBlock();
    void getch();
    void ungetChar();
    void createFile();
    void append();
    void permissions_data();
    void permissions();
    void setPermissions();
    void copy();
    void copyShouldntOverwrite();
    void link();
    void linkToDir();
    void absolutePathLinkToRelativePath();
    void readBrokenLink();
    void readTextFile_data();
    void readTextFile();
    void readTextFile2();
    void writeTextFile_data();
    void writeTextFile();
    /* void largeFileSupport(); */
    void tailFile();
    void flush();
    void bufferedRead();
    void isSequential();
    void encodeName();
    void truncate();
    void seekToPos();
    void FILEReadWrite();
    void i18nFileName_data();
    void i18nFileName();
    void longFileName_data();
    void longFileName();
    void fileEngineHandler();
    void useQFileInAFileHandler();
    void getCharFF();
    void remove_and_exists();
    void removeOpenFile();
    void fullDisk();
    void writeLargeDataBlock_data();
    void writeLargeDataBlock();
    void readFromWriteOnlyFile();
    void writeToReadOnlyFile();
    void virtualFile();
    void textFile();
    void readPastEnd();
    void rename_data();
    void rename();
    void appendAndRead();
    void miscWithUncPathAsCurrentDir();

public:
// disabled this test for the moment... it hangs
    void invalidFile_data();
    void invalidFile();
};

tst_QFile::tst_QFile()
{
}

tst_QFile::~tst_QFile()
{

}

void tst_QFile::init()
{
// TODO: Add initialization code here.
// This will be executed immediately before each test is run.
}

void tst_QFile::cleanup()
{
// TODO: Add cleanup code here.
// This will be executed immediately after each test is run.
}

void tst_QFile::initTestCase()
{
    QFile::remove("noreadfile");

    // create a file and make it read-only
    QFile file("readonlyfile");
    file.open(QFile::WriteOnly);
    file.write("a", 1);
    file.close();
    file.setPermissions(QFile::ReadOwner);

    // create another file and make it not readable
    file.setFileName("noreadfile");
    file.open(QFile::WriteOnly);
    file.write("b", 1);
    file.close();
    file.setPermissions(0);
}

void tst_QFile::cleanupTestCase()
{
    // clean up the files we created
    QFile::remove("readonlyfile");
    QFile::remove("noreadfile");
    QFile::remove("myLink.lnk");
    QFile::remove("appendme.txt");
    QFile::remove("createme.txt");
    QFile::remove("file.txt");
    QFile::remove("genfile.txt");
    QFile::remove("seekToPos.txt");
    QFile::remove("setsizeseek.txt");
    QFile::remove("stdfile.txt");
    QFile::remove("textfile.txt");
    QFile::remove("truncate.txt");
    QFile::remove("winfile.txt");
    QFile::remove("writeonlyfile");
    QFile::remove("largeblockfile.txt");
    QFile::remove("tst_qfile_copy.cpp");
}

//------------------------------------------
// The 'testfile' is currently just a
// testfile. The path of this file, the
// attributes and the contents itself
// will be changed as far as we have a
// proper way to handle files in the
// testing enviroment.
//------------------------------------------

void tst_QFile::exists()
{
    QFile f( "testfile.txt" );
    QCOMPARE( f.exists(), (bool)TRUE );

    QFile file("nobodyhassuchafile");
    file.remove();
    QVERIFY(!file.exists());

    QFile file2("nobodyhassuchafile");
    QVERIFY(file2.open(QIODevice::WriteOnly));
    file2.close();

    QVERIFY(file.exists());

    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    QVERIFY(file.exists());

    file.remove();
    QVERIFY(!file.exists());

#ifdef Q_OS_WIN
    QFile unc("//Gennan/testshare/readme.txt");
    QVERIFY(unc.exists());
#endif
}

void tst_QFile::open_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("mode");
    QTest::addColumn<bool>("ok");
    QTest::addColumn<QFile::FileError>("status");

#ifdef Q_OS_MAC
    static const QString denied("Operation not permitted");
#else
    static const QString denied("Permission denied");
#endif
    QTest::newRow( "exist_readOnly"  )
        << QString("testfile.txt") << int(QIODevice::ReadOnly)
        << (bool)TRUE << QFile::NoError;

    QTest::newRow( "exist_writeOnly" )
        << QString("testfile.txt")
        << int(QIODevice::WriteOnly)
        << (bool)FALSE
        << QFile::OpenError;

    QTest::newRow( "exist_append"    )
        << QString("testfile.txt") << int(QIODevice::Append)
        << (bool)FALSE << QFile::OpenError;

    QTest::newRow( "nonexist_readOnly"  )
        << QString("nonExist.txt") << int(QIODevice::ReadOnly)
        << (bool)FALSE << QFile::OpenError;

    QTest::newRow("emptyfile")
        << QString("")
        << int(QIODevice::ReadOnly)
        << (bool)FALSE
        << QFile::OpenError;

    QTest::newRow("nullfile") << QString() << int(QIODevice::ReadOnly) << (bool)FALSE
        << QFile::OpenError;

    QTest::newRow("two-dots") << QString("two.dots.file") << int(QIODevice::ReadOnly) << (bool)TRUE
        << QFile::NoError;

    QTest::newRow("readonlyfile") << QString("readonlyfile") << int(QIODevice::WriteOnly)
                                  << (bool)FALSE << QFile::OpenError;
    QTest::newRow("noreadfile") << QString("noreadfile") << int(QIODevice::ReadOnly)
                                << (bool)FALSE << QFile::OpenError;
#ifdef Q_OS_WIN
    QTest::newRow("//./PhysicalDrive0") << QString("//./PhysicalDrive0") << int(QIODevice::ReadOnly)
                                        << (bool)TRUE << QFile::NoError;
    QTest::newRow("uncFile") << "//gennan/testsharewritable/test.pri" << int(QIODevice::ReadOnly)
                             << true << QFile::NoError;
#endif
}

void tst_QFile::open()
{
    QFETCH( QString, filename );
    QFETCH( int, mode );

    QFile f( filename );

    QFETCH( bool, ok );

#if defined(Q_OS_WIN32)
    QEXPECT_FAIL("noreadfile", "Windows does not currently support non-readable files.", Abort);
#endif
    if (filename.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, "QFSFileEngine::open: No file name specified");

    QCOMPARE(f.open( QIODevice::OpenMode(mode) ), ok);

    QTEST( f.error(), "status" );
}

void tst_QFile::size_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("size");

    QTest::newRow( "exist01" ) << QString("testfile.txt") << 245;
    QTest::newRow( "nonexist01" ) << QString("foo.txt") << 0;
#ifdef Q_OS_WIN
    // Only test UNC on Windows./
    QTest::newRow("unc") << QString("//gennan/testsharewritable/test.pri") << 34;
#endif
}

void tst_QFile::size()
{
    QFETCH( QString, filename );
    QFile f( filename );
    QTEST( (int)f.size(), "size" );
    if (f.open(QFile::ReadOnly))
        QTEST( (int)f.size(), "size" );
}

void tst_QFile::seek()
{
    QFile::remove("newfile.txt");
    QFile file("newfile.txt");
    file.open(QIODevice::WriteOnly);
    QCOMPARE(file.size(), qint64(0));
    QCOMPARE(file.pos(), qint64(0));
    QVERIFY(file.seek(10));
    QCOMPARE(file.pos(), qint64(10));
    QCOMPARE(file.size(), qint64(0));
    QFile::remove("newfile.txt");
}

void tst_QFile::setSize()
{
    DEPENDS_ON( "size" );

    if ( QFile::exists( "createme.txt" ) )
        QFile::remove( "createme.txt" );
    QVERIFY( !QFile::exists( "createme.txt" ) );

    QFile f("createme.txt");
    QVERIFY(f.open(QIODevice::Truncate | QIODevice::ReadWrite));
    f.putChar('a');

    f.seek(0);
    char c = '\0';
    f.getChar(&c);
    QCOMPARE(c, 'a');

    QCOMPARE(f.size(), (qlonglong)1);
    QVERIFY(f.resize(99));
    QCOMPARE(f.size(), (qlonglong)99);

    f.seek(0);
    c = '\0';
    f.getChar(&c);
    QCOMPARE(c, 'a');

    QVERIFY(f.resize(1));
    QCOMPARE(f.size(), (qlonglong)1);

    f.seek(0);
    c = '\0';
    f.getChar(&c);
    QCOMPARE(c, 'a');

    f.close();

    QCOMPARE(f.size(), (qlonglong)1);
    QVERIFY(f.resize(100));
    QCOMPARE(f.size(), (qlonglong)100);
    QVERIFY(f.resize(50));
    QCOMPARE(f.size(), (qlonglong)50);
}

void tst_QFile::setSizeSeek()
{
    QFile::remove("setsizeseek.txt");
    QFile f("setsizeseek.txt");
    QVERIFY(f.open(QFile::WriteOnly));
    f.write("ABCD");

    QCOMPARE(f.pos(), qint64(4));
    f.resize(2);
    QCOMPARE(f.pos(), qint64(2));
    f.resize(4);
    QCOMPARE(f.pos(), qint64(2));
    f.resize(0);
    QCOMPARE(f.pos(), qint64(0));
    f.resize(4);
    QCOMPARE(f.pos(), qint64(0));

    f.seek(3);
    QCOMPARE(f.pos(), qint64(3));
    f.resize(2);
    QCOMPARE(f.pos(), qint64(2));
}

void tst_QFile::atEnd()
{
    QFile f( "testfile.txt" );
    f.open( QIODevice::ReadOnly );

    int size = f.size();
    f.seek( size );

    bool end = f.atEnd();
    f.close();
    QCOMPARE( end, (bool)TRUE );
}

void tst_QFile::readLine()
{
    QFile f( "testfile.txt" );
    QVERIFY(f.open( QIODevice::ReadOnly ));

    int i = 0;
    char p[128];
    int foo;
    while ( (foo=f.readLine( p, 128 )) > 0 ) {
        ++i;
        if ( i == 5 ) {
            QCOMPARE( p[0], 'T' );
            QCOMPARE( p[3], 's' );
            QCOMPARE( p[11], 'i' );
        }
    }
    f.close();
    QCOMPARE( i, 6 );
}

void tst_QFile::readLine2()
{
    QFile f( "testfile.txt" );
    f.open( QIODevice::ReadOnly );

    char p[128];
    QCOMPARE(f.readLine(p, 60), qlonglong(59));
    QCOMPARE(f.readLine(p, 60), qlonglong(59));
    memset(p, '@', sizeof(p));
    QCOMPARE(f.readLine(p, 60), qlonglong(59));

    QCOMPARE(p[57], '-');
    QCOMPARE(p[58], '\n');
    QCOMPARE(p[59], '\0');
    QCOMPARE(p[60], '@');
}

void tst_QFile::readAllStdin()
{
    QByteArray lotsOfData(1024, '@'); // 10 megs

    QProcess process;
    process.start("stdinprocess/stdinprocess all");
    for (int i = 0; i < 5; ++i) {
        QTest::qWait(1000);
        process.write(lotsOfData);
        while (process.bytesToWrite() > 0) {
            QVERIFY(process.waitForBytesWritten());
        }
    }

    process.closeWriteChannel();
    process.waitForFinished();
    QCOMPARE(process.readAll().size(), lotsOfData.size() * 5);
}

void tst_QFile::readLineStdin()
{
    QByteArray lotsOfData(1024, '@'); // 10 megs
    for (int i = 0; i < lotsOfData.size(); ++i) {
        if ((i % 32) == 31)
            lotsOfData[i] = '\n';
        else
            lotsOfData[i] = char('0' + i % 32);
    }

    QProcess process;
    process.start("stdinprocess/stdinprocess line", QIODevice::Text | QIODevice::ReadWrite);
    for (int i = 0; i < 5; ++i) {
        QTest::qWait(1000);
        process.write(lotsOfData);
        while (process.bytesToWrite() > 0) {
            QVERIFY(process.waitForBytesWritten());
        }
    }

    process.closeWriteChannel();
    QVERIFY(process.waitForFinished(5000));

    QByteArray array = process.readAll();
    QCOMPARE(array.size(), lotsOfData.size() * 5);
    for (int i = 0; i < array.size(); ++i) {
        if ((i % 32) == 31)
            QCOMPARE(char(array[i]), '\n');
        else
            QCOMPARE(char(array[i]), char('0' + i % 32));
    }
}

void tst_QFile::text()
{
    // dosfile.txt is a binary CRLF file
    QFile file("dosfile.txt");
    QVERIFY(file.open(QFile::Text | QFile::ReadOnly));
    QCOMPARE(file.readLine(),
            QByteArray("/dev/system/root     /                    reiserfs   acl,user_xattr        1 1\n"));
    QCOMPARE(file.readLine(),
            QByteArray("/dev/sda1            /boot                ext3       acl,user_xattr        1 2\n"));
    file.ungetChar('\n');
    file.ungetChar('2');
    QCOMPARE(file.readLine().constData(), QByteArray("2\n").constData());
}

void tst_QFile::missingEndOfLine()
{
    QFile file("noendofline.txt");
    QVERIFY(file.open(QFile::ReadOnly));

    int nlines = 0;
    while (!file.atEnd()) {
        ++nlines;
        file.readLine();
    }

    QCOMPARE(nlines, 3);
}

void tst_QFile::readBlock()
{
    QFile f( "testfile.txt" );
    f.open( QIODevice::ReadOnly );

    int length = 0;
    char p[256];
    length = f.read( p, 256 );
    f.close();
    QCOMPARE( length, 245 );
    QCOMPARE( p[59], 'D' );
    QCOMPARE( p[178], 'T' );
    QCOMPARE( p[199], 'l' );
}

void tst_QFile::getch()
{
    QFile f( "testfile.txt" );
    f.open( QIODevice::ReadOnly );

    char c;
    int i = 0;
    while (f.getChar(&c)) {
        QCOMPARE(f.pos(), qint64(i + 1));
        if ( i == 59 )
            QCOMPARE( c, 'D' );
        ++i;
    }
    f.close();
    QCOMPARE( i, 245 );
}

void tst_QFile::ungetChar()
{
    QFile f("testfile.txt");
    QVERIFY(f.open(QIODevice::ReadOnly));

    QByteArray array = f.readLine();
    QCOMPARE(array.constData(), "----------------------------------------------------------\n");
    f.ungetChar('\n');

    array = f.readLine();
    QCOMPARE(array.constData(), "\n");

    f.ungetChar('\n');
    f.ungetChar('-');
    f.ungetChar('-');

    array = f.readLine();
    QCOMPARE(array.constData(), "--\n");

    QFile::remove("genfile.txt");
    QFile out("genfile.txt");
    QVERIFY(out.open(QIODevice::ReadWrite));
    out.write("123");
    out.seek(0);
    QCOMPARE(out.readAll().constData(), "123");
    out.ungetChar('3');
    out.write("4");
    out.seek(0);
    QCOMPARE(out.readAll().constData(), "124");
    out.ungetChar('4');
    out.ungetChar('2');
    out.ungetChar('1');
    char buf[3];
    QCOMPARE(out.read(buf, sizeof(buf)), qint64(3));
    QCOMPARE(buf[0], '1');
    QCOMPARE(buf[1], '2');
    QCOMPARE(buf[2], '4');
}

void tst_QFile::invalidFile_data()
{
    QTest::addColumn<QString>("fileName");
#ifndef Q_WS_WIN
    QTest::newRow( "x11" ) << QString( "qwe//" );
#else
    QTest::newRow( "colon1" ) << QString( "fail:invalid" );
    QTest::newRow( "colon2" ) << QString( "f:ail:invalid" );
    QTest::newRow( "colon3" ) << QString( ":failinvalid" );
    QTest::newRow( "forwardslash" ) << QString( "fail/invalid" );
    QTest::newRow( "asterisk" ) << QString( "fail*invalid" );
    QTest::newRow( "questionmark" ) << QString( "fail?invalid" );
    QTest::newRow( "quote" ) << QString( "fail\"invalid" );
    QTest::newRow( "lt" ) << QString( "fail<invalid" );
    QTest::newRow( "gt" ) << QString( "fail>invalid" );
    QTest::newRow( "pipe" ) << QString( "fail|invalid" );
#endif
}

void tst_QFile::invalidFile()
{
    QFETCH( QString, fileName );
    QFile f( fileName );
    QVERIFY( !f.open( QIODevice::ReadWrite ) );
}

void tst_QFile::createFile()
{
    if ( QFile::exists( "createme.txt" ) )
        QFile::remove( "createme.txt" );
    QVERIFY( !QFile::exists( "createme.txt" ) );

    QFile f( "createme.txt" );
    QVERIFY( f.open( QIODevice::WriteOnly ) );
    f.close();
    QVERIFY( QFile::exists( "createme.txt" ) );
}

void tst_QFile::append()
{
    const QString name("appendme.txt");
    if (QFile::exists(name))
        QFile::remove(name);
    QVERIFY(!QFile::exists(name));

    QFile f(name);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.putChar('a');
    f.close();

    QVERIFY(f.open(QIODevice::Append));
    QVERIFY(f.pos() == 1);
    f.putChar('a');
    f.close();
    QCOMPARE(int(f.size()), 2);
}

void tst_QFile::permissions_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<uint>("perms");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QCoreApplication::instance()->applicationFilePath() << uint(QFile::ExeUser) << true;
    QTest::newRow("data1") << "tst_qfile.cpp" << uint(QFile::ReadUser) << true;
//    QTest::newRow("data2") << "tst_qfile.cpp" << int(QFile::WriteUser) << false;
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << uint(QFile::ReadUser) << true;
    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1" << uint(QFile::WriteUser) << false;
    QTest::newRow("resource3") << ":/tst_qfileinfo/resources/file1.ext1" << uint(QFile::ExeUser) << false;
}

void tst_QFile::permissions()
{
    QFETCH(QString, file);
    QFETCH(uint, perms);
    QFETCH(bool, expected);
    QFile f(file);
    QCOMPARE(((f.permissions() & perms) == QFile::Permissions(perms)), expected);
}

void tst_QFile::setPermissions()
{
    DEPENDS_ON( "permissions" ); //if that doesn't work...

    if ( QFile::exists( "createme.txt" ) )
        QFile::remove( "createme.txt" );
    QVERIFY( !QFile::exists( "createme.txt" ) );

    QFile f("createme.txt");
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.putChar('a');
    f.close();

    QFile::Permissions perms(QFile::WriteUser | QFile::ReadUser);
    QVERIFY(f.setPermissions(perms));
    QVERIFY((f.permissions() & perms) == perms);

}

void tst_QFile::copy()
{
    QFile::setPermissions("tst_qfile_copy.cpp", QFile::WriteUser);
    QFile::remove("tst_qfile_copy.cpp");
    QVERIFY(QFile::copy("tst_qfile.cpp", "tst_qfile_copy.cpp"));
    QFile in1("tst_qfile.cpp"), in2("tst_qfile_copy.cpp");
    QVERIFY(in1.open(QFile::ReadOnly));
    QVERIFY(in2.open(QFile::ReadOnly));
    QByteArray data1 = in1.readAll(), data2 = in2.readAll();
    QCOMPARE(data1, data2);
    QFile::remove( "main_copy.cpp" );
}

void tst_QFile::copyShouldntOverwrite()
{
    // Copy should not overwrite existing files.
    QFile::remove("tst_qfile.cpy");
    QFile file("tst_qfile.cpp");
    QVERIFY(file.copy("tst_qfile.cpy"));
    QVERIFY(QFile::setPermissions("tst_qfile.cpy", QFile::WriteOther));
    QVERIFY(!file.copy("tst_qfile.cpy"));
    QFile::remove("tst_qfile.cpy");
}

void tst_QFile::link()
{
    QFile::remove("myLink.lnk");
    QFileInfo info1("tst_qfile.cpp");
    QVERIFY(QFile::link("tst_qfile.cpp", "myLink.lnk"));
    QFileInfo info2("myLink.lnk");
    QVERIFY(info2.isSymLink());
#ifdef Q_OS_WIN // on windows links are always absolute
    QCOMPARE(info2.symLinkTarget(), info1.absoluteFilePath());
#else
#if QT_VERSION < 0x040101
    QCOMPARE(info2.symLinkTarget(), info1.filePath());
#else
    QCOMPARE(info2.symLinkTarget(), info1.absoluteFilePath());
#endif
#endif
    QVERIFY(QFile::remove(info2.absoluteFilePath()));
}

void tst_QFile::linkToDir()
{
    QFile::remove("myLinkToDir.lnk");
    QDir dir;
    dir.mkdir("myDir");
    QFileInfo info1("myDir");
    QVERIFY(QFile::link("myDir", "myLinkToDir.lnk"));
    QFileInfo info2("myLinkToDir.lnk");
    QVERIFY(info2.isSymLink());
#ifdef Q_OS_WIN // on windows links are alway absolute
    QCOMPARE(info2.symLinkTarget(), info1.absoluteFilePath());
#else
#if QT_VERSION < 0x040101
    QEXPECT_FAIL("", "QFileInfo::symLinkTarget() only partially resolves the link target", Continue);
    QCOMPARE(info2.symLinkTarget(), info1.filePath());
#else
    QCOMPARE(info2.symLinkTarget(), info1.absoluteFilePath());
#endif
#endif
    QVERIFY(QFile::remove(info2.absoluteFilePath()));
    dir.rmdir("myDir");
}

void tst_QFile::absolutePathLinkToRelativePath()
{
    QFile::remove("myDir/test.txt");
    QFile::remove("myDir/myLink.lnk");
    QDir dir;
    dir.mkdir("myDir");
    QFile("myDir/test.txt").open(QFile::WriteOnly);

#ifdef Q_OS_WIN
    QVERIFY(QFile::link("test.txt", "myDir/myLink.lnk"));
#else
    QVERIFY(QFile::link("myDir/test.txt", "myDir/myLink.lnk"));
#endif

    QEXPECT_FAIL("", "Symlinking using relative paths is currently different on Windows and Unix", Continue);
    QCOMPARE(QFileInfo(QFile(QFileInfo("myDir/myLink.lnk").absoluteFilePath()).symLinkTarget()).absoluteFilePath(),
             QFileInfo("myDir/test.txt").absoluteFilePath());

    QFile::remove("myDir/test.txt");
    QFile::remove("myDir/myLink.lnk");
    dir.rmdir("myDir");
}

void tst_QFile::readBrokenLink()
{
    QFile::remove("myLink.lnk");
    QFileInfo info1("file1");
    QVERIFY(QFile::link("file1", "myLink.lnk"));
    QFileInfo info2("myLink.lnk");
    QVERIFY(info2.isSymLink());
#ifdef Q_OS_WIN // on windows links are alway absolute
    QCOMPARE(info2.symLinkTarget(), info1.absoluteFilePath());
#else
#if QT_VERSION < 0x040101
    QCOMPARE(info2.symLinkTarget(), info1.filePath());
#else
    QCOMPARE(info2.symLinkTarget(), info1.absoluteFilePath());
#endif
#endif
    QVERIFY(QFile::remove(info2.absoluteFilePath()));


#if QT_VERSION >= 0x040101
    QVERIFY(QFile::link("ole/..", "myLink.lnk"));
    QCOMPARE(QFileInfo("myLink.lnk").symLinkTarget(), QDir::currentPath());
#endif
}

void tst_QFile::readTextFile_data()
{
    QTest::addColumn<QByteArray>("in");
    QTest::addColumn<QByteArray>("out");

    QTest::newRow("empty") << QByteArray() << QByteArray();
    QTest::newRow("a") << QByteArray("a") << QByteArray("a");
    QTest::newRow("a\\rb") << QByteArray("a\rb") << QByteArray("ab");
    QTest::newRow("\\n") << QByteArray("\n") << QByteArray("\n");
    QTest::newRow("\\r\\n") << QByteArray("\r\n") << QByteArray("\n");
    QTest::newRow("\\r") << QByteArray("\r") << QByteArray();
    QTest::newRow("twolines") << QByteArray("Hello\r\nWorld\r\n") << QByteArray("Hello\nWorld\n");
    QTest::newRow("twolines no endline") << QByteArray("Hello\r\nWorld") << QByteArray("Hello\nWorld");
}

void tst_QFile::readTextFile()
{
    QFETCH(QByteArray, in);
    QFETCH(QByteArray, out);

    QFile winfile("winfile.txt");
    QVERIFY(winfile.open(QFile::WriteOnly | QFile::Truncate));
    winfile.write(in);
    winfile.close();

    QVERIFY(winfile.open(QFile::ReadOnly));
    QCOMPARE(winfile.readAll(), in);
    winfile.close();

    QVERIFY(winfile.open(QFile::ReadOnly | QFile::Text));
    QCOMPARE(winfile.readAll(), out);
}

void tst_QFile::readTextFile2()
{
    {
        QFile file("testlog.txt");
        QVERIFY(file.open(QIODevice::ReadOnly));
        file.read(4097);
    }

    {
        QFile file("testlog.txt");
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        file.read(4097);
    }
}

void tst_QFile::writeTextFile_data()
{
    QTest::addColumn<QByteArray>("in");

    QTest::newRow("empty") << QByteArray();
    QTest::newRow("a") << QByteArray("a");
    QTest::newRow("a\\rb") << QByteArray("a\rb");
    QTest::newRow("\\n") << QByteArray("\n");
    QTest::newRow("\\r\\n") << QByteArray("\r\n");
    QTest::newRow("\\r") << QByteArray("\r");
    QTest::newRow("twolines crlf") << QByteArray("Hello\r\nWorld\r\n");
    QTest::newRow("twolines crlf no endline") << QByteArray("Hello\r\nWorld");
    QTest::newRow("twolines lf") << QByteArray("Hello\nWorld\n");
    QTest::newRow("twolines lf no endline") << QByteArray("Hello\nWorld");
    QTest::newRow("mixed") << QByteArray("this\nis\r\na\nmixed\r\nfile\n");
}

void tst_QFile::writeTextFile()
{
    QFETCH(QByteArray, in);

    QFile file("textfile.txt");
    QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text));
    QByteArray out = in;
#ifdef Q_OS_WIN
    out.replace('\n', "\r\n");
#endif
    QCOMPARE(file.write(in), qlonglong(in.size()));
    file.close();

    file.open(QFile::ReadOnly);
    QCOMPARE(file.readAll(), out);
}

void tst_QFile::tailFile()
{
    QSKIP("File change notifications are so far unsupported.", SkipAll);

    QFile file("tail.txt");
    QVERIFY(file.open(QFile::WriteOnly | QFile::Append));

    QFile tailFile("tail.txt");
    QVERIFY(tailFile.open(QFile::ReadOnly));
    tailFile.seek(file.size());

    QSignalSpy readSignal(&tailFile, SIGNAL(readyRead()));

    file.write("", 1);

    QTestEventLoop::instance().enterLoop(5);

    QVERIFY(!QTestEventLoop::instance().timeout());
    QCOMPARE(readSignal.count(), 1);
}

void tst_QFile::flush()
{
    QFile::remove("stdfile.txt");

    FILE *stdFile = fopen("stdfile.txt", "w");
    QVERIFY(stdFile);
    QCOMPARE(int(fwrite("abc", 3, 1, stdFile)), 1);

    {
        QFile file;
        QVERIFY(file.open(stdFile, QFile::WriteOnly | QFile::Append));
        QCOMPARE(file.pos(), qlonglong(3));
        QCOMPARE(file.write("def", 3), qlonglong(3));
        QCOMPARE(file.pos(), qlonglong(6));
    }

    fclose(stdFile);

    {
        QFile file("stdfile.txt");
        QVERIFY(file.open(QFile::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray("abcdef"));
    }
}

void tst_QFile::bufferedRead()
{
    QFile::remove("stdfile.txt");

    QFile file("stdfile.txt");
    QVERIFY(file.open(QFile::WriteOnly));
    file.write("abcdef");
    file.close();

    FILE *stdFile = fopen("stdfile.txt", "r");
    QVERIFY(stdFile);
    char c;
    QCOMPARE(int(fread(&c, 1, 1, stdFile)), 1);
    QCOMPARE(c, 'a');
    QCOMPARE(int(ftell(stdFile)), 1);

    {
        QFile file;
        QVERIFY(file.open(stdFile, QFile::ReadOnly));
        QCOMPARE(file.pos(), qlonglong(1));
        QCOMPARE(file.read(&c, 1), qlonglong(1));
        QCOMPARE(c, 'b');
        QCOMPARE(file.pos(), qlonglong(2));
    }

#if QT_VERSION <= 0x040100
    QCOMPARE(int(ftell(stdFile)), 2);
#endif

    fclose(stdFile);
}

void tst_QFile::isSequential()
{
#if defined (Q_OS_WIN)
    QSKIP("Unix only test.", SkipAll);
#endif

    QFile zero("/dev/null");
    QVERIFY(zero.open(QFile::ReadOnly));
    QVERIFY(zero.isSequential());
}

void tst_QFile::encodeName()
{
    QCOMPARE(QFile::encodeName(QString::null), QByteArray());
}

void tst_QFile::truncate()
{
    for (int i = 0; i < 2; ++i) {
        QFile file("truncate.txt");
        QVERIFY(file.open(QFile::WriteOnly));
        file.write(QByteArray(200, '@'));
        file.close();

        QVERIFY(file.open((i ? QFile::WriteOnly : QFile::ReadWrite) | QFile::Truncate));
        file.write(QByteArray(100, '$'));
        file.close();

        QVERIFY(file.open(QFile::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray(100, '$'));
    }
}

void tst_QFile::seekToPos()
{
    {
        QFile file("seekToPos.txt");
        QVERIFY(file.open(QFile::WriteOnly));
        file.write("a\r\nb\r\nc\r\n");
        file.flush();
    }

    QFile file("seekToPos.txt");
    QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
    file.seek(1);
    char c;
    QVERIFY(file.getChar(&c));
    QCOMPARE(c, '\n');

    QCOMPARE(file.pos(), qint64(3));
    file.seek(file.pos());
    QCOMPARE(file.pos(), qint64(3));

    file.seek(1);
    file.seek(file.pos());
    QCOMPARE(file.pos(), qint64(1));

}


void tst_QFile::FILEReadWrite()
{
    // Tests modifing a file. First creates it then reads in 4 bytes and then overwrites these
    // 4 bytes with new values. At the end check to see the file contains the new values.

    QFile::remove("FILEReadWrite.txt");

    // create test file
    {
        QFile f("FILEReadWrite.txt");
        QVERIFY(f.open(QFile::WriteOnly));
        QDataStream ds(&f);
        qint8 c = 0;
        ds << c;
        c = 1;
        ds << c;
        c = 2;
        ds << c;
        c = 3;
        ds << c;
        c = 4;
        ds << c;
        c = 5;
        ds << c;
        c = 6;
        ds << c;
        c = 7;
        ds << c;
        c = 8;
        ds << c;
        c = 9;
        ds << c;
        c = 10;
        ds << c;
        c = 11;
        ds << c;
        f.close();
    }

    FILE *fp = fopen("FILEReadWrite.txt", "r+b");
    QVERIFY(fp);
    QFile file;
    QVERIFY(file.open(fp, QFile::ReadWrite));
    QDataStream sfile(&file) ;

    qint8 var1,var2,var3,var4;
    while (!sfile.atEnd())
    {
        qint64 base = file.pos();

        QCOMPARE(file.pos(), base + 0);
        sfile >> var1;
        QCOMPARE(file.pos(), base + 1);
        file.flush(); // flushing should not change the base
        QCOMPARE(file.pos(), base + 1);
        sfile >> var2;
        QCOMPARE(file.pos(), base + 2);
        sfile >> var3;
        QCOMPARE(file.pos(), base + 3);
        sfile >> var4;
        QCOMPARE(file.pos(), base + 4);
        file.seek(file.pos() - 4) ;   // Move it back 4, for we are going to write new values based on old ones
        QCOMPARE(file.pos(), base + 0);
        sfile << qint8(var1 + 5);
        QCOMPARE(file.pos(), base + 1);
        sfile << qint8(var2 + 5);
        QCOMPARE(file.pos(), base + 2);
        sfile << qint8(var3 + 5);
        QCOMPARE(file.pos(), base + 3);
        sfile << qint8(var4 + 5);
        QCOMPARE(file.pos(), base + 4);

    }
    file.close();
    fclose(fp);

    // check modified file
    {
        QFile f("FILEReadWrite.txt");
        QVERIFY(f.open(QFile::ReadOnly));
        QDataStream ds(&f);
        qint8 c = 0;
        ds >> c;
        QCOMPARE(c, (qint8)5);
        ds >> c;
        QCOMPARE(c, (qint8)6);
        ds >> c;
        QCOMPARE(c, (qint8)7);
        ds >> c;
        QCOMPARE(c, (qint8)8);
        ds >> c;
        QCOMPARE(c, (qint8)9);
        ds >> c;
        QCOMPARE(c, (qint8)10);
        ds >> c;
        QCOMPARE(c, (qint8)11);
        ds >> c;
        QCOMPARE(c, (qint8)12);
        ds >> c;
        QCOMPARE(c, (qint8)13);
        ds >> c;
        QCOMPARE(c, (qint8)14);
        ds >> c;
        QCOMPARE(c, (qint8)15);
        ds >> c;
        QCOMPARE(c, (qint8)16);
        f.close();
    }

    QFile::remove("FILEReadWrite.txt");
}


/*
#include <qglobal.h>
#define BUFFSIZE 1
#define FILESIZE   0x10000000f
void tst_QFile::largeFileSupport()
{
#ifdef Q_OS_SOLARIS
    QSKIP("Solaris does not support statfs", SkipAll);
#else
    qlonglong sizeNeeded = 2147483647;
    sizeNeeded *= 2;
    sizeNeeded += 1024;
    qlonglong freespace = qlonglong(0);
#ifdef Q_WS_WIN
    _ULARGE_INTEGER free;
    if (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based) {
        if (::GetDiskFreeSpaceExW((wchar_t *)QDir::currentPath().utf16(), &free, 0, 0))
            freespace = free.QuadPart;
    } else {
        if (::GetDiskFreeSpaceExA(QDir::currentPath().local8Bit(), &free, 0, 0))
            freespace = free.QuadPart;
    }
    if (freespace != 0) {
#elif defined(Q_OS_IRIX)
    struct statfs info;
    if (statfs(QDir::currentPath().local8Bit(), &info, sizeof(struct statfs), 0) == 0) {
        freespace = qlonglong(info.f_bfree * info.f_bsize);
#else
    struct statfs info;
    if (statfs(const_cast<char *>(QDir::currentPath().toLocal8Bit().constData()), &info) == 0) {
        freespace = qlonglong(info.f_bavail * info.f_bsize);
#endif
        if (freespace > sizeNeeded) {
            QFile bigFile("bigfile");
            if (bigFile.open(QFile::ReadWrite)) {
                char c[BUFFSIZE] = {'a'};
                QVERIFY(bigFile.write(c, BUFFSIZE) == BUFFSIZE);
                qlonglong oldPos = bigFile.pos();
                QVERIFY(bigFile.resize(sizeNeeded));
                QCOMPARE(oldPos, bigFile.pos());
                QVERIFY(bigFile.seek(sizeNeeded - BUFFSIZE));
                QVERIFY(bigFile.write(c, BUFFSIZE) == BUFFSIZE);

                bigFile.close();
                if (bigFile.open(QFile::ReadOnly)) {
                    QVERIFY(bigFile.read(c, BUFFSIZE) == BUFFSIZE);
                    int i = 0;
                    for (i=0; i<BUFFSIZE; i++)
                        QCOMPARE(c[i], 'a');
                    QVERIFY(bigFile.seek(sizeNeeded - BUFFSIZE));
                    QVERIFY(bigFile.read(c, BUFFSIZE) == BUFFSIZE);
                    for (i=0; i<BUFFSIZE; i++)
                        QCOMPARE(c[i], 'a');
                    bigFile.close();
                    QVERIFY(bigFile.remove());
                } else {
                    QVERIFY(bigFile.remove());
                    QFAIL("Could not reopen file");
                }
            } else {
                QFAIL("Could not open file");
            }
        } else {
            QSKIP("Not enough space to run test", SkipSingle);
        }
    } else {
        QFAIL("Could not determin disk space");
    }
#endif
}
*/

void tst_QFile::i18nFileName_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow( "01" ) << QString::fromUtf8("xxxxxxx.txt");
}

void tst_QFile::i18nFileName()
{
     QFETCH(QString, fileName);
     if (QFile::exists(fileName)) {
         QVERIFY(QFile::remove(fileName));
     }
     {
        QFile file(fileName);
        QVERIFY(file.open(QFile::WriteOnly | QFile::Text));
        QTextStream ts(&file);
        ts.setCodec("UTF-8");
        ts << fileName << endl;
     }
     {
        QFile file(fileName);
        QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
        QTextStream ts(&file);
        ts.setCodec("UTF-8");
        QString line = ts.readLine();
        QCOMPARE(line, fileName);
     }
     QVERIFY(QFile::remove(fileName));
}


void tst_QFile::longFileName_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow( "16 chars" ) << QString::fromLatin1("longFileName.txt");
    QTest::newRow( "52 chars" ) << QString::fromLatin1("longFileNamelongFileNamelongFileNamelongFileName.txt");
    QTest::newRow( "148 chars" ) << QString::fromLatin1("longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName.txt");
    QTest::newRow( "244 chars" ) << QString::fromLatin1("longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName.txt");
    QTest::newRow( "244 chars to absolutepath" ) << QFileInfo(QString::fromLatin1("longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName.txt")).absoluteFilePath();
  /* needs to be put on a windows 2000 > test machine
  QTest::newRow( "244 chars on UNC" ) <<  QString::fromLatin1("//arsia/D/troll/tmp/longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName.txt");*/
}

void tst_QFile::longFileName()
{
    QFETCH(QString, fileName);
    if (QFile::exists(fileName)) {
        QVERIFY(QFile::remove(fileName));
    }
    {
        QFile file(fileName);
#if QT_VERSION < 0x040100
#  ifdef Q_OS_WIN
        QEXPECT_FAIL("244 chars", "Fixed in 4.1", Continue);
        QEXPECT_FAIL("244 chars to absolutepath", "Fixed in 4.1", Continue);
#  endif
#elif defined(Q_WS_WIN)
        QT_WA({ if (false) ; }, { QEXPECT_FAIL("244 chars", "Full pathname must be less than 260 chars", Abort); });
        QT_WA({ if (false) ; }, { QEXPECT_FAIL("244 chars to absolutepath", "Full pathname must be less than 260 chars", Abort); });
#endif
        QVERIFY(file.open(QFile::WriteOnly | QFile::Text));
        QTextStream ts(&file);
        ts << fileName << endl;
    }
    {
        QFile file(fileName);
        QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
        QTextStream ts(&file);
        QString line = ts.readLine();
        QCOMPARE(line, fileName);
    }
    QString newName = fileName + QLatin1String("1");
    {
        QVERIFY(QFile::copy(fileName, newName));
        QFile file(newName);
        QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
        QTextStream ts(&file);
        QString line = ts.readLine();
        QCOMPARE(line, fileName);

    }
    QVERIFY(QFile::remove(newName));
    {
        QVERIFY(QFile::rename(fileName, newName));
        QFile file(newName);
        QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
        QTextStream ts(&file);
        QString line = ts.readLine();
        QCOMPARE(line, fileName);
    }
    QVERIFY(QFile::exists(newName));
    QVERIFY(QFile::remove(newName));
}

class MyEngine : public QAbstractFileEngine
{
public:
    MyEngine(int n) { number = n; }
    virtual ~MyEngine() {}

    void setFileName(const QString &) {}
    bool open(int ) { return false; }
    bool close() { return false; }
    bool flush() { return false; }
    qint64 size() const { return 123 + number; }
    qint64 at() const { return -1; }
    bool seek(qint64) { return false; }
    bool isSequential() const { return false; }
    qint64 read(char *, qint64) { return -1; }
    qint64 write(const char *, qint64) { return -1; }
    bool remove() { return false; }
    bool copy(const QString &) { return false; }
    bool rename(const QString &) { return false; }
    bool link(const QString &) { return false; }
    bool mkdir(const QString &, bool) const { return false; }
    bool rmdir(const QString &, bool) const { return false; }
    bool setSize(qint64) { return false; }
    QStringList entryList(QDir::Filters, const QStringList &) const { return QStringList(); }
    bool caseSensitive() const { return false; }
    bool isRelativePath() const { return false; }
    FileFlags fileFlags(FileFlags) const { return 0; }
    bool chmod(uint) { return false; }
    QString fileName(FileName) const { return name; }
    uint ownerId(FileOwner) const { return 0; }
    QString owner(FileOwner) const { return QString(); }
    QDateTime fileTime(FileTime) const { return QDateTime(); }

private:
    int number;
    QString name;
};

class MyHandler : public QAbstractFileEngineHandler
{
public:
    inline QAbstractFileEngine *create(const QString &) const
    {
        return new MyEngine(1);
    }
};

class MyHandler2 : public QAbstractFileEngineHandler
{
public:
    inline QAbstractFileEngine *create(const QString &) const
    {
        return new MyEngine(2);
    }
};

void tst_QFile::fileEngineHandler()
{
    // A file that does not exist has a size of 0.
    QFile::remove("ole.bull");
    QFile file("ole.bull");
    QCOMPARE(file.size(), qint64(0));

    // Instantiating our handler will enable the new engine.
    MyHandler handler;
    file.setFileName("ole.bull");
    QCOMPARE(file.size(), qint64(124));

    // A new, identical handler should take preference over the last one.
    MyHandler2 handler2;
    file.setFileName("ole.bull");
    QCOMPARE(file.size(), qint64(125));

}

class MyRecursiveHandler : public QAbstractFileEngineHandler
{
public:
    inline QAbstractFileEngine *create(const QString &fileName) const
    {
        if (fileName.startsWith(":!")) {
            QDir dir;
            if (dir.exists(fileName.mid(2)))
                return new QFSFileEngine(fileName.mid(2));
        }
        return 0;
    }
};

void tst_QFile::useQFileInAFileHandler()
{
    // This test should not dead-lock
    MyRecursiveHandler handler;
    QFile file(":!tst_qfile.cpp");
    QVERIFY(file.exists());
}

void tst_QFile::getCharFF()
{
    QFile file("file.txt");
    file.open(QFile::ReadWrite);
    file.write("\xff\xff\xff");
    file.flush();
    file.seek(0);

    char c;
    QVERIFY(file.getChar(&c));
    QVERIFY(file.getChar(&c));
    QVERIFY(file.getChar(&c));
}

void tst_QFile::remove_and_exists()
{
    QFile::remove("tull_i_grunn.txt");
    QFile f("tull_i_grunn.txt");

    QVERIFY(!f.exists());

    bool opened = f.open(QIODevice::WriteOnly);
    QVERIFY(opened);

    f.write(QString("testing that remove/exists work...").toLatin1());
    f.close();

    QVERIFY(f.exists());

    f.remove();
    QVERIFY(!f.exists());
}

void tst_QFile::removeOpenFile()
{
    {
        // remove an opened, write-only file
        QFile::remove("remove_unclosed.txt");
        QFile f("remove_unclosed.txt");

        QVERIFY(!f.exists());
        bool opened = f.open(QIODevice::WriteOnly);
        QVERIFY(opened);
        f.write(QString("testing that remove closes the file first...").toLatin1());

        bool removed = f.remove(); // remove should both close and remove the file
        QVERIFY(removed);
        QVERIFY(!f.isOpen());
        QVERIFY(!f.exists());
        QVERIFY(f.error() == QFile::NoError);
    }

    {
        // remove an opened, read-only file
        QFile::remove("remove_unclosed.txt");

        // first, write a file that we can remove
        {
            QFile f("remove_unclosed.txt");
            QVERIFY(!f.exists());
            bool opened = f.open(QIODevice::WriteOnly);
            QVERIFY(opened);
            f.write(QString("testing that remove closes the file first...").toLatin1());
            f.close();
        }

        QFile f("remove_unclosed.txt");
        bool opened = f.open(QIODevice::ReadOnly);
        QVERIFY(opened);
        f.readAll();
        // this used to only fail on FreeBSD (and Mac OS X)
        QVERIFY(f.flush());
        bool removed = f.remove(); // remove should both close and remove the file
        QVERIFY(removed);
        QVERIFY(!f.isOpen());
        QVERIFY(!f.exists());
        QVERIFY(f.error() == QFile::NoError);
    }
}

void tst_QFile::fullDisk()
{
#if QT_VERSION < 0x040102
    QSKIP("Fixed for 4.1.2", SkipAll);
#endif

    QFile file("/dev/full");
    if (!file.exists())
        QSKIP("/dev/full doesn't exist on this system", SkipAll);

    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("foobar", 6);

    QVERIFY(!file.flush());
    QCOMPARE(file.error(), QFile::ResourceError);
    QVERIFY(!file.flush());
    QCOMPARE(file.error(), QFile::ResourceError);

    char c;
    file.write(&c, 0);
    QVERIFY(!file.flush());
    QCOMPARE(file.error(), QFile::ResourceError);
    file.write(&c, 1);
    QVERIFY(!file.flush());
    QCOMPARE(file.error(), QFile::ResourceError);

    file.close();
    QVERIFY(!file.isOpen());
    QCOMPARE(file.error(), QFile::ResourceError);
    file.open(QIODevice::WriteOnly);
    QCOMPARE(file.error(), QFile::NoError);
    file.close();
    QCOMPARE(file.error(), QFile::NoError);

    // try again without flush:
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("foobar", 6);
    file.close();
    QVERIFY(file.error() != QFile::NoError);
}

void tst_QFile::writeLargeDataBlock_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("localfile") << QString("./largeblockfile.txt");
#ifdef Q_OS_WIN
    // Some semi-randomness to avoid collisions.
    QTest::newRow("unc file")
        << QString("//Gennan/TESTSHAREWRITABLE/largefile-%1-%2.txt")
        .arg(QHostInfo::localHostName())
        .arg(QTime::currentTime().msec());
#endif
}

void tst_QFile::writeLargeDataBlock()
{
    QFETCH(QString, fileName);

    // Generate a 64MB array with well defined contents.
    QByteArray array;
    array.resize(64 * 1024 * 1024);
    for (int i = 0; i < array.size(); ++i)
        array[i] = uchar(i);

    // Remove and open the target file
    QFile file(fileName);
    file.remove();
    if (file.open(QFile::WriteOnly)) {
        file.write(array);
        file.close();
        file.open(QFile::ReadOnly);
        array.clear();
        array = file.readAll();
        file.remove();
    } else {
        QFAIL(qPrintable(QString("Couldn't open file for writing: [%1]").arg(fileName)));
    }
    // Check that we got the right content
    QCOMPARE(array.size(), 64 * 1024 * 1024);
    for (int i = 0; i < array.size(); ++i) {
        if (array[i] != char(i)) {
            QFAIL(qPrintable(QString("Wrong contents! Char at %1 = %2, expected %3")
                  .arg(i).arg(int(uchar(array[i]))).arg(int(uchar(i)))));
        }
    }
}

void tst_QFile::readFromWriteOnlyFile()
{
    QFile file("writeonlyfile");
    QVERIFY(file.open(QFile::WriteOnly));
    char c;
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::read: WriteOnly device");
    QCOMPARE(file.read(&c, 1), qint64(-1));
}

void tst_QFile::writeToReadOnlyFile()
{
    QFile file("readonlyfile");
    QVERIFY(file.open(QFile::ReadOnly));
    char c = 0;
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::write: ReadOnly device");
    QCOMPARE(file.write(&c, 1), qint64(-1));
}

void tst_QFile::virtualFile()
{
    // test if QFile works with virtual files
    QString fname;
#if defined(Q_OS_LINUX)
    fname = "/proc/self/maps";
#elif defined(Q_OS_AIX)
    fname = QString("/proc/%1/map").arg(getpid());
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    fname = "/proc/curproc/map";
#else
    QSKIP("This platform does not have 0-sized virtual files", SkipAll);
#endif

    // consistency check
    QFileInfo fi(fname);
    QVERIFY(fi.exists());
    QVERIFY(fi.isFile());
    QCOMPARE(fi.size(), Q_INT64_C(0));

    // open the file
    QFile f(fname);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(f.size(), Q_INT64_C(0));
    QVERIFY(f.atEnd());

    // read data
    QByteArray data = f.read(16);
    QCOMPARE(data.size(), 16);
    QCOMPARE(f.pos(), Q_INT64_C(16));

    // line-reading
    data = f.readLine();
    QVERIFY(!data.isEmpty());

    // read all:
    data = f.readAll();
    QVERIFY(f.pos() != 0);
#if QT_VERSION >= 0x040200
    QVERIFY(!data.isEmpty());
#endif

    // seeking
    QVERIFY(f.seek(1));
    QCOMPARE(f.pos(), Q_INT64_C(1));
}

void tst_QFile::textFile()
{
#ifdef Q_OS_WIN
    FILE *fs = ::fopen("writeabletextfile", "wt");
#else
    FILE *fs = ::fopen("writeabletextfile", "w");
#endif
    QFile f;
    QByteArray part1("This\nis\na\nfile\nwith\nnewlines\n");
    QByteArray part2("Add\nsome\nmore\nnewlines\n");

    QVERIFY(f.open(fs, QIODevice::WriteOnly));
    f.write(part1);
    f.write(part2);
    f.close();
    ::fclose(fs);

    QFile file("writeabletextfile");
    QVERIFY(file.open(QIODevice::ReadOnly));

    QByteArray data = file.readAll();

    QByteArray expected = part1 + part2;
#ifdef Q_OS_WIN
    expected.replace("\n", "\015\012");
#endif
    QCOMPARE(data, expected);
    file.close();
    file.remove();
}

void tst_QFile::readPastEnd()
{
    QFile file("tst_qfile.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.size(), qint64(file.readAll().size()));

    QCOMPARE(file.readAll().size(), 0);
    QCOMPARE(file.read(6).size(), 0);
    char c;
    QVERIFY(!file.getChar(&c));
    QCOMPARE(file.read(&c, 1), qint64(0)); // not -1
}

void tst_QFile::rename_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<QString>("destination");
    QTest::addColumn<bool>("result");

    QTest::newRow("a -> b") << QString("a") << QString("b") << false;
    QTest::newRow("a -> .") << QString("a") << QString(".") << false;
    QTest::newRow("renamefile -> renamefile") << QString("renamefile") << QString("renamefile") << false;
    QTest::newRow("renamefile -> Makefile") << QString("renamefile") << QString("resources") << false;
#ifdef Q_OS_UNIX
    QTest::newRow("renamefile -> /etc/renamefile") << QString("renamefile") << QString("/etc/renamefile") << false;
#endif
    QTest::newRow("renamefile -> renamedfile") << QString("renamefile") << QString("renamedfile") << true;
    QTest::newRow("renamefile -> ..") << QString("renamefile") << QString("..") << false;
}

void tst_QFile::rename()
{
    QFETCH(QString, source);
    QFETCH(QString, destination);
    QFETCH(bool, result);

    QFile::remove("renamedfile");
    QFile f("renamefile");
    f.open(QFile::WriteOnly);
    f.close();

    QFile file(source);
    QCOMPARE(file.rename(destination), result);
    if (result)
        QCOMPARE(file.error(), QFile::NoError);
    else
        QCOMPARE(file.error(), QFile::RenameError);

    QFile::remove("renamefile");
}

void tst_QFile::appendAndRead()
{
    QFile writeFile(QLatin1String("appendfile.txt"));
    QVERIFY(writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate));

    QFile readFile(QLatin1String("appendfile.txt"));
    QVERIFY(readFile.open(QIODevice::ReadOnly));

    // Write to the end of the file, then read that character back, and so on.
    for (int i = 0; i < 16384; ++i) {
        char c = '\0';
        writeFile.putChar(char(i % 256));
        writeFile.flush();
        QVERIFY(readFile.getChar(&c));
        QCOMPARE(c, char(i % 256));
        QCOMPARE(readFile.pos(), writeFile.pos());
    }

    // Write blocks and read them back
    for (int j = 0; j < 18; ++j) {
        writeFile.write(QByteArray(1 << j, '@'));
        writeFile.flush();
        QCOMPARE(readFile.read(1 << j).size(), 1 << j);
    }

    QFile::remove(QLatin1String("appendfile.txt"));
}

void tst_QFile::miscWithUncPathAsCurrentDir()
{
#ifdef Q_OS_WIN
    QString current = QDir::currentPath();
    QVERIFY(QDir::setCurrent("//gennan/testsharewritable"));
    QFile file("test.pri");
    QVERIFY(file.exists());
    QCOMPARE(int(file.size()), 34);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY(QDir::setCurrent(current));
#endif
}

QTEST_MAIN(tst_QFile)
#include "tst_qfile.moc"
