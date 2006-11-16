/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_QGlobal: public QObject
{
    Q_OBJECT
private slots:
    void qIsNull();
    void qInternalCallbacks();
    void for_each();
};

void tst_QGlobal::qIsNull()
{
    double d = 0.0;
    float f = 0.0f;

    QVERIFY(::qIsNull(d));
    QVERIFY(::qIsNull(f));

    d += 0.000000001;
    f += 0.0000001f;

    QVERIFY(!::qIsNull(d));
    QVERIFY(!::qIsNull(f));
}

struct {
    QObject *sender;
    QObject *receiver;
    QString signal, slot;
    int type;
    void reset() {
        sender = receiver = 0;
        signal = slot = QString();
        type = -1;
    }
} connect_info;

bool disconnect_callback(void **data)
{
    connect_info.sender = (QObject *)(data[0]);
    connect_info.receiver = (QObject *)(data[2]);
    connect_info.signal = QString::fromLatin1((const char *) data[1]);
    connect_info.slot = QString::fromLatin1((const char *) data[3]);
    return true;
}

bool connect_callback(void **data)
{
    disconnect_callback(data);
    connect_info.type = *(int *) data[4];
    return true;
}

void tst_QGlobal::qInternalCallbacks()
{
    QInternal::registerCallback(QInternal::ConnectCallback, connect_callback);
    QInternal::registerCallback(QInternal::DisconnectCallback, disconnect_callback);

    QObject a, b;
    QString signal = QLatin1String("2mysignal(x)");
    QString slot = QLatin1String("1myslot(x)");

    // Test that connect works as expected...
    connect_info.reset();
    bool ok = QObject::connect(&a, signal.toLatin1(), &b, slot.toLatin1(), Qt::AutoConnection);
    QVERIFY(ok);
    QCOMPARE(&a, connect_info.sender);
    QCOMPARE(&b, connect_info.receiver);
    QCOMPARE(signal, connect_info.signal);
    QCOMPARE(slot, connect_info.slot);
    QCOMPARE((int) Qt::AutoConnection, connect_info.type);

    // Test that disconnect works as expected
    connect_info.reset();
    ok = QObject::disconnect(&a, signal.toLatin1(), &b, slot.toLatin1());
    QVERIFY(ok);
    QCOMPARE(&a, connect_info.sender);
    QCOMPARE(&b, connect_info.receiver);
    QCOMPARE(signal, connect_info.signal);
    QCOMPARE(slot, connect_info.slot);

    // Unregister callbacks and verify that they are not triggered...
    QInternal::unregisterCallback(QInternal::ConnectCallback, connect_callback);
    QInternal::unregisterCallback(QInternal::DisconnectCallback, disconnect_callback);

    connect_info.reset();
    ok = QObject::connect(&a, signal.toLatin1(), &b, slot.toLatin1(), Qt::AutoConnection);
    QVERIFY(!ok);
    QCOMPARE(connect_info.sender, (QObject *) 0);

    ok = QObject::disconnect(&a, signal.toLatin1(), &b, slot.toLatin1());
    QVERIFY(!ok);
    QCOMPARE(connect_info.sender, (QObject *) 0);
}

void tst_QGlobal::for_each()
{
    QList<int> list;
    list << 0 << 1 << 2 << 3 << 4 << 5;

    int counter = 0;
    foreach(int i, list) {
        QCOMPARE(i, counter++);
    }
    QCOMPARE(counter, list.count());

    // do it again, to make sure we don't have any for-scoping
    // problems with older compilers
    counter = 0;
    foreach(int i, list) {
        QCOMPARE(i, counter++);
    }
    QCOMPARE(counter, list.count());
}

QTEST_MAIN(tst_QGlobal)
#include "tst_qglobal.moc"
