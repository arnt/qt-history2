#ifndef QSS_H
#define QSS_H

#include <qsocket.h>

class QWSSoundServerData;

class QWSSoundServer : public QObject {
    Q_OBJECT
public:
    QWSSoundServer(QObject* parent=0);
    ~QWSSoundServer();
private:
    QWSSoundServerData* d;
};

class QWSSoundClient : public QSocket {
    Q_OBJECT
public:
    QWSSoundClient( QObject* parent=0 );
    void play( const QString& filename );
};

#endif
