/****************************************************************************
**
** Definition of Qt Sound System.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSOUNDQSS_QWS_H
#define QSOUNDQSS_QWS_H

#ifndef QT_H
#include "qserversocket.h"
#include "qsocket.h"
#endif // QT_H

#ifndef QT_NO_SOUND

#ifndef Q_OS_MAC

class QWSSoundServerPrivate;

class QWSSoundServer : public QObject {
    Q_OBJECT
public:
    QWSSoundServer(QObject* parent=0);
    ~QWSSoundServer();
    void playFile( const QString& filename );

private:
    QWSSoundServerPrivate* d;
};

#ifndef QT_NO_QWS_SOUNDSERVER
class QWSSoundClient : public QSocket {
    Q_OBJECT
public:
    QWSSoundClient( QObject* parent=0 );
    void play( const QString& filename );
};

class QWSSoundServerClient : public QSocket {
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

class QWSSoundServerSocket : public QServerSocket {
    Q_OBJECT

public:
    QWSSoundServerSocket(QObject* parent=0, const char* name=0);
    void newConnection(int s);

signals:
    void playFile(const QString& filename);
};
#endif

#endif // Q_OS_MAC
#endif // QT_NO_SOUND

#endif // QSOUNDQSS_QWS_H
