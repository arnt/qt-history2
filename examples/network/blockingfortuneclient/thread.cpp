#include <QtNetwork>

#include "thread.h"

void FortuneThread::run()
{
    QTcpSocket socket;
    socket.setBlocking(true, 5000);
    if (!socket.connectToHost(hostName, port)) {
        emit error(socket.socketError(), socket.errorString());
        return;
    }

    while (socket.bytesAvailable() < sizeof(Q_UINT16)) {
        if (!socket.waitForReadyRead(5000)) {
            emit error(socket.socketError(),
                       socket.errorString());
            return;
        }
    }

    Q_UINT16 blockSize;
    QDataStream stream(&socket);
    stream.setVersion(7);
    stream >> blockSize;

    while (socket.bytesAvailable() < blockSize) {
        if (!socket.waitForReadyRead(5000)) {
            emit error(socket.socketError(),
                       socket.errorString());
            return;
        }
    }

    QString fortune;
    stream >> fortune;

    emit newFortune(fortune);
}

void FortuneThread::requestNewFortune(const QString &hostName, Q_UINT16 port)
{
    this->hostName = hostName;
    this->port = port;
    start();
}
