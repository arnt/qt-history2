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

#include <qtextstream.h>
#include <qlocalsocket.h>
#include <qlocalserver.h>

//TESTED_CLASS=QLocalServer, QLocalSocket
//TESTED_FILES=tests/auto/qlocalsocket/src/qlocalserver.cpp tests/auto/qlocalsocket/src/qlocalsocket.cpp

class tst_QLocalSocket : public QObject
{
    Q_OBJECT

public:
    tst_QLocalSocket();
    virtual ~tst_QLocalSocket();

public Q_SLOTS:
    void init();
    void cleanup();

private slots:
    // basics
    void server_basic();
    void server_connectionsCount();
    void socket_basic();

    void listen_data();
    void listen();

    void listenAndConnect_data();
    void listenAndConnect();

    void sendData_data();
    void sendData();

    void hitMaximumConnections_data();
    void hitMaximumConnections();

    void setSocketDescriptor();

    void threadedConnection_data();
    void threadedConnection();

    void processConnection_data();
    void processConnection();

protected:

};

tst_QLocalSocket::tst_QLocalSocket()
{
}

tst_QLocalSocket::~tst_QLocalSocket()
{
}

void tst_QLocalSocket::init()
{
}

void tst_QLocalSocket::cleanup()
{
}

class LocalServer : public QLocalServer
{
public:
    QList<int> hits;

protected:
    void incomingConnection(int socketDescriptor)
    {
        hits.append(socketDescriptor);
        QLocalServer::incomingConnection(socketDescriptor);
    }

};

void tst_QLocalSocket::server_basic()
{
    LocalServer server;
    QSignalSpy spy(&server, SIGNAL(newConnection()));
    QCOMPARE(server.close(), false);
    QCOMPARE(server.errorString(), QString());
    QCOMPARE(server.hasPendingConnections(), false);
    QCOMPARE(server.isListening(), false);
    QCOMPARE(server.serverError(), QLocalServer::NoError);
    QCOMPARE(server.maxPendingConnections(), 30);
    QCOMPARE(server.nextPendingConnection(), (QLocalSocket*)0);
    QCOMPARE(server.serverName(), QString());
    QCOMPARE(server.listen(QString()), false);
    QCOMPARE(server.hits.count(), 0);
    QCOMPARE(server.waitForNewConnection(0), false);
    QCOMPARE(spy.count(), 0);
}

void tst_QLocalSocket::server_connectionsCount()
{
    LocalServer server;
    server.setMaxPendingConnections(10);
    QCOMPARE(server.maxPendingConnections(), 10);
}

void tst_QLocalSocket::socket_basic()
{
    QLocalSocket socket;
    QCOMPARE(socket.peerName(), QString());
    QVERIFY(socket.bytesAvailable() == 0);
    QVERIFY(socket.bytesToWrite() == 0);
    QCOMPARE(socket.canReadLine(), false);
    QCOMPARE(socket.flush(), false);
    QCOMPARE(socket.isValid(), false);
    QVERIFY(socket.readBufferSize() == 0);
    socket.setReadBufferSize(0);
    QCOMPARE(socket.socketDescriptor(), -1);
    QCOMPARE(socket.state(), QLocalSocket::UnconnectedState);
    QCOMPARE(socket.waitForConnected(0), false);
    QCOMPARE(socket.waitForDisconnected(0), false);
    QCOMPARE(socket.waitForReadyRead(0), false);
    socket.close();
    socket.disconnect();
    socket.error();
    socket.errorString();
}

void tst_QLocalSocket::listen_data()
{
    listenAndConnect_data();
}

void tst_QLocalSocket::listen()
{
    LocalServer server;
    QLocalSocket socket;
    QSignalSpy spy(&server, SIGNAL(newConnection()));

    QFETCH(QString, name);
    QFETCH(bool, canListen);
    QCOMPARE(server.listen(name), canListen);

    // test listening
    QCOMPARE(server.serverName(), name);
    QCOMPARE(server.isListening(), canListen);
    QCOMPARE(server.hasPendingConnections(), false);
    QCOMPARE(server.nextPendingConnection(), (QLocalSocket*)0);
    QCOMPARE(server.hits.count(), 0);
    QCOMPARE(spy.count(), 0);
    if (canListen) {
        QVERIFY(server.errorString() == QString());
        QCOMPARE(server.serverError(), QLocalServer::NoError);
    } else {
        QVERIFY(server.errorString() != QString());
        QCOMPARE(server.serverError(), QLocalServer::NameError);
    }
}

void tst_QLocalSocket::listenAndConnect_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("canListen");
    QTest::addColumn<int>("connections");

    for (int i = 0; i < 3; ++i) {
        int connections = i;
        if (i == 2) connections = 5;
        QTest::newRow(QString("null %1").arg(i).toLatin1()) << QString() << false << connections;
        QTest::newRow(QString("tst_localsocket %1").arg(i).toLatin1()) << "tst_localsocket" << true << connections;
    }
}

void tst_QLocalSocket::listenAndConnect()
{
    LocalServer server;
    QLocalSocket socket;
    QSignalSpy spy(&server, SIGNAL(newConnection()));

    QFETCH(QString, name);
    QFETCH(bool, canListen);

    QCOMPARE(server.listen(name), canListen);
    //QCOMPARE(server.errorString(), canListen ? QString() : "name error");
    QCOMPARE(server.serverError(), canListen ? QLocalServer::NoError : QLocalServer::NameError);

    // test creating a connection
    QFETCH(int, connections);
    QList<QLocalSocket*> sockets;
    for (int i = 0; i < connections; ++i) {
        QLocalSocket *socket = new QLocalSocket;
        socket->connectToName(name);
        QCOMPARE(socket->peerName(), name);
        sockets.append(socket);
        QCOMPARE(server.waitForNewConnection(-1), canListen);
        QCOMPARE(server.hasPendingConnections(), canListen);
        if (canListen) {
            QCOMPARE(socket->errorString(), QString("Unknown error"));
            QCOMPARE(socket->error(), QLocalSocket::UnknownSocketError);
        } else {
            QVERIFY(socket->errorString() != QString());
            QVERIFY(socket->error() != QLocalSocket::UnknownSocketError);
        }
    }
    QCOMPARE(server.hasPendingConnections(), (canListen ? connections > 0 : false));
    qDeleteAll(sockets.begin(), sockets.end());

    QCOMPARE(server.close(), canListen);

    QCOMPARE(server.hits.count(), (canListen ? qMin(connections, 1) : 0));
    QCOMPARE(spy.count(), (canListen ? qMin(connections, 1) : 0));
}

void tst_QLocalSocket::sendData_data()
{
    listenAndConnect_data();
}

void tst_QLocalSocket::sendData()
{
    qRegisterMetaType<QLocalSocket::LocalSocketState>("QLocalSocket::LocalSocketState");
    qRegisterMetaType<QLocalSocket::LocalSocketError>("QLocalSocket::LocalSocketError");
    LocalServer server;
    QLocalSocket socket;
    QSignalSpy spy(&server, SIGNAL(newConnection()));

    QFETCH(QString, name);
    QFETCH(bool, canListen);

    QCOMPARE(server.listen(name), canListen);

    QSignalSpy spyConnected(&socket, SIGNAL(connected()));
    QSignalSpy spyDisconnected(&socket, SIGNAL(disconnected()));
    QSignalSpy spyError(&socket, SIGNAL(error(QLocalSocket::LocalSocketError)));
    QSignalSpy spyStateChanged(&socket, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)));

    // test creating a connection
    socket.connectToName(name);
    QCOMPARE(server.waitForNewConnection(-1), canListen);
    QCOMPARE(spyConnected.count(), canListen ? 1 : 0);
    QCOMPARE(socket.state(), canListen ? QLocalSocket::ConnectedState : QLocalSocket::UnconnectedState);

    // test sending/receiving data
    if (server.hasPendingConnections()) {
        QString testLine = "test";
        QLocalSocket *serverSocket = server.nextPendingConnection();
        QVERIFY(serverSocket);
        QCOMPARE(serverSocket->state(), QLocalSocket::ConnectedState);
        QTextStream out(serverSocket);
        QTextStream in(&socket);
        out << testLine << endl;
        if (!socket.canReadLine())
            QVERIFY(socket.waitForReadyRead());
        QCOMPARE(in.readLine(), testLine);
        QVERIFY(serverSocket->waitForBytesWritten(1000));
        QCOMPARE(serverSocket->errorString(), QString("Unknown error"));
        QCOMPARE(socket.errorString(), QString("Unknown error"));
    }

    socket.close();

    QCOMPARE(server.close(), canListen);
    QCOMPARE(spyDisconnected.count(), canListen ? 1 : 0);
    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyStateChanged.count(), canListen ? 4 : 2);

    QCOMPARE(server.hits.count(), (canListen ? 1 : 0));
    QCOMPARE(spy.count(), (canListen ? 1 : 0));
}

void tst_QLocalSocket::hitMaximumConnections_data()
{
    QTest::addColumn<int>("max");
    QTest::newRow("none") << 0;
    QTest::newRow("1") << 1;
    QTest::newRow("3") << 3;
}

void tst_QLocalSocket::hitMaximumConnections()
{
    QFETCH(int, max);
    LocalServer server;
    QString name = "tst_localsocket";
    server.setMaxPendingConnections(max);
    QCOMPARE(server.listen(name), true);
    int connections = server.maxPendingConnections() + 1;
    QList<QLocalSocket*> sockets;
    for (int i = 0; i < connections; ++i) {
        QLocalSocket *socket = new QLocalSocket;
        sockets.append(socket);
        socket->connectToName(name);
    }
    qDeleteAll(sockets.begin(), sockets.end());
}

void tst_QLocalSocket::setSocketDescriptor()
{
    QLocalSocket socket;
    socket.setSocketDescriptor(-1, QLocalSocket::ConnectingState, QIODevice::Append);
    QCOMPARE(socket.socketDescriptor(), -1);
    QCOMPARE(socket.state(), QLocalSocket::ConnectingState);
    QVERIFY((socket.openMode() & QIODevice::Append) != 0);
}

class Client : public QThread
{

public:
    void run()
    {
        QString testLine = "test";
        QLocalSocket socket;
        socket.connectToName("test");
        QVERIFY(socket.waitForConnected(10000));
        QVERIFY(socket.state() == QLocalSocket::ConnectedState);
	QVERIFY(socket.waitForReadyRead());
        QTextStream in(&socket);
        QCOMPARE(in.readLine(), testLine);
        QCOMPARE(socket.errorString(), QString("Unknown error"));
        socket.close();
    }
};

class Server : public QThread
{

public:
    int clients;
    void run()
    {
        QString testLine = "test";
        LocalServer server;
        QVERIFY(server.listen("test"));
        int done = clients;
        while (done > 0) {
            QVERIFY(server.waitForNewConnection(30000));
            QLocalSocket *serverSocket = server.nextPendingConnection();
            QVERIFY(serverSocket);
            QTextStream out(serverSocket);
            out << testLine << endl;
            QVERIFY(serverSocket->waitForBytesWritten(300000));
            QCOMPARE(serverSocket->errorString(), QString("Unknown error"));
            --done;
            delete serverSocket;
        }
        QCOMPARE(server.hits.count(), clients);
    }
};

void tst_QLocalSocket::threadedConnection_data()
{
    QTest::addColumn<int>("threads");
    QTest::newRow("1 client") << 1;
    QTest::newRow("2 clients") << 2;
    QTest::newRow("5 clients") << 5;
    QTest::newRow("10 clients") << 10;
}

void tst_QLocalSocket::threadedConnection()
{
    QFETCH(int, threads);
    Server server;
    server.clients = threads;
    server.start();

    QList<Client*> clients;
    for (int i = 0; i < threads; ++i) {
        clients.append(new Client());
        clients.last()->start();
    }

    server.wait();
    while (!clients.isEmpty()) {
        QVERIFY(clients.first()->wait(1000));
        delete clients.takeFirst();
    }
}

void tst_QLocalSocket::processConnection_data()
{
    QTest::addColumn<int>("processes");
    QTest::newRow("1 client") << 1;
    QTest::newRow("2 clients") << 2;
    QTest::newRow("5 clients") << 5;
    QTest::newRow("10 clients") << 10;
}

/*!
    Create external processes that produce and consume.
 */
void tst_QLocalSocket::processConnection()
{
    QFETCH(int, processes);
    QStringList serverArguments = QStringList() << "lackey/scripts/server.js" << QString::number(processes);
    QProcess producer;
    producer.setProcessChannelMode(QProcess::ForwardedChannels);
#ifdef Q_WS_QWS
    serverArguments << "-qws";
#endif
    QList<QProcess*> consumers;
    producer.start("lackey/lackey", serverArguments);
    QVERIFY(producer.waitForStarted(-1));
    QTest::qWait(250);
    for (int i = 0; i < processes; ++i) {
       QStringList arguments = QStringList() << "lackey/scripts/client.js";
#ifdef Q_WS_QWS
       arguments << "-qws";
#endif
        QProcess *p = new QProcess;
        p->setProcessChannelMode(QProcess::ForwardedChannels);
        consumers.append(p);
        p->start("lackey/lackey", arguments);
    }

    while (!consumers.isEmpty()) {
        consumers.first()->waitForFinished(-1);
        QCOMPARE(consumers.first()->exitStatus(), QProcess::NormalExit);
        QCOMPARE(consumers.first()->exitCode(), 0);
        delete consumers.takeFirst();
    }
    producer.waitForFinished(-1);
}

QTEST_MAIN(tst_QLocalSocket)
#include "tst_qlocalsocket.moc"

