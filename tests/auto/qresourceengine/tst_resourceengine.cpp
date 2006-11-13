/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore>

class tst_ResourceEngine: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void checkUnregisterResource_data();
    void checkUnregisterResource();
    void checkStructure_data();
    void checkStructure();
    void searchPath_data();
    void searchPath();
};

Q_DECLARE_METATYPE(QLocale)
Q_DECLARE_METATYPE(qlonglong)

void tst_ResourceEngine::initTestCase()
{
    QVERIFY(QResource::registerResource("runtime_resource.rcc"));
    QVERIFY(QResource::registerResource("runtime_resource.rcc", "/secondary_root/"));
}

void tst_ResourceEngine::cleanupTestCase()
{
    // make sure we don't leak memory
    QVERIFY(QResource::unregisterResource("runtime_resource.rcc"));
    QVERIFY(QResource::unregisterResource("runtime_resource.rcc", "/secondary_root/"));
}

void tst_ResourceEngine::checkStructure_data()
{
    QTest::addColumn<QString>("pathName");
    QTest::addColumn<QString>("contents");
    QTest::addColumn<QStringList>("containedFiles");
    QTest::addColumn<QStringList>("containedDirs");
    QTest::addColumn<QLocale>("locale");
    QTest::addColumn<qlonglong>("contentsSize");

    QFileInfo info;

    QTest::newRow("root dir")          << QString(":/")
                                       << QString()
                                       << (QStringList() << "search_file.txt")
                                       << (QStringList() << QLatin1String("aliasdir") << QLatin1String("otherdir")
                                           << QLatin1String("runtime_resource")
                                           << QLatin1String("searchpath1") << QLatin1String("searchpath2")
                                           << QLatin1String("secondary_root")
                                           << QLatin1String("test") << QLatin1String("trolltech")
                                           << QLatin1String("withoutslashes"))
                                       << QLocale()
                                       << qlonglong(0);

    QTest::newRow("secondary root")  << QString(":/secondary_root/")
                                     << QString()
                                     << QStringList()
                                     << (QStringList() << QLatin1String("runtime_resource"))
                                     << QLocale()
                                     << qlonglong(0);

    QStringList roots;
    roots << QString(":/") << QString(":/runtime_resource/") << QString(":/secondary_root/runtime_resource/");
    for(int i = 0; i < roots.size(); ++i) {
        const QString root = roots.at(i);

        QTest::newRow(QString(root + "prefix dir").toLatin1().constData())  << QString(root + "test/abc/123/+++")
                                            << QString()
                                            << (QStringList() << QLatin1String("currentdir.txt") << QLatin1String("currentdir2.txt") << QLatin1String("parentdir.txt"))
                                            << (QStringList() << QLatin1String("subdir"))
                                            << QLocale()
                                            << qlonglong(0);

        QTest::newRow(QString(root + "parent to prefix").toLatin1().constData())  << QString(root + "test/abc/123")
                                                  << QString()
                                                  << QStringList()
                                                  << (QStringList() << QLatin1String("+++"))
                                                  << QLocale()
                                                  << qlonglong(0);

        QTest::newRow(QString(root + "two parents prefix").toLatin1().constData()) << QString(root + "test/abc")
                                                   << QString()
                                                   << QStringList()
                                                   << QStringList(QLatin1String("123"))
                                                   << QLocale()
                                                   << qlonglong(0);

        QTest::newRow(QString(root + "test dir ").toLatin1().constData())          << QString(root + "test")
                                                   << QString()
                                                   << (QStringList() << QLatin1String("testdir.txt"))
                                                   << (QStringList() << QLatin1String("abc") << QLatin1String("test"))
                                                   << QLocale()
                                                   << qlonglong(0);

        QTest::newRow(QString(root + "prefix no slashes").toLatin1().constData()) << QString(root + "withoutslashes")
                                                  << QString()
                                                  << QStringList("blahblah.txt")
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(0);

        QTest::newRow(QString(root + "other dir").toLatin1().constData())         << QString(root + "otherdir")
                                                  << QString()
                                                  << QStringList(QLatin1String("otherdir.txt"))
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(0);

        QTest::newRow(QString(root + "alias dir").toLatin1().constData())         << QString(root + "aliasdir")
                                                  << QString()
                                                  << QStringList(QLatin1String("aliasdir.txt"))
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(0);

        QTest::newRow(QString(root + "second test dir").toLatin1().constData())   << QString(root + "test/test")
                                                  << QString()
                                                  << (QStringList() << QLatin1String("test1.txt") << QLatin1String("test2.txt"))
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(0);

        info = QFileInfo("testqrc/test/test/test1.txt");
        QTest::newRow(QString(root + "test1 text").toLatin1().constData())        << QString(root + "test/test/test1.txt")
                                                  << QString("abc")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/blahblah.txt");
        QTest::newRow(QString(root + "text no slashes").toLatin1().constData())   << QString(root + "withoutslashes/blahblah.txt")
                                                  << QString("qwerty")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());


        info = QFileInfo("testqrc/test/test/test2.txt");
        QTest::newRow(QString(root + "test1 text").toLatin1().constData())        << QString(root + "test/test/test2.txt")
                                                  << QString("def")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/currentdir.txt");
        QTest::newRow(QString(root + "currentdir text").toLatin1().constData())   << QString(root + "test/abc/123/+++/currentdir.txt")
                                                  << QString("\"This is the current dir\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/currentdir2.txt");
        QTest::newRow(QString(root + "currentdir text2").toLatin1().constData())  << QString(root + "test/abc/123/+++/currentdir2.txt")
                                                  << QString("\"This is also the current dir\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("parentdir.txt");
        QTest::newRow(QString(root + "parentdir text").toLatin1().constData())    << QString(root + "test/abc/123/+++/parentdir.txt")
                                                  << QString("abcdefgihklmnopqrstuvwxyz ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/subdir/subdir.txt");
        QTest::newRow(QString(root + "subdir text").toLatin1().constData())       << QString(root + "test/abc/123/+++/subdir/subdir.txt")
                                                  << QString("\"This is in the sub directory\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/test/testdir.txt");
        QTest::newRow(QString(root + "testdir text").toLatin1().constData())      << QString(root + "test/testdir.txt")
                                                  << QString("\"This is in the test directory\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/otherdir/otherdir.txt");
        QTest::newRow(QString(root + "otherdir text").toLatin1().constData())     << QString(root + "otherdir/otherdir.txt")
                                                  << QString("\"This is the other dir\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/test/testdir2.txt");
        QTest::newRow(QString(root + "alias text").toLatin1().constData())        << QString(root + "aliasdir/aliasdir.txt")
                                                  << QString("\"This is another file in this directory\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale()
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/aliasdir/aliasdir.txt");
        QTest::newRow(QString(root + "korean text").toLatin1().constData())       << QString(root + "aliasdir/aliasdir.txt")
                                                  << QString("\"This is a korean text file\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale("ko")
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/aliasdir/aliasdir.txt");
        QTest::newRow(QString(root + "korean text 2").toLatin1().constData())     << QString(root + "aliasdir/aliasdir.txt")
                                                  << QString("\"This is a korean text file\" ")
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale("ko_KR")
                                                  << qlonglong(info.size());

        info = QFileInfo("testqrc/test/german.txt");
        QTest::newRow(QString(root + "german text").toLatin1().constData())   << QString(root + "aliasdir/aliasdir.txt")
                                              << QString("Deutsch")
                                              << QStringList()
                                              << QStringList()
                                              << QLocale("de")
                                              << qlonglong(info.size());

        info = QFileInfo("testqrc/test/german.txt");
        QTest::newRow(QString(root + "german text 2").toLatin1().constData())   << QString(root + "aliasdir/aliasdir.txt")
                                                << QString("Deutsch")
                                                << QStringList()
                                                << QStringList()
                                                << QLocale("de_DE")
                                                << qlonglong(info.size());

        QFile file("testqrc/aliasdir/compressme.txt");
        file.open(QFile::ReadOnly);
        info = QFileInfo("testqrc/aliasdir/compressme.txt");
        QTest::newRow(QString(root + "compressed text").toLatin1().constData())   << QString(root + "aliasdir/aliasdir.txt")
                                                  << QString(file.readAll())
                                                  << QStringList()
                                                  << QStringList()
                                                  << QLocale("de_CH")
                                                  << qlonglong(info.size());
    }
}

void tst_ResourceEngine::checkStructure()
{
    QFETCH(QString, pathName);
    QFETCH(QString, contents);
    QFETCH(QStringList, containedFiles);
    QFETCH(QStringList, containedDirs);
    QFETCH(QLocale, locale);
    QFETCH(qlonglong, contentsSize);

    bool directory = (containedDirs.size() + containedFiles.size() > 0);
    QLocale::setDefault(locale);

    QFileInfo fileInfo(pathName);

    QVERIFY(fileInfo.exists());
    QCOMPARE(fileInfo.isDir(), directory);
    QCOMPARE(fileInfo.size(), contentsSize);
    //QVERIFY(fileInfo.isReadable());
    QVERIFY(!fileInfo.isWritable());
    QVERIFY(!fileInfo.isExecutable());

    if (directory) {
        QDir dir(pathName);

        // Test the Dir filter
        QFileInfoList list = dir.entryInfoList(QDir::Dirs, QDir::Name);
//           for(int i = 0; i < list.size(); ++i)
//               qDebug() << "one" << i << list.at(i).fileName();
//           for(int i = 0; i < containedDirs.size(); ++i)
//               qDebug() << "two" << i << containedDirs.at(i);
//           qDebug() << "one" << list.size() << containedDirs.size();
       QCOMPARE(list.size(), containedDirs.size());
//         qDebug() << "two";

        int i;
        for (i=0; i<list.size(); ++i) {
            QVERIFY(list.at(i).isDir());
            QCOMPARE(list.at(i).fileName(), containedDirs.at(i));
        }

        list = dir.entryInfoList(QDir::Files, QDir::Name);
        QCOMPARE(containedFiles.size(), list.size());

        for (i=0; i<list.size(); ++i) {
            QVERIFY(!list.at(i).isDir());
            QCOMPARE(list.at(i).fileName(), containedFiles.at(i));
        }

        list = dir.entryInfoList(QDir::NoFilter, QDir::SortFlags(QDir::Name | QDir::DirsFirst));
        QCOMPARE(containedFiles.size() + containedDirs.size(), list.size());

        for (i=0; i<list.size(); ++i) {
            QString expectedName;
            if (i < containedDirs.size())
                expectedName = containedDirs.at(i);
            else
                expectedName = containedFiles.at(i - containedDirs.size());

            QCOMPARE(list.at(i).fileName(), expectedName);
        }
    } else {
        QFile file(pathName);
        QVERIFY(file.open(QFile::ReadOnly));

        QByteArray ba = file.readAll();
        QVERIFY(QString(ba).startsWith(contents));
    }
    QLocale::setDefault(QLocale::system());
}

void tst_ResourceEngine::searchPath_data()
{
    QTest::addColumn<QString>("searchPath");
    QTest::addColumn<QString>("file");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("no_search_path")  << QString()
                                  << ":search_file.txt"
                                  << QByteArray("root\n");
    QTest::newRow("path1")  << "/searchpath1"
                         << ":search_file.txt"
                         << QByteArray("path1\n");
    QTest::newRow("no_search_path2")  << QString()
                                  << ":/search_file.txt"
                                  << QByteArray("root\n");
    QTest::newRow("path2")  << "/searchpath2"
                         << ":search_file.txt"
                         << QByteArray("path2\n");
}

void tst_ResourceEngine::searchPath()
{
    QFETCH(QString, searchPath);
    QFETCH(QString, file);
    QFETCH(QByteArray, expected);

    if(!searchPath.isEmpty())
        QDir::addResourceSearchPath(searchPath);
    QFile qf(file);
    QVERIFY(qf.open(QFile::ReadOnly));
    QByteArray actual = qf.readAll();

    actual.replace('\r', "");

    QCOMPARE(actual, expected);
    qf.close();
}

void tst_ResourceEngine::checkUnregisterResource_data()
{
    QTest::addColumn<QString>("rcc_file");
    QTest::addColumn<QString>("root");
    QTest::addColumn<QString>("file_check");
    QTest::addColumn<int>("size");

    QTest::newRow("currentdir.txt") << QString("runtime_resource.rcc") << QString("/check_unregister/")
                                    << QString(":/check_unregister/runtime_resource/test/abc/123/+++/currentdir.txt") << 27;
}

void tst_ResourceEngine::checkUnregisterResource()
{
    QFETCH(QString, rcc_file);
    QFETCH(QString, root);
    QFETCH(QString, file_check);
    QFETCH(int, size);



    QVERIFY(!QFile::exists(file_check));
    QVERIFY(QResource::registerResource(rcc_file, root));
    QVERIFY(QFile::exists(file_check));
    QVERIFY(QResource::unregisterResource(rcc_file, root));
    QVERIFY(!QFile::exists(file_check));
    QVERIFY(QResource::registerResource(rcc_file, root));
    QVERIFY(QFile::exists(file_check));
    QFileInfo fileInfo(file_check);
    fileInfo.setCaching(false);
    QVERIFY(fileInfo.exists());
    QVERIFY(!QResource::unregisterResource(rcc_file, root));
    QVERIFY(!QFile::exists(file_check));
    QCOMPARE((int)fileInfo.size(), size);
}

QTEST_MAIN(tst_ResourceEngine)

#include "tst_resourceengine.moc"
