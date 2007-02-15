#include <QtTest/QtTest>

#include <QCoreApplication>

#if QT_VERSION < 0x040200
QTEST_NOOP_MAIN
#else

#include <QFileSystemWatcher>

#ifdef Q_OS_LINUX
# ifdef QT_NO_INOTIFY
#  include <linux/version.h>
# else
#  include <sys/inotify.h>
# endif
#endif

class tst_QFileSystemWatcher : public QObject
{
    Q_OBJECT

public:
    tst_QFileSystemWatcher();

private slots:
    void basicTest_data();
    void basicTest();

    void watchDirectory_data() { basicTest_data(); }
    void watchDirectory();

    void addPath();
    void removePath();
    void addPaths();
    void removePaths();

private:
    bool do_force_native;
};

tst_QFileSystemWatcher::tst_QFileSystemWatcher()
    : do_force_native(false)
{
#ifdef Q_OS_LINUX
    // the inotify implementation in the kernel is known to be buggy in certain versions of the linux kernel
    do_force_native = true;
#ifdef QT_NO_INOTIFY
    if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13))
        do_force_native = false;
#else
    if (inotify_init() == -1)
        do_force_native = false;
#endif
#elif defined(Q_OS_WIN) || defined(Q_OS_DARWIN) || defined(Q_OS_FREEBSD)
    // we have native engines for win32, macosx and freebsd
    do_force_native = true;
#endif
}

void tst_QFileSystemWatcher::basicTest_data()
{
    QTest::addColumn<QString>("backend");
    if (do_force_native)
        QTest::newRow("native") << "native";
    QTest::newRow("poller") << "poller";
}

void tst_QFileSystemWatcher::basicTest()
{
    QFETCH(QString, backend);
    qDebug() << "Testing" << backend << "engine";

    // create test file
    QFile testFile("testfile.txt");
    testFile.remove();
    QVERIFY(testFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    testFile.write(QByteArray("hello"));
    testFile.close();

    // set some file permissions
    testFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadOther);

    // create watcher, forcing it to use a specific backend
    QFileSystemWatcher watcher;
    watcher.setObjectName(QLatin1String("_qt_autotest_force_engine_") + backend);
    watcher.addPath(testFile.fileName());

    QSignalSpy changedSpy(&watcher, SIGNAL(fileChanged(const QString &)));
    QEventLoop eventLoop;
    connect(&watcher,
            SIGNAL(fileChanged(const QString &)),
            &eventLoop,
            SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    // modify the file, should get a signal from the watcher

    // resolution of the modification time is system dependent, but it's at most 1 second when using
    // the polling engine. I've heard rumors that FAT32 has a 2 second resolution. So, we have to
    // wait a bit before we can modify the file (hrmph)...
    QTest::qWait(2000);

    testFile.open(QIODevice::WriteOnly | QIODevice::Append);
    testFile.write(QByteArray("world"));
    testFile.close();

    // qDebug() << "waiting max 5 seconds for notification for file modification to trigger(1)";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0).count(), 1);

    QString fileName = changedSpy.at(0).at(0).toString();
    QCOMPARE(fileName, testFile.fileName());

    changedSpy.clear();

    // remove the watch and modify the file, should not get a signal from the watcher
    watcher.removePath(testFile.fileName());
    testFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    testFile.write(QByteArray("hello universe!"));
    testFile.close();

    // qDebug() << "waiting max 5 seconds for notification for file modification to trigger (2)";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 0);

    // readd the file watch
    watcher.addPath(testFile.fileName());

    // change the permissions, should get a signal from the watcher
    testFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner);

    // qDebug() << "waiting max 5 seconds for notification for file permission modification to trigger(1)";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0).count(), 1);

    fileName = changedSpy.at(0).at(0).toString();
    QCOMPARE(fileName, testFile.fileName());

    changedSpy.clear();

    // remove the watch and modify file permissions, should not get a signal from the watcher
    watcher.removePath(testFile.fileName());
    testFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOther);

    // qDebug() << "waiting max 5 seconds for notification for file modification to trigger (2)";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 0);

    // readd the file watch
    watcher.addPath(testFile.fileName());

    // remove the file, should get a signal from the watcher
    QVERIFY(testFile.remove());

    // qDebug() << "waiting max 5 seconds for notification for file removal to trigger";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0).count(), 1);

    fileName = changedSpy.at(0).at(0).toString();
    QCOMPARE(fileName, testFile.fileName());

    changedSpy.clear();

    // recreate the file, we should not get any notification
    QVERIFY(testFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    testFile.write(QByteArray("hello"));
    testFile.close();

    // qDebug() << "waiting max 5 seconds for notification for file recreation to trigger";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 0);

    QVERIFY(testFile.remove());
}

void tst_QFileSystemWatcher::watchDirectory()
{
    QFETCH(QString, backend);
    qDebug() << "Testing" << backend << "engine";

    QDir().mkdir("testDir");
    QDir testDir("testDir");

    QString testFileName = testDir.filePath("testFile.txt");
    QFile::remove(testFileName);

    QFileSystemWatcher watcher;
    watcher.setObjectName(QLatin1String("_qt_autotest_force_engine_") + backend);
    watcher.addPath(testDir.dirName());

    QSignalSpy changedSpy(&watcher, SIGNAL(directoryChanged(const QString &)));
    QEventLoop eventLoop;
    connect(&watcher,
            SIGNAL(directoryChanged(const QString &)),
            &eventLoop,
            SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

    // resolution of the modification time is system dependent, but it's at most 1 second when using
    // the polling engine. I've heard rumors that FAT32 has a 2 second resolution. So we have to
    // wait before modifying the directory (hrmph)...
    QTest::qWait(2000);
    QFile testFile(testFileName);
    QString fileName;

    // remove the watch, should not get notification of a new file
    watcher.removePath(testDir.dirName());
    QVERIFY(testFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    testFile.close();

    // qDebug() << "waiting max 5 seconds for notification for file recreationg to trigger";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 0);

    watcher.addPath(testDir.dirName());

    // remove the file again, should get a signal from the watcher
    QVERIFY(testFile.remove());

    timer.start(5000);
    eventLoop.exec();

    // remove the directory, should get a signal from the watcher
    QVERIFY(QDir().rmdir("testDir"));

    // qDebug() << "waiting max 5 seconds for notification for directory removal to trigger";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 2);
    QCOMPARE(changedSpy.at(0).count(), 1);
    QCOMPARE(changedSpy.at(1).count(), 1);

    fileName = changedSpy.at(0).at(0).toString();
    QCOMPARE(fileName, testDir.dirName());
    fileName = changedSpy.at(1).at(0).toString();
    QCOMPARE(fileName, testDir.dirName());

    changedSpy.clear();

    // recreate the file, we should not get any notification
    QDir().mkdir("testDir");

    // qDebug() << "waiting max 5 seconds for notification for dir recreation to trigger";
    timer.start(5000);
    eventLoop.exec();

    QCOMPARE(changedSpy.count(), 0);

    QVERIFY(QDir().rmdir("testDir"));
}

void tst_QFileSystemWatcher::addPath()
{
    QFileSystemWatcher watcher;
    QString home = QDir::homePath();
    watcher.addPath(home);
    QCOMPARE(watcher.directories().count(), 1);
    QCOMPARE(watcher.directories().first(), home);
    watcher.addPath(home);
    QCOMPARE(watcher.directories().count(), 1);

    // With empty string
    QTest::ignoreMessage(QtWarningMsg, "QFileSystemWatcher::addPath: path is empty");
    watcher.addPath(QString());
}

void tst_QFileSystemWatcher::removePath()
{
    QFileSystemWatcher watcher;
    QString home = QDir::homePath();
    watcher.addPath(home);
    watcher.removePath(home);
    QCOMPARE(watcher.directories().count(), 0);
    watcher.removePath(home);
    QCOMPARE(watcher.directories().count(), 0);

    // With empty string
    QTest::ignoreMessage(QtWarningMsg, "QFileSystemWatcher::removePath: path is empty");
    watcher.removePath(QString());
}

void tst_QFileSystemWatcher::addPaths()
{
    QFileSystemWatcher watcher;
    QStringList paths;
    paths << QDir::homePath() << QDir::currentPath();
    watcher.addPaths(paths);
    QCOMPARE(watcher.directories().count(), 2);

    // With empty list
    paths.clear();
    QTest::ignoreMessage(QtWarningMsg, "QFileSystemWatcher::addPaths: list is empty");
    watcher.addPaths(paths);
}

void tst_QFileSystemWatcher::removePaths()
{
    QFileSystemWatcher watcher;
    QStringList paths;
    paths << QDir::homePath() << QDir::currentPath();
    watcher.addPaths(paths);
    QCOMPARE(watcher.directories().count(), 2);
    watcher.removePaths(paths);
    QCOMPARE(watcher.directories().count(), 0);

    //With empty list
    paths.clear();
    QTest::ignoreMessage(QtWarningMsg, "QFileSystemWatcher::removePaths: list is empty");
    watcher.removePaths(paths);
}

QTEST_MAIN(tst_QFileSystemWatcher)
#include "tst_qfilesystemwatcher.moc"

#endif
