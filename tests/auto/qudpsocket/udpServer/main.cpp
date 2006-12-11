#include <QtNetwork>

class Server : public QObject
{
    Q_OBJECT
public:
    Server(int port)
    {
        connect(&serverSocket, SIGNAL(readyRead()),
                this, SLOT(sendEcho()));
        if (serverSocket.bind(QHostAddress::Any, port,
                              QUdpSocket::ReuseAddressHint
                              | QUdpSocket::ShareAddress)) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
        }
        fflush(stdout);
    }

private slots:
    void sendEcho()
    {
        QHostAddress senderAddress;
        quint16 senderPort;

        char data[1024];
        qint64 bytes = serverSocket.readDatagram(data, sizeof(data), &senderAddress, &senderPort);
        if (bytes == 1 && data[0] == '\0')
            QCoreApplication::instance()->quit();
        
        for (int i = 0; i < bytes; ++i)
            data[i] += 1;
        serverSocket.writeDatagram(data, bytes, senderAddress, senderPort);        
    }

private:
    QUdpSocket serverSocket;
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    Server server(app.arguments().at(1).toInt());
    
    return app.exec();
}

#include "main.moc"
