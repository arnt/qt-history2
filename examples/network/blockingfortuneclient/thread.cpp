#include <QtNetwork>

#include "thread.h"

void Thread::requestNewFortune(const QString &hostName, Q_UINT16 port)
{
    this->hostName = hostName;
    this->port = port;
    start();
}

void Thread::run()
{
    const int Timeout = 5 * 1000;

    QTcpSocket socket;
    socket.setBlocking(true, Timeout);
    if (!socket.connectToHost(hostName, port)) {
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
    in.setVersion(7);
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
