#include <QtNetwork>

#include "fortunethread.h"

FortuneThread::FortuneThread(QObject *parent)
    : QThread(parent), quit(false)
{
}

FortuneThread::~FortuneThread()
{
    quit = true;
    cond.wakeOne();
    wait();
}

void FortuneThread::requestNewFortune(const QString &hostName, Q_UINT16 port)
{
    QMutexLocker locker(&mutex);
    this->hostName = hostName;
    this->port = port;
    if (!isRunning())
        start();
    else
        cond.wakeOne();
}

void FortuneThread::run()
{
    QString serverName = hostName;
    Q_UINT16 serverPort = port;

    while (!quit) {
        const int Timeout = 5 * 1000;

        QTcpSocket socket;
        socket.connectToHost(serverName, serverPort);

        if (!socket.waitForConnected(Timeout)) {
            emit error(socket.socketError(), socket.errorString());
            return;
        }

        while (socket.bytesAvailable() < sizeof(Q_UINT16)) {
            if (!socket.waitForReadyRead(Timeout)) {
                emit error(socket.socketError(), socket.errorString());
                return;
            }
        }

        Q_UINT16 blockSize;
        QDataStream in(&socket);
        in.setVersion(QDataStream::Qt_4_0);
        in >> blockSize;

        while (socket.bytesAvailable() < blockSize) {
            if (!socket.waitForReadyRead(Timeout)) {
                emit error(socket.socketError(), socket.errorString());
                return;
            }
        }

        QMutexLocker locker(&mutex);

        QString fortune;
        in >> fortune;
        emit newFortune(fortune);

        cond.wait(&mutex);
        serverName = hostName;
        serverPort = port;
    }
}
