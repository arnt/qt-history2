/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qstringlist.h>



//TESTED_CLASS=
//TESTED_FILES=corelib/io/qdir.h corelib/io/qdir.cpp

class tst_QDir : public QObject
{
Q_OBJECT

public:
    tst_QDir();
    virtual ~tst_QDir();

private slots:
    void getSetCheck();
    void construction();

    void setPath_data();
    void setPath();

    void entryList_data();
    void entryList();

    void entryListSimple_data();
    void entryListSimple();

    void entryListWithSymLinks();

    void mkdir_data();
    void mkdir();

    void rmdir_data();
    void rmdir();

    void exists_data();
    void exists();

    void isRelativePath_data();
    void isRelativePath();

    void canonicalPath_data();
    void canonicalPath();

    void current_data();
    void current();

    void cd_data();
    void cd();

    void setNameFilters_data();
    void setNameFilters();

    void cleanPath_data();
    void cleanPath();

    void compare();
    void QDir_default();

    void filePath_data();
    void filePath();

    void absoluteFilePath_data();
    void absoluteFilePath();

    void absolutePath_data();
    void absolutePath();

    void relativeFilePath_data();
    void relativeFilePath();

    void remove();
    void rename();

    void exists2_data();
    void exists2();

    void dirName_data();
    void dirName();

    void operator_eq();

    void dotAndDotDot();
#ifdef QT3_SUPPORT
    void matchAllDirs();
#endif
    void homePath();

    void nativeSeparators();
};

// Testing get/set functions
void tst_QDir::getSetCheck()
{
    QDir obj1;
    // Filters QDir::filter()
    // void QDir::setFilter(Filters)
    obj1.setFilter(QDir::Filters(QDir::Dirs));
    QCOMPARE(QDir::Filters(QDir::Dirs), obj1.filter());
    obj1.setFilter(QDir::Filters(QDir::Dirs | QDir::Files));
    QCOMPARE(QDir::Filters(QDir::Dirs | QDir::Files), obj1.filter());
    obj1.setFilter(QDir::Filters(QDir::NoFilter));
    QCOMPARE(QDir::Filters(QDir::NoFilter), obj1.filter());

    // SortFlags QDir::sorting()
    // void QDir::setSorting(SortFlags)
    obj1.setSorting(QDir::SortFlags(QDir::Name));
    QCOMPARE(QDir::SortFlags(QDir::Name), obj1.sorting());
    obj1.setSorting(QDir::SortFlags(QDir::Name | QDir::IgnoreCase));
    QCOMPARE(QDir::SortFlags(QDir::Name | QDir::IgnoreCase), obj1.sorting());
    obj1.setSorting(QDir::SortFlags(QDir::NoSort));
    QCOMPARE(QDir::SortFlags(QDir::NoSort), obj1.sorting());
}

tst_QDir::tst_QDir()
{
}

tst_QDir::~tst_QDir()
{

}

void tst_QDir::construction()
{
    QFileInfo myFileInfo("/machine/share/dir1/file1");
    QDir myDir(myFileInfo.absoluteDir()); // this asserted
    QCOMPARE(myFileInfo.absoluteDir().absolutePath(), myDir.absolutePath());
}

void tst_QDir::setPath_data()
{
    QTest::addColumn<QString>("dir1");
    QTest::addColumn<QString>("dir2");

    QTest::newRow("data0") << QString(".") << QString("..");
#if defined(Q_WS_WIN) && !defined(Q_OS_TEMP)
    QTest::newRow("data1") << QString("c:/") << QDir::currentPath();
#endif
}

void tst_QDir::setPath()
{
    QFETCH(QString, dir1);
    QFETCH(QString, dir2);

    QDir shared;
    QDir qDir1(dir1);
    QStringList entries1 = qDir1.entryList();
    shared.setPath(dir1);
    QCOMPARE(shared.entryList(), entries1);

    QDir qDir2(dir2);
    QStringList entries2 = qDir2.entryList();
    shared.setPath(dir2);
    QCOMPARE(shared.entryList(), entries2);
}

void tst_QDir::mkdir_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("recurse");

    QStringList dirs;
    dirs << QDir::currentPath() + "/testdir/one/two/three"
         << QDir::currentPath() + "/testdir/two"
         << QDir::currentPath() + "/testdir/two/three";
    QTest::newRow("data0") << dirs.at(0) << true;
    QTest::newRow("data1") << dirs.at(1) << false;
    QTest::newRow("data2") << dirs.at(2) << false;

    // Ensure that none of these directories already exist
    QDir dir;
    for (int i = 0; i < dirs.count(); ++i)
        dir.rmpath(dirs.at(i));
}

void tst_QDir::mkdir()
{
    QFETCH(QString, path);
    QFETCH(bool, recurse);

    QDir dir;
    dir.rmdir(path);
    if (recurse)
        QVERIFY(dir.mkpath(path));
    else
        QVERIFY(dir.mkdir(path));

    //make sure it really exists (ie that mkdir returns the right value)
    QFileInfo fi(path);
    QVERIFY(fi.exists() && fi.isDir());
}

void tst_QDir::rmdir_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("recurse");

    QTest::newRow("data0") << QDir::currentPath() + "/testdir/one/two/three" << true;
    QTest::newRow("data1") << QDir::currentPath() + "/testdir/two/three" << false;
    QTest::newRow("data2") << QDir::currentPath() + "/testdir/two" << false;
}

void tst_QDir::rmdir()
{
    QFETCH(QString, path);
    QFETCH(bool, recurse);

    QDir dir;
    if (recurse)
        QVERIFY(dir.rmpath(path));
    else
        QVERIFY(dir.rmdir(path));

    //make sure it really doesn't exist (ie that rmdir returns the right value)
    QFileInfo fi(path);
    QVERIFY(!fi.exists());
}

void tst_QDir::exists_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QDir::currentPath() << true;
    QTest::newRow("data0.1") << QDir::currentPath() + "/" << true;
    QTest::newRow("data1") << QString("/I/Do_not_expect_this_path_to_exist/") << false;
    QTest::newRow("resource0") << QString(":/tst_qdir/") << true;
    QTest::newRow("resource1") << QString(":/I/Do_not_expect_this_resource_to_exist/") << false;

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

    QTest::newRow("This drive should exist") <<  "C:/" << true;
    // find a non-existing drive and check if it does not exist
    QFileInfoList drives = QFSFileEngine::drives();
    QStringList driveLetters;
    for (int i = 0; i < drives.count(); ++i) {
        driveLetters+=drives.at(i).absoluteFilePath();
    }
    char drive = 'Z';
    QString driv;
    do {
        driv = QString::fromAscii("%1:/").arg(drive);
        if (!driveLetters.contains(driv)) break;
        --drive;
    } while (drive >= 'A');
    if (drive >= 'A') {
        QTest::newRow("This drive should not exist") <<  driv << false;
    }
#endif
}

void tst_QDir::exists()
{
    QFETCH(QString, path);
    QFETCH(bool, expected);

    QDir dir(path);
    QCOMPARE(dir.exists(), expected);
}

void tst_QDir::isRelativePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("relative");

    QTest::newRow("data0") << "../somedir" << true;
#if defined(Q_WS_WIN)
    QTest::newRow("data1") << "C:/sOmedir" << false;
#endif
    QTest::newRow("data2") << "somedir" << true;
    QTest::newRow("data3") << "/somedir" << false;
}

void tst_QDir::isRelativePath()
{
    QFETCH(QString, path);
    QFETCH(bool, relative);

    QCOMPARE(QDir::isRelativePath(path),relative);
}


void tst_QDir::QDir_default()
{
    //default constructor QDir();
    QDir dir; // according to documentation should be currentDirPath
    QCOMPARE(dir.absolutePath(), QDir::currentPath());
}

void tst_QDir::compare()
{
    // operator==
    QDir dir;
    dir.makeAbsolute();
    QVERIFY(dir == QDir::currentPath());
}

void tst_QDir::entryList_data()
{
    QTest::addColumn<QString>("dirName"); // relative from current path or abs
    QTest::addColumn<QStringList>("nameFilters");
    QTest::addColumn<int>("filterspec");
    QTest::addColumn<int>("sortspec");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("spaces1") << QDir::currentPath() + "/testdir/spaces" << QStringList("*. bar")
			  << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
			  << QStringList("foo. bar"); // notice how spaces5 works
    QTest::newRow("spaces2") << QDir::currentPath() + "/testdir/spaces" << QStringList("*.bar")
			  << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
			  << QStringList("foo.bar");
    QTest::newRow("spaces3") << QDir::currentPath() + "/testdir/spaces" << QStringList("foo.*")
			  << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
			  << QString("foo. bar,foo.bar").split(',');
    QTest::newRow("files1")  << QDir::currentPath() + "/testdir/dir" << QString("*r.cpp *.pro").split(" ")
			  << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
			  << QString("qdir.pro,qrc_qdir.cpp,tst_qdir.cpp").split(',');
    QTest::newRow("testdir1")  << QDir::currentPath() + "/testdir" << QStringList()
			  << (int)(QDir::AllDirs) << (int)(QDir::NoSort)
			  << QString(".,..,dir,spaces").split(',');
    QTest::newRow("unprintablenames")  << QDir::currentPath() + "/unprintablenames" << QStringList("*")
			  << (int)(QDir::NoFilter) << (int)(QDir::Name)
			  << QString(".,..").split(',');
    QTest::newRow("resources1") << QString(":/tst_qdir/resources/entryList") << QStringList("*.data")
                             << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
                             << QString("file1.data,file2.data,file3.data").split(',');
    QTest::newRow("resources2") << QString(":/tst_qdir/resources/entryList") << QStringList("*.data")
                             << (int)(QDir::Files) << (int)(QDir::NoSort)
                             << QString("file1.data,file2.data,file3.data").split(',');

    QTest::newRow("nofilter") << QString("entrylist/") << QStringList("*")
                              << int(QDir::NoFilter) << int(QDir::Name)
                              << QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(',');
    QTest::newRow("QDir::AllEntries") << QString("entrylist/") << QStringList("*")
                              << int(QDir::AllEntries) << int(QDir::Name)
                              << QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(',');
    QTest::newRow("QDir::Files") << QString("entrylist/") << QStringList("*")
                                 << int(QDir::Files) << int(QDir::Name)
                                 << QString("file,linktofile.lnk,writable").split(',');
    QTest::newRow("QDir::Dirs") << QString("entrylist/") << QStringList("*")
                                << int(QDir::Dirs) << int(QDir::Name)
                                << QString(".,..,directory,linktodirectory.lnk").split(',');
    QTest::newRow("QDir::Dirs | QDir::NoDotAndDotDot") << QString("entrylist/") << QStringList("*")
                                                       << int(QDir::Dirs | QDir::NoDotAndDotDot) << int(QDir::Name)
                                << QString("directory,linktodirectory.lnk").split(',');
    QTest::newRow("QDir::AllDirs") << QString("entrylist/") << QStringList("*")
                                   << int(QDir::AllDirs) << int(QDir::Name)
                                   << QString(".,..,directory,linktodirectory.lnk").split(',');
    QTest::newRow("QDir::AllDirs | QDir::Dirs") << QString("entrylist/") << QStringList("*")
                                                << int(QDir::AllDirs | QDir::Dirs) << int(QDir::Name)
                                                << QString(".,..,directory,linktodirectory.lnk").split(',');
    QTest::newRow("QDir::AllDirs | QDir::Files") << QString("entrylist/") << QStringList("*")
                                                 << int(QDir::AllDirs | QDir::Files) << int(QDir::Name)
                                                 << QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(',');
    QTest::newRow("QDir::AllEntries | QDir::NoSymLinks") << QString("entrylist/") << QStringList("*")
                                      << int(QDir::AllEntries | QDir::NoSymLinks) << int(QDir::Name)
                                      << QString(".,..,directory,file,writable").split(',');
    QTest::newRow("QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot") << QString("entrylist/") << QStringList("*")
                                      << int(QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot) << int(QDir::Name)
                                      << QString("directory,file,writable").split(',');
    QTest::newRow("QDir::Files | QDir::NoSymLinks") << QString("entrylist/") << QStringList("*")
                                                    << int(QDir::Files | QDir::NoSymLinks) << int(QDir::Name)
                                                    << QString("file,writable").split(',');
    QTest::newRow("QDir::Dirs | QDir::NoSymLinks") << QString("entrylist/") << QStringList("*")
                                                   << int(QDir::Dirs | QDir::NoSymLinks) << int(QDir::Name)
                                                   << QString(".,..,directory").split(',');
    QTest::newRow("QDir::Drives | QDir::Files | QDir::NoDotAndDotDot") << QString("entrylist/") << QStringList("*")
                                                   << int(QDir::Drives | QDir::Files | QDir::NoDotAndDotDot) << int(QDir::Name)
                                                   << QString("file,linktofile.lnk,writable").split(',');
    QTest::newRow("QDir::System") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::System) << int(QDir::Name)
                                  << QStringList("brokenlink.lnk");
    QTest::newRow("QDir::Hidden") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::Hidden) << int(QDir::Name)
                                  << QStringList();
    QTest::newRow("QDir::System | QDir::Hidden") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::System | QDir::Hidden) << int(QDir::Name)
                                  << QStringList("brokenlink.lnk");
    QTest::newRow("QDir::AllDirs | QDir::NoSymLinks") << QString("entrylist/") << QStringList("*")
                                                      << int(QDir::AllDirs | QDir::NoSymLinks) << int(QDir::Name)
                                                      << QString(".,..,directory").split(',');
    QTest::newRow("QDir::All | QDir::Hidden | QDir::System") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::All | QDir::Hidden | QDir::System) << int(QDir::Name)
                                  << QString(".,..,brokenlink.lnk,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(',');
    QTest::newRow("QDir::All | QDir::Readable") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::All | QDir::Readable) << int(QDir::Name)
                                                << QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(',');
    QTest::newRow("QDir::All | QDir::Writable") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::All | QDir::Writable) << int(QDir::Name)
                                  << QString(".,..,directory,linktodirectory.lnk,writable").split(',');
    QTest::newRow("Namefilters b*") << QString("entrylist/") << QStringList("d*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << QString("directory").split(',');
    QTest::newRow("Namefilters f*") << QString("entrylist/") << QStringList("f*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << QString("file").split(',');
    QTest::newRow("Namefilters link*") << QString("entrylist/") << QStringList("link*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << QString("linktodirectory.lnk,linktofile.lnk").split(',');
    QTest::newRow("Namefilters *to*") << QString("entrylist/") << QStringList("*to*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << QString("directory,linktodirectory.lnk,linktofile.lnk").split(',');
    QTest::newRow("Sorting QDir::Name") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(',');
    QTest::newRow("Sorting QDir::Name | QDir::Reversed") << QString("entrylist/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Name | QDir::Reversed)
                                  << QString("writable,linktofile.lnk,linktodirectory.lnk,file,directory,..,.").split(',');
    QTest::newRow("Sorting QDir::Type") << QString("types/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type)
                                  << QString(".,..,a,b,c,d,e,f,a.a,b.a,c.a,d.a,e.a,f.a,a.b,b.b,c.b,d.b,e.b,f.b,a.c,b.c,c.c,d.c,e.c,f.c").split(',');
    QTest::newRow("Sorting QDir::Type | QDir::Reversed") << QString("types/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type | QDir::Reversed)
                                  << QString("f.c,e.c,d.c,c.c,b.c,a.c,f.b,e.b,d.b,c.b,b.b,a.b,f.a,e.a,d.a,c.a,b.a,a.a,f,e,d,c,b,a,..,.").split(',');
    QTest::newRow("Sorting QDir::Type | QDir::DirsLast") << QString("types/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type | QDir::DirsLast)
                                  << QString("a,b,c,a.a,b.a,c.a,a.b,b.b,c.b,a.c,b.c,c.c,.,..,d,e,f,d.a,e.a,f.a,d.b,e.b,f.b,d.c,e.c,f.c").split(',');
    QTest::newRow("Sorting QDir::Type | QDir::DirsFirst") << QString("types/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type | QDir::DirsFirst)
                                  << QString(".,..,d,e,f,d.a,e.a,f.a,d.b,e.b,f.b,d.c,e.c,f.c,a,b,c,a.a,b.a,c.a,a.b,b.b,c.b,a.c,b.c,c.c").split(',');
    QTest::newRow("Sorting QDir::Size") << QString("types/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Size | QDir::DirsFirst)
                                  << QString(".,..,d,d.a,d.b,d.c,e,e.a,e.b,e.c,f,f.a,f.b,f.c,c.a,c.b,c.c,b.a,b.c,b.b,a.c,a.b,a.a,a,b,c").split(',');
    QTest::newRow("Sorting QDir::Size | QDir::Reversed") << QString("types/") << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Size | QDir::Reversed | QDir::DirsLast)
                                  << QString("c,b,a,a.a,a.b,a.c,b.b,b.c,b.a,c.c,c.b,c.a,f.c,f.b,f.a,f,e.c,e.b,e.a,e,d.c,d.b,d.a,d,..,.").split(',');
}

void tst_QDir::entryList()
{
    QFETCH(QString, dirName);
    QFETCH(QStringList, nameFilters);
    QFETCH(int, filterspec);
    QFETCH(int, sortspec);
    QFETCH(QStringList, expected);

    QFile("entrylist/writable").open(QIODevice::ReadWrite);
    QFile::remove("entrylist/linktofile");
    QFile::remove("entrylist/linktodirectory");
    QFile::remove("entrylist/linktofile.lnk");
    QFile::remove("entrylist/linktodirectory.lnk");
    QFile::remove("entrylist/brokenlink.lnk");
    QFile::remove("entrylist/brokenlink");
#ifdef Q_OS_WIN
    // ### Sadly, this is a platform difference right now.
    QFile::link("entryList/file", "entrylist/linktofile.lnk");
    QFile::link("entryList/directory", "entrylist/linktodirectory.lnk");
    QFile::link("entryList/nothing", "entrylist/brokenlink.lnk");
#else
    QFile::link("file", "entrylist/linktofile.lnk");
    QFile::link("directory", "entrylist/linktodirectory.lnk");
    QFile::link("nothing", "entrylist/brokenlink.lnk");
#endif

    QDir dir(dirName);
    QVERIFY(dir.exists());

    QStringList actual = dir.entryList(nameFilters, (QDir::Filters)filterspec,
                                       (QDir::SortFlags)sortspec);
    
    int max = qMin(actual.count(), expected.count());

    for (int i=0; i<max; ++i)
        QCOMPARE(actual[i], expected[i]);

    QCOMPARE(actual.count(), expected.count());
}

void tst_QDir::entryListSimple_data()
{
    QTest::addColumn<QString>("dirName");
    QTest::addColumn<int>("countMin");

    QTest::newRow("data2") << "do_not_expect_this_path_to_exist/" << 0;
    QTest::newRow("simple dir") << "resources" << 2;
    QTest::newRow("simple dir with slash") << "resources/" << 2;

#ifdef Q_OS_WIN
    QTest::newRow("unc 1") << "//gennan" << 2;
    QTest::newRow("unc 2") << "//gennan/" << 2;
    QTest::newRow("unc 3") << "//gennan/testshare" << 2;
    QTest::newRow("unc 4") << "//gennan/testshare/" << 2;
    QTest::newRow("unc 5") << "//gennan/testshare/tmp" << 2;
    QTest::newRow("unc 6") << "//gennan/testshare/tmp/" << 2;
    QTest::newRow("unc 7") << "//gennan/testshare/adirthatshouldnotexist" << 0;
    QTest::newRow("unc 8") << "//gennan/asharethatshouldnotexist" << 0;
    QTest::newRow("unc 9") << "//foobar" << 0;
#endif
}

void tst_QDir::entryListSimple()
{
    QFETCH(QString, dirName);
    QFETCH(int, countMin);

    QDir dir(dirName);
    QStringList actual = dir.entryList();
    QVERIFY(actual.count() >= countMin);
}

void tst_QDir::entryListWithSymLinks()
{
    QFile::remove("myLinkToDir.lnk");
    QFile::remove("myLinkToFile.lnk");
    QDir dir;
    dir.mkdir("myDir");
    QVERIFY(QFile::link("myDir", "myLinkToDir.lnk"));
    QVERIFY(QFile::link("tst_qdir.cpp", "myLinkToFile.lnk"));

    {
        QStringList entryList = QDir().entryList();
        QVERIFY(entryList.contains("myDir"));
        QVERIFY(entryList.contains("myLinkToDir.lnk"));
        QVERIFY(entryList.contains("myLinkToFile.lnk"));
    }
    {
        QStringList entryList = QDir().entryList(QDir::Dirs);
        QVERIFY(entryList.contains("myDir"));
        QVERIFY(entryList.contains("myLinkToDir.lnk"));
        QVERIFY(!entryList.contains("myLinkToFile.lnk"));
    }
    {
        QStringList entryList = QDir().entryList(QDir::Dirs | QDir::NoSymLinks);
        QVERIFY(entryList.contains("myDir"));
        QVERIFY(!entryList.contains("myLinkToDir.lnk"));
        QVERIFY(!entryList.contains("myLinkToFile.lnk"));
    }

    QFile::remove("myLinkToDir.lnk");
    QFile::remove("myLinkToFile.lnk");
    dir.rmdir("myDir");
}

void tst_QDir::canonicalPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("canonicalPath");
    QString appPath = QCoreApplication::instance()->applicationDirPath();

#ifdef Q_OS_MAC
    appPath = appPath.left(appPath.lastIndexOf('/', appPath.length() - 16));
#elif defined Q_WS_WIN
    if (appPath.endsWith("release", Qt::CaseInsensitive) || appPath.endsWith("debug", Qt::CaseInsensitive)) {
        QDir appDir(appPath);
        QVERIFY(appDir.cdUp());
        appPath = appDir.absolutePath();
    }
#endif

	QTest::newRow("relative") << "." << appPath;
    QTest::newRow("relativeSubDir") << "./testData/../testData" << appPath + "/testData";

#ifndef Q_WS_WIN
    QTest::newRow("absPath") << appPath + "/testData/../testData" << appPath + "/testData";
#else
    QTest::newRow("absPath") << appPath + "\\testData\\..\\testData" << appPath + "/testData";
#endif
    QTest::newRow("nonexistant") << "testd" << QString();
}

void tst_QDir::canonicalPath()
{
    QDir curPath(".");
    if (curPath.absolutePath() != curPath.canonicalPath())
        QSKIP("This test does not work if this directory path consists of symlinks.", SkipAll);

    QFETCH(QString, path);
    QFETCH(QString, canonicalPath);

	QDir dir(path);
#ifdef Q_OS_WIN
	QCOMPARE(dir.canonicalPath().toLower(), canonicalPath.toLower());
#else
    QCOMPARE(dir.canonicalPath(), canonicalPath);
#endif
}

void tst_QDir::current_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("currentDir");
    QString appPath = QCoreApplication::instance()->applicationDirPath();
#ifdef Q_OS_MAC
    appPath = appPath.left(appPath.lastIndexOf('/', appPath.length() - 16));
#elif defined Q_WS_WIN
	if (appPath.endsWith("release", Qt::CaseInsensitive))
		appPath = appPath.left(appPath.length()-8);
    else if (appPath.endsWith("debug", Qt::CaseInsensitive))
		appPath = appPath.left(appPath.length()-6);
#endif

    QTest::newRow("startup") << QString() << appPath;
    QTest::newRow("relPath") << "testData" << appPath + "/testData";
#ifndef Q_WS_WIN
    QTest::newRow("absPath") << appPath + "/testData" << appPath + "/testData";
#else
    QTest::newRow("absPath") << appPath + "\\testData" << appPath + "/testData";
#endif
    QTest::newRow("nonexistant") << "testd" << QString();
    QTest::newRow("parent") << ".." << appPath;
}

void tst_QDir::current()
{
    QFETCH(QString, path);
    QFETCH(QString, currentDir);

    if (!path.isEmpty()) {
        bool b = QDir::setCurrent(path);
        // If path is non existant, then setCurrent should be false (currentDir is empty in testData)
        QVERIFY(b == !currentDir.isEmpty());
    }
    if (!currentDir.isEmpty()) {
#ifdef Q_OS_WIN
	QCOMPARE(QDir::current().absolutePath().toLower(), currentDir.toLower());
#else
	QCOMPARE(QDir::current().absolutePath(), currentDir);
#endif
    }
}

void tst_QDir::cd_data()
{
    QTest::addColumn<QString>("startDir");
    QTest::addColumn<QString>("cdDir");
    QTest::addColumn<bool>("successExpected");
    QTest::addColumn<QString>("newDir");

    QString appPath = QDir::currentPath();
    QTest::newRow("cdUp") << QDir::currentPath() << ".." << true << appPath.left(appPath.lastIndexOf("/"));
    QTest::newRow("noChange") << QDir::currentPath() << "." << true << appPath;
#ifdef Q_OS_WIN // on windows QDir::root() is usually c:/ but cd "/" will not force it to be root
    QTest::newRow("absolute") << QDir::currentPath() << "/" << true << "/";
#else
    QTest::newRow("absolute") << QDir::currentPath() << "/" << true << QDir::root().absolutePath();
#endif
    QTest::newRow("non existant") << "." << "../anonexistingdir" << false << QDir::currentPath();
    QTest::newRow("self") << "." << (QString("../") + QFileInfo(QDir::currentPath()).fileName()) << true << QDir::currentPath();
#if QT_VERSION > 0x040100
    QTest::newRow("file") << "." << "qdir.pro" << false << "";
#endif
}

void tst_QDir::cd()
{
    QFETCH(QString, startDir);
    QFETCH(QString, cdDir);
    QFETCH(bool, successExpected);
    QFETCH(QString, newDir);

    QDir d = startDir;
    QCOMPARE(d.cd(cdDir), successExpected);
    if (successExpected)
        QCOMPARE(d.absolutePath(), newDir);
}

void tst_QDir::setNameFilters_data()
{
    // Effectively copied from entryList2() test

    QTest::addColumn<QString>("dirName"); // relative from current path or abs
    QTest::addColumn<QStringList>("nameFilters");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("spaces1") << QDir::currentPath() + "/testdir/spaces" << QStringList("*. bar")
                          << QStringList("foo. bar");
    QTest::newRow("spaces2") << QDir::currentPath() + "/testdir/spaces" << QStringList("*.bar")
			              << QStringList("foo.bar");
    QTest::newRow("spaces3") << QDir::currentPath() + "/testdir/spaces" << QStringList("foo.*")
			  			  << QString("foo. bar,foo.bar").split(",");
    QTest::newRow("files1")  << QDir::currentPath()  + "/testdir/dir" << QString("*r.cpp *.pro").split(" ")
						  << QString("qdir.pro,qrc_qdir.cpp,tst_qdir.cpp").split(",");
#if QT_VERSION >= 0x040000
    QTest::newRow("resources1") << QString(":/tst_qdir/resources/entryList") << QStringList("*.data")
                             << QString("file1.data,file2.data,file3.data").split(',');
#endif
}

void tst_QDir::setNameFilters()
{
    QFETCH(QString, dirName);
    QFETCH(QStringList, nameFilters);
    QFETCH(QStringList, expected);

    QDir dir(dirName);
    QVERIFY(dir.exists());

    dir.setNameFilters(nameFilters);
    QStringList actual = dir.entryList();
    int max = qMin(actual.count(), expected.count());

    for (int i=0; i<max; ++i)
        QCOMPARE(actual[i], expected[i]);
    QCOMPARE(actual.count(), expected.count());
}

void
tst_QDir::cleanPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("expected");

    QTest::newRow("data0") << "/Users/sam/troll/qt4.0//.." << "/Users/sam/troll";
    QTest::newRow("data1") << "/Users/sam////troll/qt4.0//.." << "/Users/sam/troll";
    QTest::newRow("data2") << "/" << "/";
    QTest::newRow("data3") << QDir::cleanPath("../.") << "..";
    QTest::newRow("data4") << QDir::cleanPath("../..") << "../..";
#if defined Q_OS_WIN
    QTest::newRow("data5") << "d:\\a\\bc\\def\\.." << "d:/a/bc";
    QTest::newRow("data6") << "d:\\a\\bc\\def\\../../.." << "d:/";
#else
    QTest::newRow("data5") << "d:\\a\\bc\\def\\.." << "d:\\a\\bc\\def\\..";
    QTest::newRow("data6") << "d:\\a\\bc\\def\\../../.." << "d:\\a\\bc\\def\\../../..";
#endif
    QTest::newRow("data7") << ".//file1.txt" << "file1.txt";
    QTest::newRow("data8") << "/foo/bar/..//file1.txt" << "/foo/file1.txt";
    QTest::newRow("data9") << "//" << "/";
}


void
tst_QDir::cleanPath()
{
    QFETCH(QString, path);
    QFETCH(QString, expected);
    QString cleaned = QDir::cleanPath(path);
    QCOMPARE(cleaned, expected);
}

void tst_QDir::absoluteFilePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedFilePath");

    QTest::newRow("0") << "/etc" << "/passwd" << "/passwd";
    QTest::newRow("1") << "/etc" << "passwd" << "/etc/passwd";
    QTest::newRow("2") << "/" << "passwd" << "/passwd";
    QTest::newRow("3") << "relative" << "path" << QDir::currentPath() + "/relative/path";
    QTest::newRow("4") << "" << "" << QDir::currentPath();
}

void tst_QDir::absoluteFilePath()
{
    QFETCH(QString, path);
    QFETCH(QString, fileName);
    QFETCH(QString, expectedFilePath);

    QDir dir(path);
    QString absFilePath = dir.absoluteFilePath(fileName);
    QCOMPARE(absFilePath, expectedFilePath);
}

void tst_QDir::absolutePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("expectedPath");

    QTest::newRow("0") << "/machine/share/dir1" << "/machine/share/dir1";
#ifdef Q_OS_WIN
    QTest::newRow("1") << "\\machine\\share\\dir1" << "/machine/share/dir1";
    QTest::newRow("2") << "//machine/share/dir1" << "//machine/share/dir1";
    QTest::newRow("3") << "\\\\machine\\share\\dir1" << "//machine/share/dir1";
    QTest::newRow("4") << "c:/machine/share/dir1" << "c:/machine/share/dir1";
    QTest::newRow("5") << "c:\\machine\\share\\dir1" << "c:/machine/share/dir1";
#endif
}

void tst_QDir::absolutePath()
{
    QFETCH(QString, path);
    QFETCH(QString, expectedPath);

    QDir dir(path);
    QCOMPARE(dir.absolutePath(), expectedPath);
}

void tst_QDir::relativeFilePath_data()
{
    QTest::addColumn<QString>("dir");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("expected");

    QTest::newRow("0") << "/foo/bar" << "ding.txt" << "ding.txt";
    QTest::newRow("1") << "/foo/bar" << "ding/dong.txt"    << "ding/dong.txt";
    QTest::newRow("2") << "/foo/bar" << "../ding/dong.txt" << "../ding/dong.txt";

    QTest::newRow("3") << "/foo/bar" << "/foo/bar/ding.txt" << "ding.txt";
    QTest::newRow("4") << "/foo/bar/" << "/foo/bar/ding/dong.txt" << "ding/dong.txt";
    QTest::newRow("5") << "/foo/bar/" << "/ding/dong.txt" << "../../ding/dong.txt";

    QTest::newRow("6") << "/" << "/ding/dong.txt" << "ding/dong.txt";
    QTest::newRow("7") << "/" << "/ding/" << "ding";
    QTest::newRow("8") << "/" << "/ding//" << "ding";
    QTest::newRow("9") << "/" << "/ding/../dong" << "dong";
    QTest::newRow("10") << "/" << "/ding/../../../../dong" << "../../../dong";

    QTest::newRow("11") << "" << "" << "";

#ifdef Q_OS_WIN
    QTest::newRow("12") << "C:/foo/bar" << "ding" << "ding";
    QTest::newRow("13") << "C:/foo/bar" << "C:/ding/dong" << "../../ding/dong";
    QTest::newRow("14") << "C:/foo/bar" << "/ding/dong" << "../../ding/dong";
    QTest::newRow("15") << "C:/foo/bar" << "D:/ding/dong" << "D:/ding/dong";
    QTest::newRow("16") << "C:" << "C:/ding/dong" << "ding/dong";
    QTest::newRow("16") << "C:/" << "C:/ding/dong" << "ding/dong";
    QTest::newRow("17") << "C:" << "C:" << "";
    QTest::newRow("18") << "C:/" << "C:" << "";
    QTest::newRow("19") << "C:" << "C:/" << "";
    QTest::newRow("20") << "C:/" << "C:/" << "";
    QTest::newRow("17") << "C:" << "C:file.txt" << "file.txt";
    QTest::newRow("18") << "C:/" << "C:file.txt" << "file.txt";
    QTest::newRow("19") << "C:" << "C:/file.txt" << "file.txt";
    QTest::newRow("20") << "C:/" << "C:/file.txt" << "file.txt";
    QTest::newRow("21") << "C:" << "D:" << "D:";
    QTest::newRow("22") << "C:" << "D:/" << "D:/";
    QTest::newRow("23") << "C:/" << "D:" << "D:";
    QTest::newRow("24") << "C:/" << "D:/" << "D:/";
#endif
}

void tst_QDir::relativeFilePath()
{
    QFETCH(QString, dir);
    QFETCH(QString, path);
    QFETCH(QString, expected);

    QCOMPARE(QDir(dir).relativeFilePath(path), expected);
}

void tst_QDir::filePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedFilePath");

    QTest::newRow("0") << "/etc" << "/passwd" << "/passwd";
    QTest::newRow("1") << "/etc" << "passwd" << "/etc/passwd";
    QTest::newRow("2") << "/" << "passwd" << "/passwd";
    QTest::newRow("3") << "relative" << "path" << "relative/path";
    QTest::newRow("4") << "" << "" << ".";
}

void tst_QDir::filePath()
{
    QFETCH(QString, path);
    QFETCH(QString, fileName);
    QFETCH(QString, expectedFilePath);

    QDir dir(path);
    QString absFilePath = dir.filePath(fileName);
    QCOMPARE(absFilePath, expectedFilePath);
}

void tst_QDir::remove()
{
    QFile f("remove-test");
    f.open(QIODevice::WriteOnly);
    f.close();
    QDir dir;
    QVERIFY(dir.remove("remove-test"));
    QVERIFY(!dir.remove("/remove-test"));
}

void tst_QDir::rename()
{
    QFile f("rename-test");
    f.open(QIODevice::WriteOnly);
    f.close();
    QDir dir;
    QVERIFY(dir.rename("rename-test", "rename-test-renamed"));
    QVERIFY(dir.rename("rename-test-renamed", "rename-test"));
#if defined(Q_OS_MAC)
    QVERIFY(!dir.rename("rename-test", "/etc/rename-test-renamed"));
#elif !defined(Q_OS_WIN) // on windows this is possible maybe make the test a bit better
    QVERIFY(!dir.rename("rename-test", "/rename-test-renamed"));
#endif
    QVERIFY(dir.remove("rename-test"));
}

void tst_QDir::exists2_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("exists");

    QTest::newRow("0") << "." << true;
    QTest::newRow("1") << "/" << true;
    QTest::newRow("2") << "" << false;
    QTest::newRow("3") << "testData" << true;
    QTest::newRow("4") << "/testData" << false;
    QTest::newRow("5") << "tst_qdir.cpp" << true;
    QTest::newRow("6") << "/resources.cpp" << false;
}

void tst_QDir::exists2()
{
    QFETCH(QString, path);
    QFETCH(bool, exists);

    if (path.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, "QDir::exists: Empty or null file name");

    QDir dir;
    if (exists)
        QVERIFY(dir.exists(path));
    else
        QVERIFY(!dir.exists(path));
}

void tst_QDir::dirName_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("dirName");

    QTest::newRow("slash0") << "c:/winnt/system32" << "system32";
    QTest::newRow("slash1") << "/winnt/system32" << "system32";
    QTest::newRow("slash2") << "c:/winnt/system32/kernel32.dll" << "kernel32.dll";
#ifdef Q_OS_WIN
    QTest::newRow("bslash0") << "c:\\winnt\\system32" << "system32";
    QTest::newRow("bslash1") << "\\winnt\\system32" << "system32";
    QTest::newRow("bslash2") << "c:\\winnt\\system32\\kernel32.dll" << "kernel32.dll";
#endif
}

void tst_QDir::dirName()
{
    QFETCH(QString, path);
    QFETCH(QString, dirName);

    QDir dir(path);
    QCOMPARE(dir.dirName(), dirName);
}

void tst_QDir::operator_eq()
{
    QDir dir1(".");
    dir1 = dir1;
    dir1.setPath("..");
}

void tst_QDir::dotAndDotDot()
{
#if QT_VERSION < 0x040100
    QSKIP("The DotAndDotDot flag was introduced in 4.1", SkipAll);
#else
    QDir dir(QString("testdir/"));
    QStringList entryList = dir.entryList(QDir::Dirs);
    QCOMPARE(entryList, QStringList() << QString(".") << QString("..") << QString("dir") << QString("spaces"));
    entryList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCOMPARE(entryList, QStringList() << QString("dir") << QString("spaces"));
#endif
}

#ifdef QT3_SUPPORT
/*
    Tets that the setMatchAllDirs setting survies a call to setFilter.
*/
void tst_QDir::matchAllDirs()
{
    QDir dir("/");
    dir.setMatchAllDirs(true);
    dir.setNameFilters(QStringList() << "*.foo");
    dir.setFilter(QDir::Hidden);
    QVERIFY(dir.matchAllDirs());
    QVERIFY(dir.entryList().count() > 0);
    dir.setMatchAllDirs(false);
    dir.setFilter(QDir::Hidden);
    QVERIFY(dir.matchAllDirs() == false);
    QCOMPARE(dir.entryList().count(), 0);

}
#endif

void tst_QDir::homePath()
{
	QString strHome = QDir::homePath();

	QDir homeDir(strHome);
	QStringList entries = homeDir.entryList();
	for (int i = 0; i < entries.count(); ++i) {
		QFileInfo fi(QDir::homePath() + "/" + entries[i]);
		//qDebug() << fi.absoluteFilePath() << (fi.exists() ? "exists" : "not exists");
		QCOMPARE( fi.exists(), true);
	}
}

void tst_QDir::nativeSeparators()
{
#ifdef Q_OS_WIN
    QCOMPARE(QDir::toNativeSeparators(QLatin1String("/")), QString("\\"));
    QCOMPARE(QDir::toNativeSeparators(QLatin1String("\\")), QString("\\"));
    QCOMPARE(QDir::fromNativeSeparators(QLatin1String("/")), QString("/"));
    QCOMPARE(QDir::fromNativeSeparators(QLatin1String("\\")), QString("/"));
#else
    QCOMPARE(QDir::toNativeSeparators(QLatin1String("/")), QString("/"));
    QCOMPARE(QDir::toNativeSeparators(QLatin1String("\\")), QString("\\"));
    QCOMPARE(QDir::fromNativeSeparators(QLatin1String("/")), QString("/"));
    QCOMPARE(QDir::fromNativeSeparators(QLatin1String("\\")), QString("\\"));
#endif
}

QTEST_MAIN(tst_QDir)
#include "tst_qdir.moc"
