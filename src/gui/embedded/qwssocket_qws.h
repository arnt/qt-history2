/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSSOCKET_QWS_H
#define QWSSOCKET_QWS_H

#include "QtNetwork/qtcpsocket.h"
#include "QtNetwork/qtcpserver.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MULTIPROCESS

class QWSSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit QWSSocket(QObject *parent=0);
    ~QWSSocket();

    void connectToLocalFile(const QString &file);

private:
    Q_DISABLE_COPY(QWSSocket)
};


class QWSServerSocket : public QTcpServer
{
    Q_OBJECT
public:
    QWSServerSocket(const QString& file, QObject *parent=0);
    ~QWSServerSocket();

private:
    Q_DISABLE_COPY(QWSServerSocket)

    void init(const QString &file);
};

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSSOCKET_QWS_H
