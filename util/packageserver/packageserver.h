#ifndef PACKAGESERVER_H
#define PACKAGESERVER_H

#include <qsocket.h>
#include <qserversocket.h>
#include <qcstring.h>

class PackageSocket : public QSocket
{
    Q_OBJECT

public:
    PackageSocket( int socket, QObject *parent = 0 );

private slots:
    void readClient();
    void connectionClosed();

private:
    int size, pos;
    QByteArray buffer;

};

class PackageServer : public QServerSocket
{
    Q_OBJECT

public:
    PackageServer( QObject *parent = 0 );
    void newConnection( int socket );

};

#endif
