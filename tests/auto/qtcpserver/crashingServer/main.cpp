/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtNetwork>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTcpServer server;
    if (!server.listen(QHostAddress::LocalHost, 49199)) {
        qDebug("Failed to listen: %s", server.errorString().toLatin1().constData());
        return 1;
    }

    printf("Listening\n");
    fflush(stdout);

    server.waitForNewConnection(5000);
    *(char *)0 = 0;
    return 0;
}
