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

#ifndef QSOUNDQSS_QWS_H
#define QSOUNDQSS_QWS_H

#include "qtcpserver.h"
#include "qtcpsocket.h"

#ifndef QT_NO_SOUND

#ifndef Q_OS_MAC

class QWSSoundServerPrivate;

class QWSSoundServer : public QObject {
    Q_OBJECT
public:
    QWSSoundServer(QObject* parent=0);
    ~QWSSoundServer();
    void playFile(const QString& filename);

private:
    QWSSoundServerPrivate* d;
};

#ifndef QT_NO_QWS_SOUNDSERVER
class QWSSoundClient : public QTcpSocket {
    Q_OBJECT
public:
    QWSSoundClient(QObject* parent=0);
    void play(const QString& filename);
};

class QWSSoundServerClient : public QTcpSocket {
    Q_OBJECT

public:
    QWSSoundServerClient(int s, QObject* parent);
    ~QWSSoundServerClient();

signals:
    void play(const QString&);

private slots:
    void destruct();
    void tryReadCommand();
};

class QWSSoundServerSocket : public QTcpServer {
    Q_OBJECT

public:
    QWSSoundServerSocket(QObject* parent=0, const char* name=0);
    void incomingConnection(int s);

signals:
    void playFile(const QString& filename);
};
#endif

#endif // Q_OS_MAC
#endif // QT_NO_SOUND

#endif // QSOUNDQSS_QWS_H
