/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qfile.h>
#include <qdir.h>
#include <qcoreapplication.h>
#include <qtemporaryfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include <qdebug.h>
//TESTED_CLASS=
//TESTED_FILES=corelib/io/qfileinfo.h corelib/io/qfileinfo.cpp

class tst_QFileInfo : public QObject
{
Q_OBJECT

public:
    tst_QFileInfo();

private slots:
    void getSetCheck();
    void isFile_data();
    void isFile();

    void isDir_data();
    void isDir();

    void isRoot_data();
    void isRoot();

    void exists_data();
    void exists();

    void absolutePath_data();
    void absolutePath();

    void absFilePath_data();
    void absFilePath();

    void canonicalPath();
    void canonicalFilePath();

    void fileName_data();
    void fileName();

    void dir_data();
    void dir();

    void suffix_data();
    void suffix();

    void completeSuffix_data();
    void completeSuffix();

    void baseName_data();
    void baseName();

    void completeBaseName_data();
    void completeBaseName();

    void permission_data();
    void permission();

    void size_data();
    void size();

    void compare_data();
    void compare();

    void consistent_data();
    void consistent();

    void fileTimes_data();
    void fileTimes();

    void isSymLink();

    void isHidden();

    void refresh();

#ifdef Q_OS_WIN
    void brokenShortcut();
#endif

#ifdef Q_OS_UNIX
    void isWritable();
#endif
    void isExecutable();
	void testDecomposedUnicodeNames_data();
	void testDecomposedUnicodeNames();
};

// Testing get/set functions
void tst_QFileInfo::getSetCheck()
{
    QFileInfo obj1;
    // bool QFileInfo::caching()
    // void QFileInfo::setCaching(bool)
    obj1.setCaching(false);
    QCOMPARE(false, obj1.caching());
    obj1.setCaching(true);
    QCOMPARE(true, obj1.caching());
}

tst_QFileInfo::tst_QFileInfo()
{
}

void tst_QFileInfo::isFile_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QDir::currentPath() << false;
    QTest::newRow("data1") << "tst_qfileinfo.cpp" << true;
    QTest::newRow("data2") << ":/tst_qfileinfo/resources/" << false;
    QTest::newRow("data3") << ":/tst_qfileinfo/resources/file1" << true;
}

void tst_QFileInfo::isFile()
{
    QFETCH(QString, path);
    QFETCH(bool, expected);

    QFileInfo fi(path);
    QCOMPARE(fi.isFile(), expected);
}


void tst_QFileInfo::isDir_data()
{
    // create a broken symlink
    QFile::remove("brokenlink.lnk");
    QFile file3("dummyfile");
    file3.open(QIODevice::WriteOnly);
    if (file3.link("brokenlink.lnk")) {
        file3.remove();
        QFileInfo info3("brokenlink.lnk");
        QVERIFY( info3.isSymLink() );
    }

    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QDir::currentPath() << true;
    QTest::newRow("data1") << "tst_qfileinfo.cpp" << false;
    QTest::newRow("data2") << ":/tst_qfileinfo/resources/" << true;
    QTest::newRow("data3") << ":/tst_qfileinfo/resources/file1" << false;

    QTest::newRow("simple dir") << "resources" << true;
    QTest::newRow("simple dir with slash") << "resources/" << true;

    QTest::newRow("broken link") << "brokenlink.lnk" << false;

#ifdef Q_OS_WIN
    QTest::newRow("drive 1") << "c:" << true;
    QTest::newRow("drive 2") << "c:/" << true;
    //QTest::newRow("drive 2") << "t:s" << false;
    QTest::newRow("unc 1") << "//gennan" << true;
    QTest::newRow("unc 2") << "//gennan/" << true;
    QTest::newRow("unc 3") << "//gennan/testshare" << true;
    QTest::newRow("unc 4") << "//gennan/testshare/" << true;
    QTest::newRow("unc 5") << "//gennan/testshare/tmp" << true;
    QTest::newRow("unc 6") << "//gennan/testshare/tmp/" << true;
    QTest::newRow("unc 7") << "//gennan/testshare/adirthatshouldnotexist" << false;
#endif
}

void tst_QFileInfo::isDir()
{
    QFETCH(QString, path);
    QFETCH(bool, expected);

    QFileInfo fi(path);
    QCOMPARE(fi.isDir(), expected);
}

void tst_QFileInfo::isRoot_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QDir::currentPath() << false;
    QTest::newRow("data1") << "/" << true;
    QTest::newRow("data2") << ":/tst_qfileinfo/resources/" << false;
    QTest::newRow("data3") << ":/" << true;

    QTest::newRow("simple dir") << "resources" << false;
    QTest::newRow("simple dir with slash") << "resources/" << false;

#ifdef Q_OS_WIN
    QTest::newRow("drive 1") << "c:" << false;
    QTest::newRow("drive 2") << "c:/" << true;
    QTest::newRow("unc 1") << "//gennan" << true;
    QTest::newRow("unc 2") << "//gennan/" << true;
    QTest::newRow("unc 3") << "//gennan/testshare" << false;
    QTest::newRow("unc 4") << "//gennan/testshare/" << false;
    QTest::newRow("unc 7") << "//foobar" << false;
#endif
}

void tst_QFileInfo::isRoot()
{
    QFETCH(QString, path);
    QFETCH(bool, expected);

    QFileInfo fi(path);
    QCOMPARE(fi.isRoot(), expected);
}

void tst_QFileInfo::exists_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QDir::currentPath() << true;
    QTest::newRow("data1") << "tst_qfileinfo.cpp" << true;
    QTest::newRow("data2") << "/I/do_not_expect_this_path_to_exist/" << false;
    QTest::newRow("data3") << ":/tst_qfileinfo/resources/" << true;
    QTest::newRow("data4") << ":/tst_qfileinfo/resources/file1" << true;
    QTest::newRow("data5") << ":/I/do_not_expect_this_path_to_exist/" << false;
    QTest::newRow("data6") << "resources/*" << false;
    QTest::newRow("data7") << "resources/*.foo" << false;
    QTest::newRow("data8") << "resources/*.ext1" << false;
    QTest::newRow("data9") << "." << true;
    QTest::newRow("data10") << ". " << false;

    QTest::newRow("simple dir") << "resources" << true;
    QTest::newRow("simple dir with slash") << "resources/" << true;

#ifdef Q_OS_WIN
    QTest::newRow("unc 1") << "//gennan" << true;
    QTest::newRow("unc 2") << "//gennan/" << true;
    QTest::newRow("unc 3") << "//gennan/testshare" << true;
    QTest::newRow("unc 4") << "//gennan/testshare/" << true;
    QTest::newRow("unc 5") << "//gennan/testshare/tmp" << true;
    QTest::newRow("unc 6") << "//gennan/testshare/tmp/" << true;
    QTest::newRow("unc 7") << "//gennan/testshare/adirthatshouldnotexist" << false;
    QTest::newRow("unc 8") << "//gennan/asharethatshouldnotexist" << false;
    QTest::newRow("unc 9") << "//foobar" << false;
#endif
}

void tst_QFileInfo::exists()
{
    QFETCH(QString, path);
    QFETCH(bool, expected);

    QFileInfo fi(path);
    QCOMPARE(fi.exists(), expected);
}

void tst_QFileInfo::absolutePath_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("filename");

    QString drivePrefix;
#ifdef Q_OS_WIN
    drivePrefix = QDir::currentPath().left(2);
#endif
    QTest::newRow("0") << "/machine/share/dir1/" << drivePrefix + "/machine/share/dir1" << "";
    QTest::newRow("1") << "/machine/share/dir1" << drivePrefix + "/machine/share" << "dir1";
    QTest::newRow("2") << "/usr/local/bin" << drivePrefix + "/usr/local" << "bin";
    QTest::newRow("3") << "/usr/local/bin/" << drivePrefix + "/usr/local/bin" << "";
    QTest::newRow("/test") << "/test" << drivePrefix + "/" << "test";

#ifdef Q_OS_WIN
    // see task 102898
    QTest::newRow("c:\\autoexec.bat") << "c:\\autoexec.bat" << "C:/"
                                      << "autoexec.bat";
#endif
}

void tst_QFileInfo::absolutePath()
{
    QFETCH(QString, file);
    QFETCH(QString, path);
    QFETCH(QString, filename);

    QFileInfo fi(file);

    QCOMPARE(fi.absolutePath(), path);
    QCOMPARE(fi.fileName(), filename);
}

void tst_QFileInfo::absFilePath_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("expected");

    QTest::newRow("relativeFile") << "tmp.txt" << QDir::currentPath() + "/tmp.txt";
    QTest::newRow("relativeFileInSubDir") << "temp/tmp.txt" << QDir::currentPath() + "/" + "temp/tmp.txt";
#ifdef Q_OS_WIN
    QString curr = QDir::currentPath();
    curr.remove(0, 2);   // Make it a absolute path with no drive specifier: \depot\qt-4.2\tests\auto\qfileinfo 
    QTest::newRow(".")            << curr << QDir::currentPath();
    QTest::newRow("absFilePath") << "c:\\home\\andy\\tmp.txt" << "C:/home/andy/tmp.txt";
#else
    QTest::newRow("absFilePath") << "/home/andy/tmp.txt" << "/home/andy/tmp.txt";
#endif
}

void tst_QFileInfo::absFilePath()
{
    QFETCH(QString, file);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    QCOMPARE(fi.absoluteFilePath(), expected);
}

void tst_QFileInfo::canonicalPath()
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(true);
    tempFile.open();
    QFileInfo fi(tempFile.fileName());
    QCOMPARE(fi.canonicalPath(), QFileInfo(QDir::tempPath()).canonicalFilePath());
}

void tst_QFileInfo::canonicalFilePath()
{
    const QString fileName("tmp.canon");
    QFile tempFile(fileName);
    QVERIFY(tempFile.open(QFile::WriteOnly));
    QFileInfo fi(tempFile.fileName());
    QCOMPARE(fi.canonicalFilePath(), QDir::currentPath() + "/" + fileName);
    tempFile.remove();
}

void tst_QFileInfo::fileName_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("expected");

    QTest::newRow("relativeFile") << "tmp.txt" << "tmp.txt";
    QTest::newRow("relativeFileInSubDir") << "temp/tmp.txt" << "tmp.txt";
#ifdef Q_OS_WIN
    QTest::newRow("absFilePath") << "c:\\home\\andy\\tmp.txt" << "tmp.txt";
#else
    QTest::newRow("absFilePath") << "/home/andy/tmp.txt" << "tmp.txt";
#endif
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << "file1.ext1";
    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1.ext2" << "file1.ext1.ext2";

    QTest::newRow("ending slash [small]") << QString::fromLatin1("/a/") << QString::fromLatin1("");
    QTest::newRow("no ending slash [small]") << QString::fromLatin1("/a") << QString::fromLatin1("a");

    QTest::newRow("ending slash") << QString::fromLatin1("/somedir/") << QString::fromLatin1("");
    QTest::newRow("no ending slash") << QString::fromLatin1("/somedir") << QString::fromLatin1("somedir");
}

void tst_QFileInfo::fileName()
{
    QFETCH(QString, file);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    QCOMPARE(fi.fileName(), expected);
}

void tst_QFileInfo::dir_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<bool>("absPath");
    QTest::addColumn<QString>("expected");

    QTest::newRow("relativeFile") << "tmp.txt" << false << ".";
    QTest::newRow("relativeFileAbsPath") << "tmp.txt" << true << QDir::currentPath();
    QTest::newRow("relativeFileInSubDir") << "temp/tmp.txt" << false << "temp";
    QTest::newRow("relativeFileInSubDirAbsPath") << "temp/tmp.txt" << true << QDir::currentPath() + "/temp";
    QTest::newRow("absFilePath") << QDir::currentPath() + "/tmp.txt" << false << QDir::currentPath();
    QTest::newRow("absFilePathAbsPath") << QDir::currentPath() + "/tmp.txt" << true << QDir::currentPath();
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << true << ":/tst_qfileinfo/resources";
}

void tst_QFileInfo::dir()
{
    QFETCH(QString, file);
    QFETCH(bool, absPath);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    if (absPath)
        QCOMPARE(fi.absolutePath(), expected);
    else
        QCOMPARE(fi.path(), expected);
}


void tst_QFileInfo::suffix_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("expected");

    QTest::newRow("noextension0") << "file" << "";
    QTest::newRow("noextension1") << "/path/to/file" << "";
    QTest::newRow("data0") << "file.tar" << "tar";
    QTest::newRow("data1") << "file.tar.gz" << "gz";
    QTest::newRow("data2") << "/path/file/file.tar.gz" << "gz";
    QTest::newRow("data3") << "/path/file.tar" << "tar";
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << "ext1";
    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1.ext2" << "ext2";
    QTest::newRow("hidden1") << ".ext1" << "ext1";
    QTest::newRow("hidden1") << ".ext" << "ext";
    QTest::newRow("hidden1") << ".ex" << "ex";
    QTest::newRow("hidden1") << ".e" << "e";
    QTest::newRow("hidden2") << ".ext1.ext2" << "ext2";
    QTest::newRow("hidden2") << ".ext.ext2" << "ext2";
    QTest::newRow("hidden2") << ".ex.ext2" << "ext2";
    QTest::newRow("hidden2") << ".e.ext2" << "ext2";
    QTest::newRow("hidden2") << "..ext2" << "ext2";
}

void tst_QFileInfo::suffix()
{
    QFETCH(QString, file);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    QCOMPARE(fi.suffix(), expected);
}


void tst_QFileInfo::completeSuffix_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("expected");

    QTest::newRow("noextension0") << "file" << "";
    QTest::newRow("noextension1") << "/path/to/file" << "";
    QTest::newRow("data0") << "file.tar" << "tar";
    QTest::newRow("data1") << "file.tar.gz" << "tar.gz";
    QTest::newRow("data2") << "/path/file/file.tar.gz" << "tar.gz";
    QTest::newRow("data3") << "/path/file.tar" << "tar";
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << "ext1";
    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1.ext2" << "ext1.ext2";
}

void tst_QFileInfo::completeSuffix()
{
    QFETCH(QString, file);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    QCOMPARE(fi.completeSuffix(), expected);
}

void tst_QFileInfo::baseName_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("expected");

    QTest::newRow("data0") << "file.tar" << "file";
    QTest::newRow("data1") << "file.tar.gz" << "file";
    QTest::newRow("data2") << "/path/file/file.tar.gz" << "file";
    QTest::newRow("data3") << "/path/file.tar" << "file";
    QTest::newRow("data4") << "/path/file" << "file";
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << "file1";
    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1.ext2" << "file1";
}

void tst_QFileInfo::baseName()
{
    QFETCH(QString, file);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    QCOMPARE(fi.baseName(), expected);
}

void tst_QFileInfo::completeBaseName_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("expected");

    QTest::newRow("data0") << "file.tar" << "file";
    QTest::newRow("data1") << "file.tar.gz" << "file.tar";
    QTest::newRow("data2") << "/path/file/file.tar.gz" << "file.tar";
    QTest::newRow("data3") << "/path/file.tar" << "file";
    QTest::newRow("data4") << "/path/file" << "file";
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << "file1";
    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1.ext2" << "file1.ext1";
}

void tst_QFileInfo::completeBaseName()
{
    QFETCH(QString, file);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    QCOMPARE(fi.completeBaseName(), expected);
}

void tst_QFileInfo::permission_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("perms");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QCoreApplication::instance()->applicationFilePath() << int(QFile::ExeUser) << true;
    QTest::newRow("data1") << "tst_qfileinfo.cpp" << int(QFile::ReadUser) << true;
//    QTest::newRow("data2") << "tst_qfileinfo.cpp" << int(QFile::WriteUser) << false;
    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << int(QFile::ReadUser) << true;
    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1" << int(QFile::WriteUser) << false;
    QTest::newRow("resource3") << ":/tst_qfileinfo/resources/file1.ext1" << int(QFile::ExeUser) << false;

}

void tst_QFileInfo::permission()
{
    QFETCH(QString, file);
    QFETCH(int, perms);
    QFETCH(bool, expected);
    QFileInfo fi(file);
    QCOMPARE(fi.permission((QFile::Permissions)perms), expected);
}

void tst_QFileInfo::size_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("size");

    QTest::newRow("resource1") << ":/tst_qfileinfo/resources/file1.ext1" << 0;
    QFile::remove("file1");
    QFile file("file1");
    QVERIFY(file.open(QFile::WriteOnly));
    QCOMPARE(file.write("JAJAJAA"), qint64(7));
    QTest::newRow("created-file") << "file1" << 7;

    QTest::newRow("resource2") << ":/tst_qfileinfo/resources/file1.ext1.ext2" << 0;
}

void tst_QFileInfo::size()
{
    QFETCH(QString, file);

    QFileInfo fi(file);
    (void)fi.permissions();     // see task 104198
    QTEST(int(fi.size()), "size");
}

void tst_QFileInfo::compare_data()
{
    QTest::addColumn<QString>("file1");
    QTest::addColumn<QString>("file2");
    QTest::addColumn<bool>("same");

    QTest::newRow("data0") << QString::fromLatin1("tst_qfileinfo.cpp") << QString::fromLatin1("tst_qfileinfo.cpp") << true;
    QTest::newRow("data1") << QString::fromLatin1("tst_qfileinfo.cpp") << QString::fromLatin1("/tst_qfileinfo.cpp") << false;
    QTest::newRow("data2") << QString::fromLatin1("tst_qfileinfo.cpp")
                        << QDir::currentPath() + QString::fromLatin1("/tst_qfileinfo.cpp") << true;
    QTest::newRow("casesense1") << QString::fromLatin1("tst_qfileInfo.cpp")
                        << QDir::currentPath() + QString::fromLatin1("/tst_qfileinfo.cpp")
#ifdef Q_OS_WIN
                        << true;
#else
                        << false;
#endif

}
void tst_QFileInfo::compare()
{
    QFETCH(QString, file1);
    QFETCH(QString, file2);
    QFETCH(bool, same);
    QFileInfo fi1(file1), fi2(file2);
    QCOMPARE(same, fi1 == fi2);
}

void tst_QFileInfo::consistent_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("expected");

#ifdef Q_OS_WIN
    QTest::newRow("slashes") << QString::fromLatin1("\\a\\a\\a\\a") << QString::fromLatin1("/a/a/a/a");
#endif
    QTest::newRow("ending slash") << QString::fromLatin1("/a/somedir/") << QString::fromLatin1("/a/somedir/");
    QTest::newRow("no ending slash") << QString::fromLatin1("/a/somedir") << QString::fromLatin1("/a/somedir");
}

void tst_QFileInfo::consistent()
{
    QFETCH(QString, file);
    QFETCH(QString, expected);

    QFileInfo fi(file);
    QCOMPARE(fi.filePath(), expected);
    QCOMPARE(fi.dir().path() + "/" + fi.fileName(), expected);
}


void tst_QFileInfo::fileTimes_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::newRow("simple") << QString::fromLatin1("simplefile.txt");
    QTest::newRow( "longfile" ) << QString::fromLatin1("longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName.txt");
    QTest::newRow( "longfile absolutepath" ) << QFileInfo(QString::fromLatin1("longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName"
                                                     "longFileNamelongFileNamelongFileNamelongFileName.txt")).absoluteFilePath();
}

void tst_QFileInfo::fileTimes()
{
    QFETCH(QString, fileName);
    if (QFile::exists(fileName)) {
        QVERIFY(QFile::remove(fileName));
    }
    QTest::qSleep(1000);
    {
        QFile file(fileName);
#if QT_VERSION < 0x040100
#  ifdef Q_OS_WIN
        QEXPECT_FAIL("longfile", "Fixed in 4.1", Continue);
        QEXPECT_FAIL("longfile absolutepath", "Fixed in 4.1", Continue);
#  endif
#endif
        QVERIFY(file.open(QFile::WriteOnly | QFile::Text));
        QTextStream ts(&file);
        ts << fileName << endl;
    }
    QTest::qSleep(1000);
    QDateTime beforeWrite = QDateTime::currentDateTime();
    QTest::qSleep(2000);
    {
        QFileInfo fileInfo(fileName);
        QVERIFY(fileInfo.created() < beforeWrite);
        QFile file(fileName);
        QVERIFY(file.open(QFile::ReadWrite | QFile::Text));
        QTextStream ts(&file);
        ts << fileName << endl;
    }
    QTest::qSleep(1000);
    QDateTime beforeRead = QDateTime::currentDateTime();
    QTest::qSleep(1000);
    {
        QFileInfo fileInfo(fileName);
// On unix created() returns the same as lastModified().
#ifndef Q_OS_UNIX
        QVERIFY(fileInfo.created() < beforeWrite);
#endif
        QVERIFY(fileInfo.lastModified() > beforeWrite);
        QFile file(fileName);
        QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
        QTextStream ts(&file);
        QString line = ts.readLine();
        QCOMPARE(line, fileName);
    }

    QFileInfo fileInfo(fileName);
#ifndef Q_OS_UNIX
    QVERIFY(fileInfo.created() < beforeWrite);
#endif
    if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
        QVERIFY(fileInfo.lastRead().addDays(1) > beforeRead);
    }else{
        QVERIFY(fileInfo.lastRead() > beforeRead);
    }
    QVERIFY(fileInfo.lastModified() > beforeWrite);
    QVERIFY(fileInfo.lastModified() < beforeRead);

    QVERIFY(QFile::remove(fileName));
}

void tst_QFileInfo::isSymLink()
{
    QFile::remove("link.lnk");
    QFile::remove("brokenlink.lnk");

    QFileInfo info1("tst_qfileinfo.cpp");
    QVERIFY( !info1.isSymLink() );

    QFile file2("tst_qfileinfo.cpp");
    if (file2.link("link.lnk")) {
        QFileInfo info2("link.lnk");
        QVERIFY( info2.isSymLink() );
    }

    QFile file3("dummyfile");
    file3.open(QIODevice::WriteOnly);
    if (file3.link("brokenlink.lnk")) {
        file3.remove();
        QFileInfo info3("brokenlink.lnk");
        QVERIFY( info3.isSymLink() );
    }

}

void tst_QFileInfo::isHidden()
{
    // Drives
    foreach (QFileInfo info, QDir::drives()) {
        QVERIFY(info.exists());
        QVERIFY(info.isDir());
        QVERIFY(!info.isHidden());
    }
}

void tst_QFileInfo::refresh()
{
    QFile::remove("file1");
    QFile file("file1");
    QVERIFY(file.open(QFile::WriteOnly));
    QCOMPARE(file.write("JAJAJAA"), qint64(7));
    file.flush();

    QFileInfo info(file);
    QCOMPARE(info.size(), qint64(7));

    QCOMPARE(file.write("JOJOJO"), qint64(6));
    file.flush();

    QCOMPARE(info.size(), qint64(7));
    info.refresh();
    QCOMPARE(info.size(), qint64(13));

    QFileInfo info2 = info;
    QCOMPARE(info2.size(), info.size());

    info2.refresh();
    QCOMPARE(info2.size(), info.size());
}

#ifdef Q_OS_WIN
void tst_QFileInfo::brokenShortcut()
{
    QString linkName("borkenlink.lnk");
    QFile::remove(linkName);
    QFile file(linkName);
    file.open(QFile::WriteOnly);
    file.write("b0rk");
    file.close();

    QFileInfo info(linkName);
    QVERIFY(info.isSymLink());
    QVERIFY(!info.exists());
    QFile::remove(linkName);
}
#endif

#ifdef Q_OS_UNIX
void tst_QFileInfo::isWritable()
{
    if (::getuid() == 0)
        QVERIFY(QFileInfo("/etc/passwd").isWritable());
    else
        QVERIFY(!QFileInfo("/etc/passwd").isWritable());
}
#endif

void tst_QFileInfo::isExecutable()
{
    QString appPath = QCoreApplication::applicationDirPath();
    appPath += "/tst_qfileinfo";
#if defined(Q_OS_WIN)
    appPath += ".exe";
#endif
    QFileInfo fi(appPath);
    QCOMPARE(fi.isExecutable(), true);

    QCOMPARE(QFileInfo("qfileinfo.pro").isExecutable(), false);
}


void tst_QFileInfo::testDecomposedUnicodeNames_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("exists");
	QString currPath = QDir::currentPath();
    QTest::newRow("latin-only") << currPath + "/TestFiles/4.pdf" << "4.pdf" << true;
	QTest::newRow("one-decomposed uni") << currPath + QString::fromUtf8("/TestFiles/4 ä.pdf") << QString::fromUtf8("4 ä.pdf") << true;
	QTest::newRow("many-decomposed uni") << currPath + QString::fromUtf8("/TestFiles/4 äääcopy.pdf") << QString::fromUtf8("4 äääcopy.pdf") << true;
	QTest::newRow("no decomposed") << currPath + QString::fromUtf8("/TestFiles/4 øøøcopy.pdf") << QString::fromUtf8("4 øøøcopy.pdf") << true;
}

void tst_QFileInfo::testDecomposedUnicodeNames()
{
#ifndef Q_OS_MAC
    QSKIP("This is a OS X only test (unless you know more about filesystems, then maybe you should try it ;)", SkipAll);
#endif
    QFETCH(QString, filePath);

    QFileInfo file(filePath);
    QTEST(file.fileName(), "fileName");
    QTEST(file.exists(), "exists");
}

QTEST_MAIN(tst_QFileInfo)
#include "tst_qfileinfo.moc"
