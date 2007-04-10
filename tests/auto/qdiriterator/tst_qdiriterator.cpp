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
#include <qdiriterator.h>
#include <qfileinfo.h>
#include <qstringlist.h>

Q_DECLARE_METATYPE(QDirIterator::IteratorFlags)
Q_DECLARE_METATYPE(QDir::Filters)

//TESTED_CLASS=
//TESTED_FILES=corelib/io/qdiriterator.h corelib/io/qdiriterator.cpp

class tst_QDirIterator : public QObject
{
    Q_OBJECT

public:
    tst_QDirIterator();
    virtual ~tst_QDirIterator();

private slots:
    void iterateRelativeDirectory_data();
    void iterateRelativeDirectory();
    void iterateResource_data();
    void iterateResource();
    void stopLinkLoop();
    void engineWithNoIterator();
    void absoluteFilePathsFromRelativeIteratorPath();
    void recurseWithFilters() const;
};

tst_QDirIterator::tst_QDirIterator()
{
    QFile::remove("entrylist/entrylist1.lnk");
    QFile::remove("entrylist/entrylist2.lnk");
    QFile::remove("entrylist/entrylist3.lnk");
    QFile::remove("entrylist/entrylist4.lnk");
    QFile::remove("entrylist/directory/entrylist1.lnk");
    QFile::remove("entrylist/directory/entrylist2.lnk");
    QFile::remove("entrylist/directory/entrylist3.lnk");
    QFile::remove("entrylist/directory/entrylist4.lnk");

#ifdef Q_OS_WIN
    // ### Sadly, this is a platform difference right now.
    QFile::link("entrylist/file", "entrylist/linktofile.lnk");
    QFile::link("entrylist/directory", "entrylist/linktodirectory.lnk");
    QFile::link("entrylist/nothing", "entrylist/brokenlink.lnk");
#else
    QFile::link("file", "entrylist/linktofile.lnk");
    QFile::link("directory", "entrylist/linktodirectory.lnk");
    QFile::link("nothing", "entrylist/brokenlink.lnk");
#endif
    QFile("entrylist/writable").open(QIODevice::ReadWrite);
}

tst_QDirIterator::~tst_QDirIterator()
{
    QFile::remove("entrylist/directory");
    QFile::remove("entrylist/linktofile.lnk");
    QFile::remove("entrylist/linktodirectory.lnk");
    QFile::remove("entrylist/brokenlink.lnk");
    QFile::remove("entrylist/brokenlink");
    QFile::remove("entrylist/writable");
}

void tst_QDirIterator::iterateRelativeDirectory_data()
{
    QTest::addColumn<QString>("dirName"); // relative from current path or abs
    QTest::addColumn<QDirIterator::IteratorFlags>("flags");
    QTest::addColumn<QDir::Filters>("filters");
    QTest::addColumn<QStringList>("nameFilters");
    QTest::addColumn<QStringList>("entries");
    
    QTest::newRow("no flags")
        << QString("entrylist") << QDirIterator::IteratorFlags(0)
        << QDir::Filters(QDir::NoFilter) << QStringList("*")
        << QString("entrylist/.,"
                   "entrylist/..,"
                   "entrylist/file,"
                   "entrylist/linktofile.lnk,"
                   "entrylist/directory,"
                   "entrylist/linktodirectory.lnk,"
                   "entrylist/writable").split(',');

    QTest::newRow("QDir::Subdirectories | QDir::FollowSymlinks")
        << QString("entrylist") << QDirIterator::IteratorFlags(QDirIterator::Subdirectories | QDirIterator::FollowSymlinks)
        << QDir::Filters(QDir::NoFilter) << QStringList("*")
        << QString("entrylist/.,"
                   "entrylist/..,"
                   "entrylist/file,"
                   "entrylist/linktofile.lnk,"
                   "entrylist/directory,"
                   "entrylist/directory/.,"
                   "entrylist/directory/..,"
                   "entrylist/directory/dummy,"
                   "entrylist/linktodirectory.lnk,"
                   "entrylist/writable").split(',');

    QTest::newRow("QDir::Subdirectories / QDir::Files")
        << QString("entrylist") << QDirIterator::IteratorFlags(QDirIterator::Subdirectories)
        << QDir::Filters(QDir::Files) << QStringList("*")
        << QString("entrylist/file,"
                   "entrylist/linktofile.lnk,"
                   "entrylist/writable").split(',');

    QTest::newRow("QDir::Subdirectories | QDir::FollowSymlinks / QDir::Files")
        << QString("entrylist") << QDirIterator::IteratorFlags(QDirIterator::Subdirectories | QDirIterator::FollowSymlinks)
        << QDir::Filters(QDir::Files) << QStringList("*")
        << QString("entrylist/file,"
                   "entrylist/linkotfile.lnk,"
                   "entrylist/directory/dummy,"
                   "entrylist/writable").split(',');
}

void tst_QDirIterator::iterateRelativeDirectory()
{
    QFETCH(QString, dirName);
    QFETCH(QDirIterator::IteratorFlags, flags);
    QFETCH(QDir::Filters, filters);
    QFETCH(QStringList, nameFilters);
    QFETCH(QStringList, entries);

    QDirIterator it(dirName, nameFilters, filters, flags);
    QStringList list;
    while (it.hasNext())
        list << it.next(); 

    list.sort();
    QStringList sortedEntries = entries;
    sortedEntries.sort();

    if (sortedEntries != list) {
        qDebug() << "EXPECTED:" << sortedEntries;
        qDebug() << "ACTUAL:" << list;
    }

    QEXPECT_FAIL("QDir::Subdirectories | QDir::FollowSymlinks / QDir::Files", "Unsupported so far", Continue);
    QCOMPARE(list, sortedEntries);
}

void tst_QDirIterator::iterateResource_data()
{
    QTest::addColumn<QString>("dirName"); // relative from current path or abs
    QTest::addColumn<QDirIterator::IteratorFlags>("flags");
    QTest::addColumn<QDir::Filters>("filters");
    QTest::addColumn<QStringList>("nameFilters");
    QTest::addColumn<QStringList>("entries");

    QTest::newRow("invalid") << QString::fromLatin1(":/burpaburpa") << QDirIterator::IteratorFlags(0)
                             << QDir::Filters(QDir::NoFilter) << QStringList(QLatin1String("*"))
                             << QStringList();
    QTest::newRow(":/") << QString::fromLatin1(":/") << QDirIterator::IteratorFlags(0)
                               << QDir::Filters(QDir::NoFilter) << QStringList(QLatin1String("*"))
                               << QString::fromLatin1(":/entrylist").split(QLatin1String(","));
    QTest::newRow(":/entrylist") << QString::fromLatin1(":/entrylist") << QDirIterator::IteratorFlags(0)
                               << QDir::Filters(QDir::NoFilter) << QStringList(QLatin1String("*"))
                               << QString::fromLatin1(":/entrylist/directory,:/entrylist/file").split(QLatin1String(","));
    QTest::newRow(":/ recursive") << QString::fromLatin1(":/") << QDirIterator::IteratorFlags(QDirIterator::Subdirectories)
                                         << QDir::Filters(QDir::NoFilter) << QStringList(QLatin1String("*"))
                                         << QString::fromLatin1(":/entrylist,:/entrylist/directory,:/entrylist/directory/dummy,:/entrylist/file").split(QLatin1String(","));
}

void tst_QDirIterator::iterateResource()
{
    QFETCH(QString, dirName);
    QFETCH(QDirIterator::IteratorFlags, flags);
    QFETCH(QDir::Filters, filters);
    QFETCH(QStringList, nameFilters);
    QFETCH(QStringList, entries);

    QDirIterator it(dirName, nameFilters, filters, flags);
    QStringList list;
    while (it.hasNext())
        list << it.next();

    list.sort();
    QStringList sortedEntries = entries;
    sortedEntries.sort();

    if (sortedEntries != list) {
        qDebug() << "EXPECTED:" << sortedEntries;
        qDebug() << "ACTUAL:" << list;
    }

    QCOMPARE(list, sortedEntries);
}

void tst_QDirIterator::stopLinkLoop()
{
#ifdef Q_OS_WIN
    // ### Sadly, this is a platform difference right now.
    QFile::link(QDir::currentPath() + QLatin1String("/entrylist"), "entrylist/entrylist1.lnk");
    QFile::link("entrylist/.", "entrylist/entrylist2.lnk");
    QFile::link("entrylist/../entrylist/.", "entrylist/entrylist3.lnk");
    QFile::link("entrylist/..", "entrylist/entrylist4.lnk");
    QFile::link(QDir::currentPath() + QLatin1String("/entrylist"), "entrylist/directory/entrylist1.lnk");
    QFile::link("entrylist/.", "entrylist/directory/entrylist2.lnk");
    QFile::link("entrylist/../directory/.", "entrylist/directory/entrylist3.lnk");
    QFile::link("entrylist/..", "entrylist/directory/entrylist4.lnk");
#else
    QFile::link(QDir::currentPath() + QLatin1String("/entrylist"), "entrylist/entrylist1.lnk");
    QFile::link(".", "entrylist/entrylist2.lnk");
    QFile::link("../entrylist/.", "entrylist/entrylist3.lnk");
    QFile::link("..", "entrylist/entrylist4.lnk");
    QFile::link(QDir::currentPath() + QLatin1String("/entrylist"), "entrylist/directory/entrylist1.lnk");
    QFile::link(".", "entrylist/directory/entrylist2.lnk");
    QFile::link("../directory/.", "entrylist/directory/entrylist3.lnk");
    QFile::link("..", "entrylist/directory/entrylist4.lnk");
#endif

    QDirIterator it(QLatin1String("entrylist"), QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    QStringList list;
    int max = 200;
    while (--max && it.hasNext())
        it.next();
    QVERIFY(max);
    
    // The goal of this test is only to ensure that the test above doesn't go
    // haywire.

#ifdef Q_OS_WIN
    // ### Sadly, this is a platform difference right now.
    QFile::link(QDir::currentPath() + QLatin1String("/entrylist"), "entrylist/entrylist1.lnk");
    QFile::link("../../entrylist/.", "entrylist/entrylist2.lnk");
    QFile::link("entrylist/..", "entrylist/entrylist3.lnk");
#else
    QFile::remove("entrylist/entrylist1.lnk");
    QFile::remove("entrylist/entrylist2.lnk");
    QFile::remove("entrylist/entrylist3.lnk");
    QFile::remove("entrylist/entrylist4.lnk");
    QFile::remove("entrylist/directory/entrylist1.lnk");
    QFile::remove("entrylist/directory/entrylist2.lnk");
    QFile::remove("entrylist/directory/entrylist3.lnk");
    QFile::remove("entrylist/directory/entrylist4.lnk");
#endif
}

class EngineWithNoIterator : public QFSFileEngine
{
public:
    EngineWithNoIterator(const QString &fileName)
        : QFSFileEngine(fileName)
    { }

    QAbstractFileEngineIterator *beginEntryList(QDir::Filters, const QStringList &)
    { return 0; }
};

class EngineWithNoIteratorHandler : public QAbstractFileEngineHandler
{
public:
    QAbstractFileEngine *create(const QString &fileName) const
    {
        return new EngineWithNoIterator(fileName);
    }
};

void tst_QDirIterator::engineWithNoIterator()
{
    EngineWithNoIteratorHandler handler;

    QDir("entrylist").entryList();
    QVERIFY(true); // test that the above line doesn't crash
}

void tst_QDirIterator::absoluteFilePathsFromRelativeIteratorPath()
{
    QDirIterator it("entrylist/", QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        it.next();
        QVERIFY(QFileInfo(it.filePath()).absoluteFilePath().contains("entrylist"));
    }
}

void tst_QDirIterator::recurseWithFilters() const
{
    QStringList nameFilters;
    nameFilters.append("*.txt");

    QDirIterator it("recursiveDirs/", nameFilters, QDir::Files,
                    QDirIterator::Subdirectories);

    QVERIFY(it.hasNext());
    it.next();
    QCOMPARE(it.fileInfo().filePath(), QString::fromLatin1("recursiveDirs/textFileA.txt"));

    QEXPECT_FAIL("", "This is a known bug.", Abort);
    QVERIFY(it.hasNext());
    it.next();
    QCOMPARE(it.fileInfo().filePath(), QString::fromLatin1("recursiveDirs/dir1/textFileB.txt"));

    QVERIFY(!it.hasNext());
}

QTEST_MAIN(tst_QDirIterator)

#include "tst_qdiriterator.moc"

