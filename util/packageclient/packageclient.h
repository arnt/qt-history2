#ifndef PACKAGECLIENT_H
#define PACKAGECLIENT_H

#include <qsocket.h>
#include <qcstring.h>

class PackageClient : public QSocket
{
    Q_OBJECT

public:
    PackageClient( const QString &source, const QString &dest, const QString &host );

private slots:
    void startCopy();
    void readResult();
    void arrrrrrg();
    void doConnect();

private:
    QString filename;
    QString destination;
    QString hostname;
    int size, pos;
    QByteArray buffer;

};

#endif
