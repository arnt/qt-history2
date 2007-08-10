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

#include "qlocalserver.h"
#include "qlocalsocket.h"

#include <qapplication.h>
#include <qdebug.h>

class EchoServer : public QLocalServer
{
public:
    void incomingConnection(int socketDescriptor) {
        QLocalServer::incomingConnection(socketDescriptor);
        QLocalSocket *socket = nextPendingConnection();
        socket->open(QIODevice::ReadWrite);

        qDebug() << "server connection";

        do {
            const int Timeout = 5 * 1000;
            while (!socket->canReadLine()) {
                if (!socket->waitForReadyRead(Timeout)) {
                    return;
                }
            }
            char str[100];
            int n = socket->readLine(str, 100);
	    if (n < 0) {
                perror("recv");
                break;
            }
            if (n == 0)
                break;
	    qDebug() << "Read" << str;
            if ("exit" == str)
                qApp->quit();

            if (socket->write(str, 100) < 0) {
                perror("send");
                break;
            }
        } while (true);
    }
};

#define SOCK_PATH "echo_socket"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setQuitOnLastWindowClosed(false);

    EchoServer echoServer;
    echoServer.listen(SOCK_PATH);

    return application.exec();
}

