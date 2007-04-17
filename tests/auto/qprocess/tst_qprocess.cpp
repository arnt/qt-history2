/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtNetwork/QHostInfo>
#include <stdlib.h>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

//TESTED_CLASS=
//TESTED_FILES=corelib/io/qprocess.h corelib/io/qprocess.cpp corelib/io/qprocess_p.h corelib/io/qprocess_unix.cpp

Q_DECLARE_METATYPE(QList<QProcess::ExitStatus>);
Q_DECLARE_METATYPE(QProcess::ExitStatus);

#define QPROCESS_VERIFY(Process, Fn) \
{ \
const bool ret = Process.Fn; \
if (ret == false) \
	qWarning("QProcess error: %d: %s", Process.error(), qPrintable(Process.errorString())); \
QVERIFY(ret); \
}

class tst_QProcess : public QObject
{
    Q_OBJECT

public:
    tst_QProcess();
    virtual ~tst_QProcess();

public slots:
    void init();
    void cleanup();

private slots:
    void getSetCheck();
    void constructing();
    void simpleStart();
    void startDetached();
    void crashTest();
    void crashTest2();
    void echoTest_data();
    void echoTest();
    void echoTest2();
    void echoTest_performance();
#if defined Q_OS_WIN
    void echoTestGui();
    void batFiles_data();
    void batFiles();
#endif
    void exitStatus_data();
    void exitStatus();
    void loopBackTest();
    void readTimeoutAndThenCrash();
    void waitForFinished();
    void deadWhileReading();
    void restartProcessDeadlock();
    void closeWriteChannel();
    void closeReadChannel();
    void openModes();
    void emitReadyReadOnlyWhenNewDataArrives();
    void hardExit();
    void softExit();
    void softExitInSlots_data();
    void softExitInSlots();
    void mergedChannels();
    void forwardedChannels();
    void atEnd();
    void atEnd2();
    void processInAThread();
    void processesInMultipleThreads();
    void waitForFinishedWithTimeout();
    void waitForReadyReadInAReadyReadSlot();
    void waitForBytesWrittenInABytesWrittenSlot();
    void spaceArgsTest_data();
    void spaceArgsTest();
    void exitCodeTest();
    void systemEnvironment();
    void spaceInName();
    void lockupsInStartDetached();
    void waitForReadyReadForNonexistantProcess();
    void setStandardInputFile();
    void setStandardOutputFile_data();
    void setStandardOutputFile();
    void setStandardOutputProcess_data();
    void setStandardOutputProcess();
    void failToStart();
    void failToStartWithWait();
    void failToStartWithEventLoop();
    void removeFileWhileProcessIsRunning();
    void fileWriterProcess();
    void detachedWorkingDirectoryAndPid();

protected slots:
    void readFromProcess();
    void exitLoopSlot();
    void restartProcess();
    void waitForReadyReadInAReadyReadSlotSlot();
    void waitForBytesWrittenInABytesWrittenSlotSlot();
    
private:
    QProcess *process;
    qint64 bytesAvailable;
};

// Testing get/set functions
void tst_QProcess::getSetCheck()
{
    QProcess obj1;
    // ProcessChannelMode QProcess::readChannelMode()
    // void QProcess::setReadChannelMode(ProcessChannelMode)
    obj1.setReadChannelMode(QProcess::ProcessChannelMode(QProcess::SeparateChannels));
    QCOMPARE(QProcess::ProcessChannelMode(QProcess::SeparateChannels), obj1.readChannelMode());
    obj1.setReadChannelMode(QProcess::ProcessChannelMode(QProcess::MergedChannels));
    QCOMPARE(QProcess::ProcessChannelMode(QProcess::MergedChannels), obj1.readChannelMode());
    obj1.setReadChannelMode(QProcess::ProcessChannelMode(QProcess::ForwardedChannels));
    QCOMPARE(QProcess::ProcessChannelMode(QProcess::ForwardedChannels), obj1.readChannelMode());

    // ProcessChannel QProcess::readChannel()
    // void QProcess::setReadChannel(ProcessChannel)
    obj1.setReadChannel(QProcess::ProcessChannel(QProcess::StandardOutput));
    QCOMPARE(QProcess::ProcessChannel(QProcess::StandardOutput), obj1.readChannel());
    obj1.setReadChannel(QProcess::ProcessChannel(QProcess::StandardError));
    QCOMPARE(QProcess::ProcessChannel(QProcess::StandardError), obj1.readChannel());
}

tst_QProcess::tst_QProcess()
{
}

tst_QProcess::~tst_QProcess()
{
}

void tst_QProcess::init()
{
}

void tst_QProcess::cleanup()
{
}

//-----------------------------------------------------------------------------
void tst_QProcess::constructing()
{
    QProcess process;
    QCOMPARE(process.readChannel(), QProcess::StandardOutput);
    QCOMPARE(process.workingDirectory(), QString());
    QCOMPARE(process.environment(), QStringList());
    QCOMPARE(process.error(), QProcess::UnknownError);
    QCOMPARE(process.state(), QProcess::NotRunning);
    QCOMPARE(process.pid(), Q_PID(0));
    QCOMPARE(process.readAllStandardOutput(), QByteArray());
    QCOMPARE(process.readAllStandardError(), QByteArray());
    QCOMPARE(process.canReadLine(), false);

    // QIODevice
    QCOMPARE(process.openMode(), QIODevice::NotOpen);
    QVERIFY(!process.isOpen());
    QVERIFY(!process.isReadable());
    QVERIFY(!process.isWritable());
    QVERIFY(process.isSequential());
    QCOMPARE(process.pos(), qlonglong(0));
    QCOMPARE(process.size(), qlonglong(0));
    QVERIFY(process.atEnd());
    QCOMPARE(process.bytesAvailable(), qlonglong(0));
    QCOMPARE(process.bytesToWrite(), qlonglong(0));
    QVERIFY(!process.errorString().isEmpty());

    char c;
    QCOMPARE(process.read(&c, 1), qlonglong(-1));
    QCOMPARE(process.write(&c, 1), qlonglong(-1));
}

void tst_QProcess::simpleStart()
{
#ifdef Q_OS_UNIX
    QString path = "PATH=";
    path += ::getenv("PATH");
    path += ":/usr/sbin:/sbin:/usr/etc";
    ::putenv(strdup(path.toLatin1()));
#endif
    process = new QProcess;
    connect(process, SIGNAL(readyRead()), this, SLOT(readFromProcess()));

    process->start("ping");
    if (process->state() != QProcess::Starting)
        QCOMPARE(process->state(), QProcess::Running);
    QVERIFY(process->waitForStarted(5000));
    QCOMPARE(process->state(), QProcess::Running);
    while (process->waitForReadyRead(5000))
    { }
    QCOMPARE(process->state(), QProcess::NotRunning);

    delete process;
    process = 0;
}
//-----------------------------------------------------------------------------
void tst_QProcess::startDetached()
{
#if QT_VERSION >= 0x040200
    QProcess proc;
    QVERIFY(proc.startDetached("testProcessNormal/testProcessNormal",
                               QStringList() << "arg1" << "arg2"));
#endif
    QCOMPARE(QProcess::startDetached("nonexistingexe"), false);
}

//-----------------------------------------------------------------------------
void tst_QProcess::readFromProcess()
{
    int lines = 0;
    while (process->canReadLine()) {
        ++lines;
        QByteArray line = process->readLine();
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::crashTest()
{
    process = new QProcess;
    process->start("testProcessCrash/testProcessCrash");
    QVERIFY(process->waitForStarted(5000));

    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ExitStatus");

    QSignalSpy spy(process, SIGNAL(error(QProcess::ProcessError)));
    QSignalSpy spy2(process, SIGNAL(finished(int, QProcess::ExitStatus)));

    QVERIFY(process->waitForFinished(5000));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const QProcess::ProcessError *>(spy.at(0).at(0).constData()), QProcess::Crashed);

    QCOMPARE(spy2.count(), 1);
    QCOMPARE(*static_cast<const QProcess::ExitStatus *>(spy2.at(0).at(1).constData()), QProcess::CrashExit);

    QCOMPARE(process->exitStatus(), QProcess::CrashExit);

    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::crashTest2()
{
    process = new QProcess;
    process->start("testProcessCrash/testProcessCrash");
    QVERIFY(process->waitForStarted(5000));

    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ExitStatus");

    QSignalSpy spy(process, SIGNAL(error(QProcess::ProcessError)));
    QSignalSpy spy2(process, SIGNAL(finished(int, QProcess::ExitStatus)));

    QObject::connect(process, SIGNAL(finished(int)), this, SLOT(exitLoopSlot()));

    QTestEventLoop::instance().enterLoop(5);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Failed to detect crash : operation timed out");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const QProcess::ProcessError *>(spy.at(0).at(0).constData()), QProcess::Crashed);

    QCOMPARE(spy2.count(), 1);
    QCOMPARE(*static_cast<const QProcess::ExitStatus *>(spy2.at(0).at(1).constData()), QProcess::CrashExit);

    QCOMPARE(process->exitStatus(), QProcess::CrashExit);

    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::echoTest_data()
{
    QTest::addColumn<QByteArray>("input");

    QTest::newRow("1") << QByteArray("H");
    QTest::newRow("2") << QByteArray("He");
    QTest::newRow("3") << QByteArray("Hel");
    QTest::newRow("4") << QByteArray("Hell");
    QTest::newRow("5") << QByteArray("Hello");
    QTest::newRow("100 bytes") << QByteArray(100, '@');
    QTest::newRow("1000 bytes") << QByteArray(1000, '@');
    QTest::newRow("10000 bytes") << QByteArray(10000, '@');
}

//-----------------------------------------------------------------------------
void tst_QProcess::echoTest()
{
    QFETCH(QByteArray, input);

    process = new QProcess;
    connect(process, SIGNAL(readyRead()), this, SLOT(exitLoopSlot()));

#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif
    QVERIFY(process->waitForStarted(5000));

    process->write(input);

    QTime stopWatch;
    stopWatch.start();
    do {
        QVERIFY(process->isOpen());
        QTestEventLoop::instance().enterLoop(2);
    } while (stopWatch.elapsed() < 60000 && process->bytesAvailable() < input.size());
    if (stopWatch.elapsed() >= 60000)
        QFAIL("Timed out");

    QByteArray message = process->readAll();
    QCOMPARE(message.size(), input.size());

    char *c1 = message.data();
    char *c2 = input.data();
    while (*c1 && *c2) {
        if (*c1 != *c2)
            QCOMPARE(*c1, *c2);
        ++c1;
        ++c2;
    }
    QCOMPARE(*c1, *c2);

    process->write("", 1);

    QVERIFY(process->waitForFinished(5000));


    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::exitLoopSlot()
{
    QTestEventLoop::instance().exitLoop();
}

//-----------------------------------------------------------------------------
void tst_QProcess::echoTest2()
{
    process = new QProcess;
    connect(process, SIGNAL(readyRead()), this, SLOT(exitLoopSlot()));

#ifdef Q_OS_MAC
    process->start("testProcessEcho2/testProcessEcho2.app");
#else
    process->start("testProcessEcho2/testProcessEcho2");
#endif
    QVERIFY(process->waitForStarted(5000));
    QVERIFY(!process->waitForReadyRead(250));
    QCOMPARE(process->error(), QProcess::Timedout);

    process->write("Hello");
    QSignalSpy spy1(process, SIGNAL(readyReadStandardOutput()));
    QSignalSpy spy2(process, SIGNAL(readyReadStandardError()));

    QTime stopWatch;
    stopWatch.start();
    forever {
        QTestEventLoop::instance().enterLoop(1);
        if (stopWatch.elapsed() >= 30000)
            QFAIL("Timed out");
        process->setReadChannel(QProcess::StandardOutput);
        qint64 baso = process->bytesAvailable();

        process->setReadChannel(QProcess::StandardError);
        qint64 base = process->bytesAvailable();
        if (baso == 5 && base == 5)
            break;
    }

    QVERIFY(spy1.count() > 0);
    QVERIFY(spy2.count() > 0);

    QCOMPARE(process->readAllStandardOutput(), QByteArray("Hello"));
    QCOMPARE(process->readAllStandardError(), QByteArray("Hello"));

    process->write("", 1);
    QVERIFY(process->waitForFinished(5000));

    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::echoTest_performance()
{
    QProcess process;
#ifdef Q_OS_MAC
    process.start("testProcessLoopback/testProcessLoopback.app");
#else
    process.start("testProcessLoopback/testProcessLoopback");
#endif

    QByteArray array;
    array.resize(1024 * 1024);
    for (int j = 0; j < array.size(); ++j)
        array[j] = 'a' + (j % 20);

    QVERIFY(process.waitForStarted());

    QTime stopWatch;
    stopWatch.start();
    
    qDebug() << "Writing 10MB to a loopback process...";

    for (int i = 0; i < 10; ++i) {
        process.write(array);
        while (process.bytesToWrite() > 0) {
            QVERIFY(process.waitForBytesWritten(5000));
            QVERIFY(process.waitForReadyRead(5000));
        }

        while (process.bytesAvailable() < ((i + 1) * array.size())) 
            QVERIFY2(process.waitForReadyRead(5000), qPrintable(process.errorString()));
    }

    qDebug() << "Elapsed time:" << qPrintable(QString("%1s").arg(stopWatch.elapsed() / 1000.0, 0, 'g', 2))
             << qPrintable(QString("(%1 MB/s)").arg((10.0 * 1000.0) / double(stopWatch.elapsed()), 0, 'g', 2));

    QByteArray dump = process.readAll();
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < array.size(); ++j)
            QCOMPARE(char(dump.at(i * (1024 * 1024) + j)), char('a' + (j % 20)));
    }

    process.closeWriteChannel();
    QVERIFY(process.waitForFinished());
}

#if defined Q_OS_WIN
//-----------------------------------------------------------------------------
void tst_QProcess::echoTestGui()
{
    QProcess process;

    process.start("testProcessEchoGui/testProcessEchoGui");


    process.write("Hello");
    process.write("q");

    QVERIFY(process.waitForFinished(50000));

    QCOMPARE(process.readAllStandardOutput(), QByteArray("Hello"));
    QCOMPARE(process.readAllStandardError(), QByteArray("Hello"));
}

//-----------------------------------------------------------------------------
void tst_QProcess::batFiles_data()
{
    QTest::addColumn<QString>("batFile");
    QTest::addColumn<QByteArray>("output");

    QTest::newRow("simple") << QString::fromLatin1("testBatFiles/simple.bat") << QByteArray("Hello");
    QTest::newRow("with space") << QString::fromLatin1("testBatFiles/with space.bat") << QByteArray("Hello");
}

void tst_QProcess::batFiles()
{
    QFETCH(QString, batFile);
    QFETCH(QByteArray, output);

    QProcess proc;
    
    proc.start(batFile, QStringList());

    QVERIFY(proc.waitForFinished(5000));

    QVERIFY(proc.bytesAvailable() > 0);

    QVERIFY(proc.readAll().startsWith(output));
}

#endif

//-----------------------------------------------------------------------------
void tst_QProcess::exitStatus_data()
{
    QTest::addColumn<QStringList>("processList");
    QTest::addColumn<QList<QProcess::ExitStatus> >("exitStatus");

    QTest::newRow("normal") << (QStringList() << "testProcessNormal/testProcessNormal")
                            << (QList<QProcess::ExitStatus>() << QProcess::NormalExit);
    QTest::newRow("crash") << (QStringList() << "testProcessCrash/testProcessCrash")
                            << (QList<QProcess::ExitStatus>() << QProcess::CrashExit);

    QTest::newRow("normal-crash") << (QStringList()
                                      << "testProcessNormal/testProcessNormal"
                                      << "testProcessCrash/testProcessCrash")
                                  << (QList<QProcess::ExitStatus>()
                                      << QProcess::NormalExit
                                      << QProcess::CrashExit);
    QTest::newRow("crash-normal") << (QStringList()
                                      << "testProcessCrash/testProcessCrash"
                                      << "testProcessNormal/testProcessNormal")
                                  << (QList<QProcess::ExitStatus>()
                                      << QProcess::CrashExit
                                      << QProcess::NormalExit);
}

void tst_QProcess::exitStatus()
{
    process = new QProcess;
    QFETCH(QStringList, processList);
    QFETCH(QList<QProcess::ExitStatus>, exitStatus);

    Q_ASSERT(processList.count() == exitStatus.count());
    for (int i = 0; i < processList.count(); ++i) {
        process->start(processList.at(i));
        QVERIFY(process->waitForStarted(5000));
        QVERIFY(process->waitForFinished(5000));

        QCOMPARE(process->exitStatus(), exitStatus.at(i));
    }

    process->deleteLater();
    process = 0;
}
//-----------------------------------------------------------------------------
void tst_QProcess::loopBackTest()
{
    process = new QProcess;
#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif
    QVERIFY(process->waitForStarted(5000));

    for (int i = 0; i < 100; ++i) {
        process->write("Hello");
        do {
            QVERIFY(process->waitForReadyRead(5000));
        } while (process->bytesAvailable() < 5);
        QCOMPARE(process->readAll(), QByteArray("Hello"));
    }

    process->write("", 1);
    QVERIFY(process->waitForFinished(5000));

    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::readTimeoutAndThenCrash()
{
    process = new QProcess;
#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif
    if (process->state() != QProcess::Starting)
        QCOMPARE(process->state(), QProcess::Running);

    QVERIFY(process->waitForStarted(5000));
    QCOMPARE(process->state(), QProcess::Running);

    QVERIFY(!process->waitForReadyRead(5000));
    QCOMPARE(process->error(), QProcess::Timedout);

    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    QSignalSpy spy(process, SIGNAL(error(QProcess::ProcessError)));

    process->kill();

    QVERIFY(process->waitForFinished(5000));
    QCOMPARE(process->state(), QProcess::NotRunning);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(*static_cast<const QProcess::ProcessError *>(spy.at(0).at(0).constData()), QProcess::Crashed);

    delete process;
    process = 0;
}

void tst_QProcess::waitForFinished()
{
    QProcess process;

#ifdef Q_OS_MAC
    process.start("testProcessOutput/testProcessOutput.app");
#else
    process.start("testProcessOutput/testProcessOutput");
#endif

    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);

    QString output = process.readAll();
    QCOMPARE(output.count("\n"), 10*1024);

    process.start("blurdybloop");
    QVERIFY(!process.waitForFinished());
    QCOMPARE(process.error(), QProcess::FailedToStart);
}


void tst_QProcess::deadWhileReading()
{
    QProcess process;

#ifdef Q_OS_MAC
    process.start("testProcessDeadWhileReading/testProcessDeadWhileReading.app");
#else
    process.start("testProcessDeadWhileReading/testProcessDeadWhileReading");
#endif

    QString output;

    QVERIFY(process.waitForStarted(5000));
    while (process.waitForReadyRead(5000))
        output += process.readAll();

    QCOMPARE(output.count("\n"), 10*1024);
    process.waitForFinished();
}

//-----------------------------------------------------------------------------
void tst_QProcess::restartProcessDeadlock()
{
    // The purpose of this test is to detect whether restarting a
    // process in the finished() connected slot causes a deadlock
    // because of the way QProcessManager uses its locks.
    QProcess proc;
    process = &proc;
    connect(process, SIGNAL(finished(int)), this, SLOT(restartProcess()));

#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif

    QCOMPARE(process->write("", 1), qlonglong(1));
    QVERIFY(process->waitForFinished(5000));

    process->disconnect(SIGNAL(finished(int)));

    QCOMPARE(process->write("", 1), qlonglong(1));

    QVERIFY(process->waitForFinished(5000));
}

void tst_QProcess::restartProcess()
{
#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif
}

//-----------------------------------------------------------------------------
void tst_QProcess::closeWriteChannel()
{
    QProcess more;
    more.start("testProcessEOF/testProcessEOF");

    QVERIFY(more.waitForStarted(5000));
    QVERIFY(!more.waitForReadyRead(250));
    QCOMPARE(more.error(), QProcess::Timedout);

    QVERIFY(more.write("Data to read") != -1);

    QVERIFY(!more.waitForReadyRead(250));
    QCOMPARE(more.error(), QProcess::Timedout);

    more.closeWriteChannel();

    QVERIFY(more.waitForReadyRead(5000));
    QVERIFY(more.readAll().startsWith("Data to read"));

    if (more.state() == QProcess::Running)
        more.write("q");
    QVERIFY(more.waitForFinished(5000));
}

//-----------------------------------------------------------------------------
void tst_QProcess::closeReadChannel()
{
    for (int i = 0; i < 10; ++i) {
        QProcess::ProcessChannel channel1 = QProcess::StandardOutput;
        QProcess::ProcessChannel channel2 = QProcess::StandardError;

        QProcess proc;
#ifdef Q_OS_MAC
        proc.start("testProcessEcho2/testProcessEcho2.app");
#else
        proc.start("testProcessEcho2/testProcessEcho2");
#endif
        QVERIFY(proc.waitForStarted(5000));
        proc.closeReadChannel(i&1 ? channel2 : channel1);
        proc.setReadChannel(i&1 ? channel2 : channel1);
        proc.write("Data");

        QVERIFY(!proc.waitForReadyRead(5000));
        QVERIFY(proc.readAll().isEmpty());

        proc.setReadChannel(i&1 ? channel1 : channel2);

        while (proc.bytesAvailable() < 4 && proc.waitForReadyRead(5000))
        { }

        QCOMPARE(proc.readAll(), QByteArray("Data"));

        proc.write("", 1);
        QVERIFY(proc.waitForFinished(5000));
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::openModes()
{
    QProcess proc;
    QVERIFY(!proc.isOpen());
    QVERIFY(proc.openMode() == QProcess::NotOpen);
#ifdef Q_OS_MAC
    proc.start("testProcessEcho3/testProcessEcho3.app");
#else
    proc.start("testProcessEcho3/testProcessEcho3");
#endif
    QVERIFY(proc.waitForStarted(5000));
    QVERIFY(proc.isOpen());
    QVERIFY(proc.openMode() == QProcess::ReadWrite);
    QVERIFY(proc.isReadable());
    QVERIFY(proc.isWritable());

    proc.write("Data");

    proc.closeWriteChannel();

    QVERIFY(proc.isWritable());
    QVERIFY(proc.openMode() == QProcess::ReadWrite);

    while (proc.bytesAvailable() < 4 && proc.waitForReadyRead(5000))
    { }

    QCOMPARE(proc.readAll().constData(), QByteArray("Data").constData());

    proc.closeReadChannel(QProcess::StandardOutput);

    QVERIFY(proc.openMode() == QProcess::ReadWrite);
    QVERIFY(proc.isReadable());

    proc.closeReadChannel(QProcess::StandardError);

    QVERIFY(proc.openMode() == QProcess::ReadWrite);
    QVERIFY(proc.isReadable());

    proc.close();
    QVERIFY(!proc.isOpen());
    QVERIFY(!proc.isReadable());
    QVERIFY(!proc.isWritable());
    QCOMPARE(proc.state(), QProcess::NotRunning);
}

//-----------------------------------------------------------------------------
void tst_QProcess::emitReadyReadOnlyWhenNewDataArrives()
{
    QProcess proc;
    connect(&proc, SIGNAL(readyRead()), this, SLOT(exitLoopSlot()));
    QSignalSpy spy(&proc, SIGNAL(readyRead()));

#ifdef Q_OS_MAC
    proc.start("testProcessEcho/testProcessEcho.app");
#else
    proc.start("testProcessEcho/testProcessEcho");
#endif

    QCOMPARE(spy.count(), 0);

    proc.write("A");

    QTestEventLoop::instance().enterLoop(5);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Operation timed out");

    QCOMPARE(spy.count(), 1);

    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(QTestEventLoop::instance().timeout());
    QVERIFY(!proc.waitForReadyRead(250));

    QObject::disconnect(&proc, SIGNAL(readyRead()), 0, 0);
    proc.write("B");
    QVERIFY(proc.waitForReadyRead(5000));

    proc.write("", 1);
    QVERIFY(proc.waitForFinished(5000));
}

//-----------------------------------------------------------------------------
void tst_QProcess::hardExit()
{
    QProcess proc;

#ifdef Q_OS_MAC
    proc.start("testProcessEcho/testProcessEcho.app");
#else
    proc.start("testProcessEcho/testProcessEcho");
#endif

    QVERIFY(proc.waitForStarted(5000));

    proc.kill();

    QVERIFY(proc.waitForFinished(5000));
    QCOMPARE(int(proc.state()), int(QProcess::NotRunning));
    QCOMPARE(int(proc.error()), int(QProcess::Crashed));
}

//-----------------------------------------------------------------------------
void tst_QProcess::softExit()
{
    QProcess proc;

    proc.start("testSoftExit/testSoftExit");

    QVERIFY(proc.waitForStarted(10000));
    QVERIFY(proc.waitForReadyRead(10000));

    proc.terminate();

    QVERIFY(proc.waitForFinished(10000));
    QCOMPARE(int(proc.state()), int(QProcess::NotRunning));
    QCOMPARE(int(proc.error()), int(QProcess::UnknownError));
}

class SoftExitProcess : public QProcess
{
    Q_OBJECT
public:
    bool waitedForFinished;
    
    SoftExitProcess(int n) : waitedForFinished(false), n(n), killing(false)
    {
        connect(this, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(finishedSlot(int, QProcess::ExitStatus)));

        switch (n) {
        case 0:
            setReadChannelMode(QProcess::MergedChannels);
            connect(this, SIGNAL(readyRead()), this, SLOT(terminateSlot()));
            break;
        case 1:
            connect(this, SIGNAL(readyReadStandardOutput()),
                    this, SLOT(terminateSlot()));
            break;
        case 2:
            connect(this, SIGNAL(readyReadStandardError()),
                    this, SLOT(terminateSlot()));
            break;
        case 3:
            connect(this, SIGNAL(started()),
                    this, SLOT(terminateSlot()));
            break;
        case 4:
        default:
            connect(this, SIGNAL(stateChanged(QProcess::ProcessState)),
                    this, SLOT(terminateSlot()));
            break;
        }
    }

public slots:
    void terminateSlot()
    {
        if (killing || (n == 4 && state() != Running)) {
            // Don't try to kill the process before it is running - that can
            // be hazardous, as the actual child process might not be running
            // yet. Also, don't kill it "recursively".
            return;
        }
        killing = true;
        readAll();
        terminate();
        if ((waitedForFinished = waitForFinished(5000)) == false) {
            kill();
            if (state() != NotRunning)
                waitedForFinished = waitForFinished(5000);
        }
    }

    void finishedSlot(int, QProcess::ExitStatus)
    {
        waitedForFinished = true;
    }

private:
    int n;
    bool killing;
};

//-----------------------------------------------------------------------------
void tst_QProcess::softExitInSlots_data()
{
    QTest::addColumn<QString>("appName");

#ifdef Q_OS_MAC
    QTest::newRow("gui app") << "testGuiProcess/testGuiProcess.app";
#else
    QTest::newRow("gui app") << "testGuiProcess/testGuiProcess";
#endif
#ifdef Q_OS_MAC
    QTest::newRow("console app") << "testProcessEcho2/testProcessEcho2.app";
#else
    QTest::newRow("console app") << "testProcessEcho2/testProcessEcho2";
#endif
}

//-----------------------------------------------------------------------------
void tst_QProcess::softExitInSlots()
{
    QFETCH(QString, appName);

    for (int i = 0; i < 5; ++i) {
        SoftExitProcess proc(i);
        proc.start(appName);
        proc.write("OLEBOLE", 8); // include the \0
        QTestEventLoop::instance().enterLoop(1);
        QCOMPARE(proc.state(), QProcess::NotRunning);
        QVERIFY(proc.waitedForFinished);
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::mergedChannels()
{
    QProcess process;
    process.setReadChannelMode(QProcess::MergedChannels);
    QCOMPARE(process.readChannelMode(), QProcess::MergedChannels);

#ifdef Q_OS_MAC
    process.start("testProcessEcho2/testProcessEcho2.app");
#else
    process.start("testProcessEcho2/testProcessEcho2");
#endif

    QVERIFY(process.waitForStarted(5000));

    for (int i = 0; i < 100; ++i) {
        QCOMPARE(process.write("abc"), qlonglong(3));
        while (process.bytesAvailable() < 6)
            QVERIFY(process.waitForReadyRead(5000));
        QCOMPARE(process.readAll(), QByteArray("aabbcc"));
    }

    process.closeWriteChannel();
    QVERIFY(process.waitForFinished(5000));
}

//-----------------------------------------------------------------------------
void tst_QProcess::forwardedChannels()
{
    QProcess process;
    process.setReadChannelMode(QProcess::ForwardedChannels);
    QCOMPARE(process.readChannelMode(), QProcess::ForwardedChannels);
    
#ifdef Q_OS_MAC
    process.start("testProcessEcho2/testProcessEcho2.app");
#else
    process.start("testProcessEcho2/testProcessEcho2");
#endif

    QVERIFY(process.waitForStarted(5000));
    QCOMPARE(process.write("forwarded\n"), qlonglong(10));
    QVERIFY(!process.waitForReadyRead(250));
    QCOMPARE(process.bytesAvailable(), qlonglong(0));

    process.closeWriteChannel();
    QVERIFY(process.waitForFinished(5000));
}


//-----------------------------------------------------------------------------
void tst_QProcess::atEnd()
{
    QProcess process;

#ifdef Q_OS_MAC
    process.start("testProcessEcho/testProcessEcho.app");
#else
    process.start("testProcessEcho/testProcessEcho");
#endif
    process.write("abcdefgh\n");

    while (process.bytesAvailable() < 8)
        QVERIFY(process.waitForReadyRead(5000));

    QTextStream stream(&process);
    QVERIFY(!stream.atEnd());
    QString tmp = stream.readLine();
    QVERIFY(stream.atEnd());
    QCOMPARE(tmp, QString::fromLatin1("abcdefgh"));

    process.write("", 1);
    QVERIFY(process.waitForFinished(5000));
}

class TestThread : public QThread
{
    Q_OBJECT
public:
    inline int code()
    {
        return exitCode;
    }

protected:
    inline void run()
    {
        exitCode = 90210;

        QProcess process;
        connect(&process, SIGNAL(finished(int)), this, SLOT(catchExitCode(int)),
                Qt::DirectConnection);

#ifdef Q_OS_MAC
        process.start("testProcessEcho/testProcessEcho.app");
#else
        process.start("testProcessEcho/testProcessEcho");
#endif
        QCOMPARE(process.write("abc\0", 4), qint64(4));
        exitCode = exec();
    }

protected slots:
    inline void catchExitCode(int exitCode)
    {
        this->exitCode = exitCode;
        exit(exitCode);
    }

private:
    int exitCode;
};

//-----------------------------------------------------------------------------
void tst_QProcess::processInAThread()
{
    for (int i = 0; i < 3; ++i) {
        TestThread thread;
        thread.start();
        QVERIFY(thread.wait(10000));
        QCOMPARE(thread.code(), 0);
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::processesInMultipleThreads()
{
    for (int i = 0; i < 10; ++i) {
        TestThread thread1;
        TestThread thread2;
        TestThread thread3;

        thread1.start();
        thread2.start();
        thread3.start();

        QVERIFY(thread2.wait(10000));
        QVERIFY(thread3.wait(10000));
        QVERIFY(thread1.wait(10000));

        QCOMPARE(thread1.code(), 0);
        QCOMPARE(thread2.code(), 0);
        QCOMPARE(thread3.code(), 0);
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::waitForFinishedWithTimeout()
{
    process = new QProcess(this);

#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif
    QVERIFY(process->waitForStarted(5000));
    QVERIFY(!process->waitForFinished(1));

    process->write("", 1);
    QVERIFY(process->waitForFinished());
}

//-----------------------------------------------------------------------------
void tst_QProcess::waitForReadyReadInAReadyReadSlot()
{
    process = new QProcess(this);
    connect(process, SIGNAL(readyRead()), this, SLOT(waitForReadyReadInAReadyReadSlotSlot()));
    connect(process, SIGNAL(finished(int)), this, SLOT(exitLoopSlot()));
    bytesAvailable = 0;

#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif
    QVERIFY(process->waitForStarted(5000));

    QSignalSpy spy(process, SIGNAL(readyRead()));
    process->write("foo");
    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(spy.count(), 1);
    
    process->disconnect();
    QVERIFY(process->waitForFinished(5000));
    QVERIFY(process->bytesAvailable() > bytesAvailable);
    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::waitForReadyReadInAReadyReadSlotSlot()
{
    bytesAvailable = process->bytesAvailable();
    process->write("bar", 4);
    QVERIFY(process->waitForReadyRead(5000));
    QTestEventLoop::instance().exitLoop();
}

//-----------------------------------------------------------------------------
void tst_QProcess::waitForBytesWrittenInABytesWrittenSlot()
{
    process = new QProcess(this);
    connect(process, SIGNAL(bytesWritten(qint64)), this, SLOT(waitForBytesWrittenInABytesWrittenSlotSlot()));
    bytesAvailable = 0;

#ifdef Q_OS_MAC
    process->start("testProcessEcho/testProcessEcho.app");
#else
    process->start("testProcessEcho/testProcessEcho");
#endif
    QVERIFY(process->waitForStarted(5000));

    qRegisterMetaType<qint64>("qint64");
    QSignalSpy spy(process, SIGNAL(bytesWritten(qint64)));
    process->write("f");
    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(spy.count(), 1);
    process->write("", 1);
    process->disconnect();
    QVERIFY(process->waitForFinished());
    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::waitForBytesWrittenInABytesWrittenSlotSlot()
{
    process->write("b");
    QVERIFY(process->waitForBytesWritten(5000));
    QTestEventLoop::instance().exitLoop();
}

//-----------------------------------------------------------------------------
void tst_QProcess::spaceArgsTest_data()
{
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QString>("stringArgs");

    // arg1 | arg2
    QTest::newRow("arg1 arg2") << (QStringList() << QString::fromLatin1("arg1") << QString::fromLatin1("arg2"))
                               << QString::fromLatin1("arg1 arg2");
    // "arg1" | ar "g2
    QTest::newRow("\"\"\"\"arg1\"\"\"\" \"ar \"\"\"g2\"") << (QStringList() << QString::fromLatin1("\"arg1\"") << QString::fromLatin1("ar \"g2"))
                                                          << QString::fromLatin1("\"\"\"\"arg1\"\"\"\" \"ar \"\"\"g2\"");
    // ar g1 | a rg 2
    QTest::newRow("\"ar g1\" \"a rg 2\"") << (QStringList() << QString::fromLatin1("ar g1") << QString::fromLatin1("a rg 2"))
                                          << QString::fromLatin1("\"ar g1\" \"a rg 2\"");
    // -lar g1 | -l"ar g2"
    QTest::newRow("\"-lar g1\" \"-l\"\"\"ar g2\"\"\"\"") << (QStringList() << QString::fromLatin1("-lar g1") << QString::fromLatin1("-l\"ar g2\""))
                                                         << QString::fromLatin1("\"-lar g1\" \"-l\"\"\"ar g2\"\"\"\"");
    // ar"g1
    QTest::newRow("ar\"\"\"\"g1") << (QStringList() << QString::fromLatin1("ar\"g1"))
                                  << QString::fromLatin1("ar\"\"\"\"g1");
    // ar/g1
    QTest::newRow("ar\\g1") << (QStringList() << QString::fromLatin1("ar\\g1"))
                            << QString::fromLatin1("ar\\g1");
    // ar\g"1
    QTest::newRow("ar\\g\"\"\"\"1") << (QStringList() << QString::fromLatin1("ar\\g\"1"))
                                    << QString::fromLatin1("ar\\g\"\"\"\"1");
    // arg\"1
    QTest::newRow("arg\\\"\"\"1") << (QStringList() << QString::fromLatin1("arg\\\"1"))
                                  << QString::fromLatin1("arg\\\"\"\"1");
    // """"
    QTest::newRow("\"\"\"\"\"\"\"\"\"\"\"\"") << (QStringList() << QString::fromLatin1("\"\"\"\""))
                                              << QString::fromLatin1("\"\"\"\"\"\"\"\"\"\"\"\"");
    // """" | "" ""
    QTest::newRow("\"\"\"\"\"\"\"\"\"\"\"\" \"\"\"\"\"\"\" \"\"\"\"\"\"\"") << (QStringList() << QString::fromLatin1("\"\"\"\"") << QString::fromLatin1("\"\" \"\""))
                                                                            << QString::fromLatin1("\"\"\"\"\"\"\"\"\"\"\"\" \"\"\"\"\"\"\" \"\"\"\"\"\"\"");
    // ""  ""
    QTest::newRow("\"\"\"\"\"\"\" \"\" \"\"\"\"\"\"\" (bogus double quotes)") << (QStringList() << QString::fromLatin1("\"\"  \"\""))
                                                                              << QString::fromLatin1("\"\"\"\"\"\"\" \"\" \"\"\"\"\"\"\"");
    // ""  ""
    QTest::newRow(" \"\"\"\"\"\"\" \"\" \"\"\"\"\"\"\"   (bogus double quotes)") << (QStringList() << QString::fromLatin1("\"\"  \"\""))
                                                                                 << QString::fromLatin1(" \"\"\"\"\"\"\" \"\" \"\"\"\"\"\"\"   ");
}

//-----------------------------------------------------------------------------
void tst_QProcess::spaceArgsTest()
{
    QFETCH(QStringList, args);
    QFETCH(QString, stringArgs);

    QStringList programs;
    programs << QString::fromLatin1("testProcessSpacesArgs/nospace")
             << QString::fromLatin1("testProcessSpacesArgs/one space")
             << QString::fromLatin1("testProcessSpacesArgs/two space s");

    process = new QProcess(this);

    for (int i = 0; i < programs.size(); ++i) {

        QString program = programs.at(i);

        process->start(program, args);

        QVERIFY(process->waitForStarted(5000));
        QVERIFY(process->waitForFinished(5000));

        QStringList actual = QString::fromLatin1(process->readAll()).split("|");
        QVERIFY(!actual.isEmpty());
        // not onterested in the program name, it might be different.
        actual.removeFirst();

        QCOMPARE(actual, args);

        if (program.contains(" "))
            program = "\"" + program + "\"";

        if (!stringArgs.isEmpty())
            program += QString::fromLatin1(" ") + stringArgs;

        process->start(program);

        QVERIFY(process->waitForStarted(5000));
        QVERIFY(process->waitForFinished(5000));

        actual = QString::fromLatin1(process->readAll()).split("|");
        QVERIFY(!actual.isEmpty());
        // not onterested in the program name, it might be different.
        actual.removeFirst();

        QCOMPARE(actual, args);
    }

    delete process;
    process = 0;
}

//-----------------------------------------------------------------------------
void tst_QProcess::exitCodeTest()
{
    for (int i = 0; i < 255; ++i) {
        QProcess process;
        process.start("testExitCodes/testExitCodes " + QString::number(i));
        QVERIFY(process.waitForFinished(5000));
        QCOMPARE(process.exitCode(), i);
        QCOMPARE(process.error(), QProcess::UnknownError);
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::failToStart()
{
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    QProcess process;
    QSignalSpy errorSpy(&process, SIGNAL(error(QProcess::ProcessError)));
    QSignalSpy finishedSpy(&process, SIGNAL(finished(int)));
    QSignalSpy finishedSpy2(&process, SIGNAL(finished(int, QProcess::ExitStatus)));

// Mac OS X has a really low defualt process limit (100), so spawning 
// to many processes here will cause test failures later on.
#ifdef Q_OS_MAC
	const int attempts = 25;
#else
	const int attempts = 50;
#endif

    for (int j = 0; j < 8; ++j) {
        for (int i = 0; i < attempts; ++i) {
            QCOMPARE(errorSpy.count(), j * attempts + i);
            process.start("/blurp");

            switch (j) {
            case 0:
            case 1:
                QVERIFY(!process.waitForStarted());
                break;
            case 2:
            case 3:
                QVERIFY(!process.waitForFinished());
                break;
            case 4:
            case 5:
                QVERIFY(!process.waitForReadyRead());
                break;
            case 6:
            case 7:
            default:
                QVERIFY(!process.waitForBytesWritten());
                break;
            }

            QCOMPARE(process.error(), QProcess::FailedToStart);
            QCOMPARE(errorSpy.count(), j * attempts + i + 1);
            QCOMPARE(finishedSpy.count(), 0);
            QCOMPARE(finishedSpy2.count(), 0);
        }
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::failToStartWithWait()
{
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    QProcess process;
    QEventLoop loop;
    QSignalSpy errorSpy(&process, SIGNAL(error(QProcess::ProcessError)));
    QSignalSpy finishedSpy(&process, SIGNAL(finished(int)));
    QSignalSpy finishedSpy2(&process, SIGNAL(finished(int, QProcess::ExitStatus)));

    for (int i = 0; i < 50; ++i) {
        process.start("/blurp", QStringList() << "-v" << "-debug");
        process.waitForStarted();

        QCOMPARE(process.error(), QProcess::FailedToStart);
        QCOMPARE(errorSpy.count(), i + 1);
        QCOMPARE(finishedSpy.count(), 0);
        QCOMPARE(finishedSpy2.count(), 0);
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::failToStartWithEventLoop()
{
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    QProcess process;
    QEventLoop loop;
    QSignalSpy errorSpy(&process, SIGNAL(error(QProcess::ProcessError)));
    QSignalSpy finishedSpy(&process, SIGNAL(finished(int)));
    QSignalSpy finishedSpy2(&process, SIGNAL(finished(int, QProcess::ExitStatus)));

    // The error signal may be emitted before start() returns
    connect(&process, SIGNAL(error(QProcess::ProcessError)), &loop, SLOT(quit()), Qt::QueuedConnection);


    for (int i = 0; i < 50; ++i) {
        process.start("/blurp", QStringList() << "-v" << "-debug");

        loop.exec();

        QCOMPARE(process.error(), QProcess::FailedToStart);
        QCOMPARE(errorSpy.count(), i + 1);
        QCOMPARE(finishedSpy.count(), 0);
        QCOMPARE(finishedSpy2.count(), 0);
    }
}

//-----------------------------------------------------------------------------
void tst_QProcess::removeFileWhileProcessIsRunning()
{
    QFile file("removeFile.txt");
    QVERIFY(file.open(QFile::WriteOnly));

    QProcess process;
#ifdef Q_OS_MAC
    process.start("testProcessEcho/testProcessEcho.app");
#else
    process.start("testProcessEcho/testProcessEcho");
#endif

    QVERIFY(process.waitForStarted(5000));

    QVERIFY(file.remove());

    process.write("", 1);
    QVERIFY(process.waitForFinished(5000));
}

//-----------------------------------------------------------------------------
void tst_QProcess::systemEnvironment()
{
    QVERIFY(!QProcess::systemEnvironment().isEmpty());
}

//-----------------------------------------------------------------------------
void tst_QProcess::spaceInName()
{
    QProcess process;
    process.start("test Space In Name/testSpaceInName", QStringList());
    QVERIFY(process.waitForStarted());
    process.write("", 1);
    QVERIFY(process.waitForFinished());
}

//-----------------------------------------------------------------------------
void tst_QProcess::lockupsInStartDetached()
{
    QHostInfo::lookupHost(QString("something.invalid"), 0, 0);
    QProcess::execute("yjhbrty");
    QProcess::startDetached("yjhbrty");
}

//-----------------------------------------------------------------------------
void tst_QProcess::atEnd2()
{
    QProcess process;

#ifdef Q_OS_MAC
    process.start("testProcessEcho/testProcessEcho.app");
#else
    process.start("testProcessEcho/testProcessEcho");
#endif
    process.write("Foo\nBar\nBaz\nBodukon\nHadukan\nTorwukan\nend\n");
    process.putChar('\0');
    QVERIFY(process.waitForFinished());
    QList<QByteArray> lines;
    while (!process.atEnd()) {
        lines << process.readLine();
    }
    QCOMPARE(lines.size(), 7);
}

//-----------------------------------------------------------------------------
void tst_QProcess::waitForReadyReadForNonexistantProcess()
{
    // This comes from task 108968
    // Start a program that doesn't exist, process events and then try to waitForReadyRead
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    
    QProcess process;
    QSignalSpy errorSpy(&process, SIGNAL(error(QProcess::ProcessError)));
    QSignalSpy finishedSpy1(&process, SIGNAL(finished(int)));
    QSignalSpy finishedSpy2(&process, SIGNAL(finished(int, QProcess::ExitStatus)));
    QVERIFY(!process.waitForReadyRead()); // used to crash
    process.start("doesntexist");
    QVERIFY(!process.waitForReadyRead());
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.at(0).at(0).toInt(), 0);
    QCOMPARE(finishedSpy1.count(), 0);
    QCOMPARE(finishedSpy2.count(), 0);
}

//-----------------------------------------------------------------------------
void tst_QProcess::setStandardInputFile()
{
    static const char data[] = "A bunch\1of\2data\3\4\5\6\7...";
    QProcess process;
    QFile file("data");
    
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(data, sizeof data);
    file.close();

    process.setStandardInputFile("data");
#ifdef Q_OS_MAC
    process.start("testProcessEcho/testProcessEcho.app");
#else
    process.start("testProcessEcho/testProcessEcho");
#endif

    QPROCESS_VERIFY(process, waitForFinished()); 
	QByteArray all = process.readAll();
    QCOMPARE(all.size(), int(sizeof data) - 1); // testProcessEcho drops the ending \0
    QVERIFY(all == data);
}

//-----------------------------------------------------------------------------
void tst_QProcess::setStandardOutputFile_data()
{
    QTest::addColumn<int>("channelToTest");
    QTest::addColumn<int>("_channelMode");
    QTest::addColumn<bool>("append");

    QTest::newRow("stdout-truncate") << int(QProcess::StandardOutput)
                                     << int(QProcess::SeparateChannels)
                                     << false;
    QTest::newRow("stdout-append") << int(QProcess::StandardOutput)
                                   << int(QProcess::SeparateChannels)
                                   << true;

    QTest::newRow("stderr-truncate") << int(QProcess::StandardError)
                                     << int(QProcess::SeparateChannels)
                                     << false;
    QTest::newRow("stderr-append") << int(QProcess::StandardError)
                                   << int(QProcess::SeparateChannels)
                                   << true;

    QTest::newRow("merged-truncate") << int(QProcess::StandardOutput)
                                     << int(QProcess::MergedChannels)
                                     << false;
    QTest::newRow("merged-append") << int(QProcess::StandardOutput)
                                   << int(QProcess::MergedChannels)
                                   << true;
}

void tst_QProcess::setStandardOutputFile()
{
    static const char data[] = "Original data. ";
    static const char testdata[] = "Test data.";

    QFETCH(int, channelToTest);
    QFETCH(int, _channelMode);
    QFETCH(bool, append);

    QProcess::ProcessChannelMode channelMode = QProcess::ProcessChannelMode(_channelMode);
    QIODevice::OpenMode mode = append ? QIODevice::Append : QIODevice::Truncate;

    // create the destination file with data
    QFile file("data");
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(data, sizeof data - 1);
    file.close();

    // run the process
    QProcess process;
    process.setReadChannelMode(channelMode);
    if (channelToTest == QProcess::StandardOutput)
        process.setStandardOutputFile("data", mode);
    else
        process.setStandardErrorFile("data", mode);

#ifdef Q_OS_MAC
    process.start("testProcessEcho2/testProcessEcho2.app");
#else
    process.start("testProcessEcho2/testProcessEcho2");
#endif
    process.write(testdata, sizeof testdata);
    QPROCESS_VERIFY(process,waitForFinished()); 

    // open the file again and verify the data
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray all = file.readAll();
    file.close();

    int expectedsize = sizeof testdata - 1;
    if (mode == QIODevice::Append) {
        QVERIFY(all.startsWith(data));
        expectedsize += sizeof data - 1;
    }
    if (channelMode == QProcess::MergedChannels) {
        expectedsize += sizeof testdata - 1;
    } else {
        QVERIFY(all.endsWith(testdata));
    }

    QCOMPARE(all.size(), expectedsize);
}

//-----------------------------------------------------------------------------
void tst_QProcess::setStandardOutputProcess_data()
{
    QTest::addColumn<bool>("merged");
    QTest::newRow("separate") << false;
    QTest::newRow("merged") << true;
}

void tst_QProcess::setStandardOutputProcess()
{
    QProcess source;
    QProcess sink;
    
    QFETCH(bool, merged);
    source.setReadChannelMode(merged ? QProcess::MergedChannels : QProcess::SeparateChannels);
    source.setStandardOutputProcess(&sink);

#ifdef Q_OS_MAC
    source.start("testProcessEcho2/testProcessEcho2.app");
    sink.start("testProcessEcho2/testProcessEcho2.app");
#else
    source.start("testProcessEcho2/testProcessEcho2");
    sink.start("testProcessEcho2/testProcessEcho2");
#endif

    QByteArray data("Hello, World");
    source.write(data);
    source.closeWriteChannel();
    QPROCESS_VERIFY(source, waitForFinished());
    QPROCESS_VERIFY(sink, waitForFinished());
    QByteArray all = sink.readAll();

    if (!merged)
        QCOMPARE(all, data);
    else
        // shivery typing...
        QCOMPARE(all, QByteArray("HHeelllloo,,  WWoorrlldd"));
}

void tst_QProcess::fileWriterProcess()
{
    QString stdinStr;
    for (int i = 0; i < 5000; ++i)
        stdinStr += QString::fromLatin1("%1 -- testing testing 1 2 3\n").arg(i);

    QTime stopWatch;
    stopWatch.start();
    do {
        QFile::remove("fileWriterProcess.txt");
        QProcess process;
        process.start("fileWriterProcess/fileWriterProcess",
                      QIODevice::ReadWrite | QIODevice::Text);
        process.write(stdinStr.toLatin1());
        process.closeWriteChannel();
        while (process.bytesToWrite()) {
            QVERIFY(stopWatch.elapsed() < 3500);
            QVERIFY(process.waitForBytesWritten(2000));
        }
        QVERIFY(process.waitForFinished());
        QCOMPARE(QFile("fileWriterProcess.txt").size(), qint64(stdinStr.size()));
    } while (stopWatch.elapsed() < 3000);
}

void tst_QProcess::detachedWorkingDirectoryAndPid()
{
    qint64 pid;

    QFile infoFile(QDir::currentPath() + QLatin1String("/detachedinfo.txt"));

    QString workingDir = QDir::currentPath() + "/testDetached";
    QVERIFY(QFile::exists(workingDir));

    QStringList args;
    args << infoFile.fileName();
    QVERIFY(QProcess::startDetached(QDir::currentPath() + QLatin1String("/testDetached/testDetached"), args, workingDir, &pid));

    while (!infoFile.exists()) {
        QTest::qSleep(100);
    }

    QVERIFY(infoFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString actualWorkingDir = QString::fromUtf8(infoFile.readLine());
    actualWorkingDir.chop(1); // strip off newline
    QByteArray processIdString = infoFile.readLine();
    processIdString.chop(1);
    infoFile.close();
    QVERIFY(infoFile.remove());

    bool ok = false;
    qint64 actualPid = processIdString.toInt(&ok);
    QVERIFY(ok);

    QCOMPARE(actualWorkingDir, workingDir);
    QCOMPARE(actualPid, pid);
}

QTEST_MAIN(tst_QProcess)
#include "tst_qprocess.moc"
