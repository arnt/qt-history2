#ifndef MAIN_H
#define MAIN_H

#include <qobject.h>
#include <qhttp.h>

class TestClient : public QObject
{
    Q_OBJECT
public:
    TestClient();

    void get( const QString& host, int port, const QString& path );

private slots:
    void reply( const QHttpResponseHeader& repl, const QByteArray& data );

private:
    QHttpClient* m_client;
};

#endif
