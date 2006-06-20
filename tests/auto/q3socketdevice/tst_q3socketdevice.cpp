/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qapplication.h>

#include <q3socketdevice.h>

//TESTED_CLASS=
//TESTED_FILES=network/q3socketdevice.h network/q3socketdevice.cpp network/q3socketdevice_unix.cpp

class tst_Q3SocketDevice : public QObject
{
    Q_OBJECT

public:
    tst_Q3SocketDevice();
    virtual ~tst_Q3SocketDevice();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void readNull();
};

tst_Q3SocketDevice::tst_Q3SocketDevice()
{
}

tst_Q3SocketDevice::~tst_Q3SocketDevice()
{
}

void tst_Q3SocketDevice::initTestCase()
{
}

void tst_Q3SocketDevice::cleanupTestCase()
{
}

void tst_Q3SocketDevice::init()
{
}

void tst_Q3SocketDevice::cleanup()
{
}

void tst_Q3SocketDevice::readNull()
{
    Q3SocketDevice device;
    device.setBlocking(true);

    int attempts = 10;
    while (attempts--) {
        // connect to imap.troll.no
        if (device.connect(QHostAddress("62.70.27.18"), 143))
            break;
    }

    // some static state checking
    QVERIFY(device.isValid());
    QCOMPARE(device.type(), Q3SocketDevice::Stream);
    QCOMPARE(device.protocol(), Q3SocketDevice::IPv4);
    QVERIFY(device.socket() != -1);
    QVERIFY(device.blocking());
#if defined Q_OS_IRIX
    // IRIX defaults to the opposite in Qt 3, so we won't fix
    // this in Qt 4.
    QVERIFY(device.addressReusable());
#else
    QVERIFY(!device.addressReusable());
#endif
    QCOMPARE(device.peerPort(), quint16(143));
    QCOMPARE(device.peerAddress().toString(),
            QHostAddress("62.70.27.18").toString());
    QCOMPARE(device.error(), Q3SocketDevice::NoError);

    // write a logout notice
    QCOMPARE(device.writeBlock("X LOGOUT\r\n", Q_ULONG(10)), Q_LONG(10));

    // expect three lines of response: greeting, bye-warning and
    // logout command completion.
    int ch;
    for (int i = 0; i < 3; ++i) {
        do {
            QVERIFY((ch = device.getch()) != -1);
        } while (char(ch) != '\n');
    }

    // here, read() will return 0.
    char c;
    QCOMPARE(device.readBlock(&c, 1), qint64(0));
    QVERIFY(!device.isValid());
}

QTEST_APPLESS_MAIN(tst_Q3SocketDevice)
#include "tst_q3socketdevice.moc"
