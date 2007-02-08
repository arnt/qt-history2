/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <private/qnativesocketengine_p.h>

class tst_QSocketNotifier : public QObject
{
    Q_OBJECT
public:
    tst_QSocketNotifier();
    ~tst_QSocketNotifier();

private slots:
    void unexpectedDisconnection();
};

tst_QSocketNotifier::tst_QSocketNotifier()
{ }

tst_QSocketNotifier::~tst_QSocketNotifier()
{
}

class UnexpectedDisconnectTester : public QObject
{
    Q_OBJECT
public:
    QNativeSocketEngine *readEnd1, *readEnd2;
    int sequence;

    UnexpectedDisconnectTester(QNativeSocketEngine *s1, QNativeSocketEngine *s2)
        : readEnd1(s1), readEnd2(s2), sequence(0)
    {
        QSocketNotifier *notifier1 =
            new QSocketNotifier(readEnd1->socketDescriptor(), QSocketNotifier::Read, this);
        connect(notifier1, SIGNAL(activated(int)), SLOT(handleActivated()));
        QSocketNotifier *notifier2 =
            new QSocketNotifier(readEnd2->socketDescriptor(), QSocketNotifier::Read, this);
        connect(notifier2, SIGNAL(activated(int)), SLOT(handleActivated()));
    }

public slots:
    void handleActivated()
    {
        char data1[1], data2[1];
        ++sequence;
        if (sequence == 1) {
            // read from both ends
            (void) readEnd1->read(data1, sizeof(data1));
            (void) readEnd2->read(data2, sizeof(data2));
            emit finished();
        } else if (sequence == 2) {
            // we should never get here
            QCOMPARE(readEnd2->read(data2, sizeof(data2)), qint64(-2));
            QVERIFY(readEnd2->isValid());
        }
    }

signals:
    void finished();
};

void tst_QSocketNotifier::unexpectedDisconnection()
{
    /*
      Given two sockets and two QSocketNotifiers registered on each
      their socket. If both sockets receive data, and the first slot
      invoked by one of the socket notifiers empties both sockets, the
      other notifier will also emit activated(). This results in
      unexpected disconnection in QAbstractSocket.

      The use case is that somebody calls one of the
      waitFor... functions in a QSocketNotifier activated slot, and
      the waitFor... functions do local selects that can empty both
      stdin and stderr while waiting for fex bytes to be written.
    */

    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));

    QNativeSocketEngine readEnd1;
    readEnd1.initialize(QAbstractSocket::TcpSocket);
    bool b = readEnd1.connectToHost(server.serverAddress(), server.serverPort());    
    readEnd1.waitForWrite();
    while (!b)
        b = readEnd1.connectToHost(server.serverAddress(), server.serverPort());
    QVERIFY(readEnd1.state() == QAbstractSocket::ConnectedState);
    QVERIFY(server.waitForNewConnection());    
    QTcpSocket *writeEnd1 = server.nextPendingConnection();
    QVERIFY(writeEnd1 != 0);
    
    QNativeSocketEngine readEnd2;
    readEnd2.initialize(QAbstractSocket::TcpSocket);
    b = readEnd2.connectToHost(server.serverAddress(), server.serverPort());    
    readEnd2.waitForWrite();
    while (!b)
        b = readEnd2.connectToHost(server.serverAddress(), server.serverPort());
    QVERIFY(readEnd2.state() == QAbstractSocket::ConnectedState);
    QVERIFY(server.waitForNewConnection());    
    QTcpSocket *writeEnd2 = server.nextPendingConnection();    
    QVERIFY(writeEnd2 != 0);

    writeEnd1->write("1", 1);
    writeEnd2->write("2", 1);

    writeEnd1->waitForBytesWritten();
    writeEnd2->waitForBytesWritten();

    writeEnd1->flush();
    writeEnd2->flush();

    UnexpectedDisconnectTester tester(&readEnd1, &readEnd2);
    QEventLoop eventLoop;
    connect(&tester, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    QVERIFY(readEnd1.state() == QAbstractSocket::ConnectedState);
    QVERIFY(readEnd2.state() == QAbstractSocket::ConnectedState);
#ifdef Q_OS_WIN
    qWarning("### Windows returns 1 activation, Unix returns 2.");
    QCOMPARE(tester.sequence, 1);
#else
    QCOMPARE(tester.sequence, 2);
#endif

    readEnd1.close();
    readEnd2.close();
    writeEnd1->close();
    writeEnd2->close();
    server.close();
}

QTEST_MAIN(tst_QSocketNotifier)
#include <tst_qsocketnotifier.moc>
