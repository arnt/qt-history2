#include "fortunethread.h"

#include <QtNetwork>

FortuneThread::FortuneThread(int socketDescriptor, const QString &fortune, QObject *parent)
    : QThread(parent), socket(socketDescriptor), text(fortune)
{
}

void FortuneThread::run()
{
    QTcpSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(socket)) {
        emit error(tcpSocket.error());
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (Q_UINT16)0;
    out << text;
    out.device()->seek(0);
    out << (Q_UINT16)(block.size() - sizeof(Q_UINT16));

    tcpSocket.write(block);
    tcpSocket.flush();
    tcpSocket.close();
}
