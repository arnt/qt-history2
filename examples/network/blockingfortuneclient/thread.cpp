#include <QtNetwork>

#include "fortunethread.h"

void FortuneThread::requestNewFortune(const QString &hostName, Q_UINT16 port)
{
    QMutexLocker lock(&mutex);
    this->hostName = hostName;
    this->port = port;
    start();
}

void FortuneThread::run()
{
    const int Timeout = 5 * 1000;

    QTcpSocket socket;

    mutex.lock();
    socket.connectToHost(hostName, port);
    mutex.unlock();

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

    QString fortune;
    in >> fortune;
    emit newFortune(fortune);
}
