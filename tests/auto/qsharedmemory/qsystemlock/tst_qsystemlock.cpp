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
#include <qsystemlock.h>

//TESTED_CLASS=QSystemLock
//TESTED_FILES=core/io/qsystemlock.h gcore/io/qsystemlock.cpp

#define EXISTING_SHARE "existing"

class tst_QSystemLock : public QObject
{
    Q_OBJECT

public:
    tst_QSystemLock();
    virtual ~tst_QSystemLock();

public Q_SLOTS:
    void init();
    void cleanup();

private slots:
    void key_data();
    void key();

    void basicLock();
    void complexLock();
    void lockModes();
    void sucessive();
    void processes_data();
    void processes();

private:
    QSystemLock *existingLock;

};

tst_QSystemLock::tst_QSystemLock()
{
}

tst_QSystemLock::~tst_QSystemLock()
{
}

void tst_QSystemLock::init()
{
    existingLock = new QSystemLock(EXISTING_SHARE);
}

void tst_QSystemLock::cleanup()
{
    delete existingLock;
}

void tst_QSystemLock::key_data()
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
void tst_QSystemLock::key()
{
    QFETCH(QString, constructorKey);
    QFETCH(QString, setKey);

    QSystemLock sl(constructorKey);
    QCOMPARE(sl.key(), constructorKey);
    sl.setKey(setKey);
    QCOMPARE(sl.key(), setKey);
}

void tst_QSystemLock::basicLock()
{
    QSystemLock lock("foo");
    QVERIFY(lock.lock());
    QVERIFY(lock.unlock());
}

void tst_QSystemLock::complexLock()
{
    QSystemLock lock("foo");
    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.unlock());

    QVERIFY(lock.lock(QSystemLock::ReadWrite));
    QVERIFY(lock.unlock());

    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.unlock());
    QVERIFY(lock.unlock());
}

void tst_QSystemLock::lockModes()
{
    QSystemLock reader1("library");
    QSystemLock reader2("library");

    QSystemLock librarian("library");
    QVERIFY(reader1.lock(QSystemLock::ReadOnly));
    QVERIFY(reader2.lock(QSystemLock::ReadOnly));
    QVERIFY(reader1.unlock());
    QVERIFY(reader2.unlock());
    QVERIFY(librarian.lock(QSystemLock::ReadWrite));
    QVERIFY(librarian.unlock());
}

void tst_QSystemLock::sucessive()
{
    QSystemLock lock("library");
    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.lock(QSystemLock::ReadOnly));
    QVERIFY(lock.unlock());
    QVERIFY(lock.unlock());
    QVERIFY(lock.unlock());
    QVERIFY(lock.unlock());
    QVERIFY(lock.unlock());
    QVERIFY(!lock.unlock());
}

void tst_QSystemLock::processes_data()
{
    QTest::addColumn<int>("readOnly");
    QTest::addColumn<int>("readWrite");
    for (int i = 0; i < 5; ++i) {
        QTest::newRow("1/0 process") << 1 << 0;
        QTest::newRow("0/1 process") << 0 << 1;
    	QTest::newRow("0/4 process") << 0 << 4;
        QTest::newRow("1/1 process") << 1 << 1;
        QTest::newRow("10/1 process") << 10 << 1;
        QTest::newRow("1/10 process") << 1 << 10;
        QTest::newRow("10/10 processes") << 10 << 10;
    }
}

/*!
    Create external processes
 */
void tst_QSystemLock::processes()
{
    QFETCH(int, readOnly);
    QFETCH(int, readWrite);

    QStringList scripts;
    for (int i = 0; i < readOnly; ++i)
        scripts.append("../lackey/scripts/systemlock_read.js");
    for (int i = 0; i < readWrite; ++i)
        scripts.append("../lackey/scripts/systemlock_readwrite.js");

    QList<QProcess*> consumers;
    for (int i = 0; i < scripts.count(); ++i) {
        QStringList arguments = QStringList() << scripts.at(i);
        QProcess *p = new QProcess;
        p->setProcessChannelMode(QProcess::ForwardedChannels);
        consumers.append(p);
	p->start("../lackey/lackey", arguments);
    }

    while (!consumers.isEmpty()) {
        consumers.first()->waitForFinished(3000);
	consumers.first()->kill();
        QCOMPARE(consumers.first()->exitStatus(), QProcess::NormalExit);
        QCOMPARE(consumers.first()->exitCode(), 0);
        delete consumers.takeFirst();
    }
}

QTEST_MAIN(tst_QSystemLock)
#include "tst_qsystemlock.moc"

