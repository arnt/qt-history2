/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsystemsemaphore.h>

//TESTED_CLASS=QSystemSemaphore
//TESTED_FILES=core/io/qsystemsemaphore.h gcore/io/qsystemsemaphore.cpp

#define EXISTING_SHARE "existing"

#define LACKYLOC "../qsharedmemory/lackey"

class tst_QSystemSemaphore : public QObject
{
    Q_OBJECT

public:
    tst_QSystemSemaphore();
    virtual ~tst_QSystemSemaphore();

public Q_SLOTS:
    void init();
    void cleanup();

private slots:
    void key_data();
    void key();

    void basicacquire();
    void complexacquire();

    void basicProcesses();

    void processes_data();
    void processes();

    void undo();
    void initialValue();

private:
    QSystemSemaphore *existingLock;

};

tst_QSystemSemaphore::tst_QSystemSemaphore()
{
}

tst_QSystemSemaphore::~tst_QSystemSemaphore()
{
}

void tst_QSystemSemaphore::init()
{
    existingLock = new QSystemSemaphore(EXISTING_SHARE, 1);
}

void tst_QSystemSemaphore::cleanup()
{
    delete existingLock;
}

void tst_QSystemSemaphore::key_data()
{
    QTest::addColumn<QString>("constructorKey");
    QTest::addColumn<QString>("setKey");

    QTest::newRow("null, null") << QString() << QString();
    QTest::newRow("null, one") << QString() << QString("one");
    QTest::newRow("one, two") << QString("one") << QString("two");
}

/*!
    Basic key testing
 */
void tst_QSystemSemaphore::key()
{
    QFETCH(QString, constructorKey);
    QFETCH(QString, setKey);

    QSystemSemaphore sl(constructorKey);
    QCOMPARE(sl.key(), constructorKey);
    sl.setKey(setKey);
    QCOMPARE(sl.key(), setKey);
}

void tst_QSystemSemaphore::basicacquire()
{
    QSystemSemaphore sem("foo", 1);
    QVERIFY(sem.acquire());
    QVERIFY(sem.release());
}

void tst_QSystemSemaphore::complexacquire()
{
    QSystemSemaphore sem("foo", 2);
    QVERIFY(sem.acquire());
    QVERIFY(sem.release());
    QVERIFY(sem.acquire());
    QVERIFY(sem.release());
    QVERIFY(sem.acquire());
    QVERIFY(sem.acquire());
    QVERIFY(sem.release());
    QVERIFY(sem.release());
}

void tst_QSystemSemaphore::basicProcesses()
{
    QSystemSemaphore sem("store", 0);

    QStringList acquireArguments = QStringList() << LACKYLOC "/scripts/systemsemaphore_acquire.js";
    QProcess acquire;
    acquire.setProcessChannelMode(QProcess::ForwardedChannels);

    QStringList releaseArguments = QStringList() << LACKYLOC "/scripts/systemsemaphore_release.js";
    QProcess release;
    release.setProcessChannelMode(QProcess::ForwardedChannels);

    acquire.start(LACKYLOC "/lackey", acquireArguments);
    QVERIFY(!acquire.waitForFinished(100));
    QVERIFY(acquire.state()== QProcess::Running);
    acquire.kill();
    release.start(LACKYLOC "/lackey", releaseArguments);
    QVERIFY(acquire.waitForFinished(100));
    QVERIFY(acquire.state()== QProcess::NotRunning);
}

void tst_QSystemSemaphore::processes_data()
{
    QTest::addColumn<int>("processes");
    for (int i = 0; i < 5; ++i) {
        QTest::newRow("1 process") << 1;
        QTest::newRow("3 process") << 3;
        QTest::newRow("10 process") << 10;
    }
}

void tst_QSystemSemaphore::processes()
{
    QSystemSemaphore sem("store", 1);

    QFETCH(int, processes);
    QStringList scripts;
    for (int i = 0; i < processes; ++i)
        scripts.append(LACKYLOC "/scripts/systemsemaphore_acquirerelease.js");

    QList<QProcess*> consumers;
    for (int i = 0; i < scripts.count(); ++i) {
        QStringList arguments = QStringList() << scripts.at(i);
        QProcess *p = new QProcess;
        p->setProcessChannelMode(QProcess::ForwardedChannels);
        consumers.append(p);
        p->start(LACKYLOC "/lackey", arguments);
    }

    while (!consumers.isEmpty()) {
        consumers.first()->waitForFinished();
        QCOMPARE(consumers.first()->exitStatus(), QProcess::NormalExit);
        QCOMPARE(consumers.first()->exitCode(), 0);
        delete consumers.takeFirst();
    }
}
void tst_QSystemSemaphore::undo()
{
#ifdef Q_OS_WIN
    QSKIP("This test only checks a unix behavior", SkipSingle);
#endif

    QSystemSemaphore sem("store", 1);

    QStringList acquireArguments = QStringList() << LACKYLOC "/scripts/systemsemaphore_acquire.js";
    QProcess acquire;
    acquire.setProcessChannelMode(QProcess::ForwardedChannels);

    QStringList releaseArguments = QStringList() << LACKYLOC "/scripts/systemsemaphore_release.js";
    QProcess release;
    release.setProcessChannelMode(QProcess::ForwardedChannels);

    acquire.start(LACKYLOC "/lackey", acquireArguments);
    QVERIFY(acquire.waitForFinished(100));
    QVERIFY(acquire.state()== QProcess::NotRunning);

    // At process exit the kernel should auto undo

    acquire.start(LACKYLOC "/lackey", acquireArguments);
    QVERIFY(acquire.waitForFinished(100));
    QVERIFY(acquire.state()== QProcess::NotRunning);
}

void tst_QSystemSemaphore::initialValue()
{
    QSystemSemaphore sem("store", 1);

    QStringList acquireArguments = QStringList() << LACKYLOC "/scripts/systemsemaphore_acquire.js";
    QProcess acquire;
    acquire.setProcessChannelMode(QProcess::ForwardedChannels);

    QStringList releaseArguments = QStringList() << LACKYLOC "/scripts/systemsemaphore_release.js";
    QProcess release;
    release.setProcessChannelMode(QProcess::ForwardedChannels);

    acquire.start(LACKYLOC "/lackey", acquireArguments);
    QVERIFY(acquire.waitForFinished(100));
    QVERIFY(acquire.state()== QProcess::NotRunning);

    acquire.start(LACKYLOC "/lackey", acquireArguments << "2");
    QVERIFY(!acquire.waitForFinished(100));
    QVERIFY(acquire.state()== QProcess::Running);
    acquire.kill();

    release.start(LACKYLOC "/lackey", releaseArguments);
    QVERIFY(acquire.waitForFinished(100));
    QVERIFY(acquire.state()== QProcess::NotRunning);
}
QTEST_MAIN(tst_QSystemSemaphore)
#include "tst_qsystemsemaphore.moc"

