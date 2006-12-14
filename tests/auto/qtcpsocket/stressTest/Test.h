#ifndef TEST_H
#define TEST_H

//------------------------------------------------------------------------------
// Qt
#include <Q3ServerSocket>
#include <Q3Socket>
#include <QTcpServer>
#include <QTcpSocket>

//------------------------------------------------------------------------------
class My3Socket : public Q3Socket
{
    Q_OBJECT
public:
    My3Socket(QObject *parent);

    void sendTest(Q_UINT32 num);
    bool safeShutDown;

private slots:
    void read();
    void closed();
};

//------------------------------------------------------------------------------
class My3Server : public Q3ServerSocket
{
    Q_OBJECT
public:
    My3Server(QObject *parent = 0);

    void newConnection(int socket);

private slots:
    void stopServer();

private:
    My3Socket *m_socket;
};

//------------------------------------------------------------------------------
class My4Socket : public QTcpSocket
{
    Q_OBJECT
public:
    My4Socket(QObject *parent);

    void sendTest(Q_UINT32 num);
    bool safeShutDown;

private slots:
    void read();
    void closed();
};

//------------------------------------------------------------------------------
class My4Server : public QTcpServer
{
    Q_OBJECT
public:
    My4Server(QObject *parent = 0);

protected:
    void incomingConnection(int socket);

private slots:
    void stopServer();

private:
    My4Socket *m_socket;
};

//------------------------------------------------------------------------------
class Test : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Qt4Client,
        Qt4Server,
        Qt3Client,
        Qt3Server
    };
    Test(Type type);
};

//------------------------------------------------------------------------------
#endif	// TEST_H
