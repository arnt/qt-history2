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


/*! 
    \class QNetworkProxy

    \since 4.1

    \brief The QNetworkProxy class provides a network layer proxy.

    \reentrant
    \ingroup io
    \module network

    QNetworkProxy provides the method for configuring network layer
    proxy support to the Qt network classes. The currently supported
    classes are QAbstractSocket, QTcpSocket, QUdpSocket, QTcpServer,
    QHttp and QFtp. The proxy support is designed to be as transparent
    as possible. This means that existing network-enabled applications
    that you have written should automatically support network proxy
    using the following code.

    \code
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::Socks5Proxy);
        proxy.setHostName("proxy.example.com");
        proxy.setPort(1080);
        proxy.setUser("username");
        proxy.setPassword("password");
        QNetworkProxy::setApplicationProxy(proxy);
    \endcode

    An alternative to setting an application wide proxy is to specify
    the proxy for individual sockets using QAbstractSocket::setProxy()
    and QTcpServer::setProxy(). In this way, it is possible to disable
    the use of a proxy for specific sockets using the following code:

    \code
        serverSocket->setProxy(QNetworkProxy::NoProxy);
    \endcode

    Network proxy is not used if the address used in \l
    {QAbstractSocket::connectToHost()}{connectToHost()}, \l
    {QUdpSocket::bind()}{bind()} or \l
    {QTcpServer::listen()}{listen()} is equivalent to
    QHostAddress::LocalHost or QHostAddress::LocalHostIPv6.

    Each type of proxy support has certain restrictions associated with it.
    You should read the \l{ProxyType} documentation carefully before
    selecting a proxy type to use.

    \section1 SOCKS5

    The SOCKS5 support in Qt 4 is based on \l{RFC 1928} and \l{RFC 1929}.
    The supported authentication methods are no authentication and
    username/password authentication.  Both IPv4 and IPv6 are
    supported, but domain name resolution via the SOCKS server is not
    supported; i.e. all domain names are resolved locally. There are
    several things to remember when using SOCKS5 with QUdpSocket and
    QTcpServer:

    With QUdpSocket, a call to \l {QUdpSocket::bind()}{bind()} may fail
    with a timeout error. If a port number other than 0 is passed to
    \l {QUdpSocket::bind()}{bind()}, it is not guaranteed that it is the
    specified port that will be used.
    Use \l{QUdpSocket::localPort()}{localPort()} and
    \l{QUdpSocket::localAddress()}{localAddress()} to get the actual
    address and port number in use. Because proxied UDP goes through
    two UDP connections, it is more likely that packets will be dropped.

    With QTcpServer a call to \l{QTcpServer::listen()}{listen()} may
    fail with a timeout error. If a port number other than 0 is passed
    to \l{QTcpServer::listen()}{listen()}, then it is not guaranteed
    that it is the specified port that will be used.
    Use \l{QTcpServer::serverPort()}{serverPort()} and
    \l{QTcpServer::serverAddress()}{serverAddress()} to get the actual
    address and port used to listen for connections. SOCKS5 only supports
    one accepted connection per call to \l{QTcpServer::listen()}{listen()},
    and each call is likely to result in a different
    \l{QTcpServer::serverPort()}{serverPort()} being used.

    \sa QAbstractSocket, QTcpServer
*/

/*!
    \enum QNetworkProxy::ProxyType

    This enum describes the types of network proxying provided in Qt.

    \value DefaultProxy Proxy is determined based on the application proxy set using setApplicationProxy()
    \value Socks5Proxy \l Socks5 proxying is used
    \value NoProxy No proxying is used

    \sa setType(), type()
*/

#include "qnetworkproxy.h"

#ifndef QT_NO_NETWORKPROXY

#include "qsocks5socketengine_p.h"
#include "qmutex.h"
#include "qatomic.h"

class QGlobalNetworkProxy
{
public:
    QGlobalNetworkProxy()
        : mutex(QMutex::Recursive)
#ifndef QT_NO_SOCKS5
        , socks5SocketEngineHandler(0)
#endif
    {
    }

    ~QGlobalNetworkProxy()
    {
#ifndef QT_NO_SOCKS5
        delete socks5SocketEngineHandler;
#endif
    }

    void init()
    {
#ifndef QT_NO_SOCKS5
        QMutexLocker lock(&mutex);
        if (!socks5SocketEngineHandler)
            socks5SocketEngineHandler = new QSocks5SocketEngineHandler();
#endif
    }

    void setApplicationProxy(const QNetworkProxy &proxy)
    {
        QMutexLocker lock(&mutex);
        applicationLevelProxy = proxy;
    }

    QNetworkProxy applicationProxy()
    {
        QMutexLocker lock(&mutex);
        return applicationLevelProxy;
    }

private:
    QMutex mutex;
    QNetworkProxy applicationLevelProxy;
#ifndef QT_NO_SOCKS5
    QSocks5SocketEngineHandler *socks5SocketEngineHandler;
#endif
};

Q_GLOBAL_STATIC(QGlobalNetworkProxy, globalNetworkProxy);

class QNetworkProxyPrivate
{
public:
    QNetworkProxy::ProxyType type;
    QString user;
    QString password;
    QString hostName;
    quint16 port;
};

/*!
    Constructs a QNetworkProxy with DefaultProxy type; the proxy type is
    determined by applicationProxy(), which defaults to NoProxy.

    \sa setType(), setApplicationProxy()
*/
QNetworkProxy::QNetworkProxy()
 : d_ptr(new QNetworkProxyPrivate)
{
    Q_D(QNetworkProxy);
    d->type = DefaultProxy;
    d->port = 0;
}

/*!
    Constructs a QNetworkProxy with \a type, \a hostName, \a port,
    \a user and \a password.
*/
QNetworkProxy::QNetworkProxy(ProxyType type, const QString &hostName, quint16 port,
                  const QString &user, const QString &password)
 : d_ptr(new QNetworkProxyPrivate)
{
    setType(type);
    setHostName(hostName);
    setPort(port);
    setUser(user);
    setPassword(password);
}

/*!
    Constructs a copy of \a other.
*/
QNetworkProxy::QNetworkProxy(const QNetworkProxy &other)
    : d_ptr(new QNetworkProxyPrivate)
{
    *d_ptr = *other.d_ptr;
}

/*!
    Destroys the QNetworkProxy object.
*/
QNetworkProxy::~QNetworkProxy()
{
    delete d_ptr;
}

/*!
    \since 4.2

    Assigns the value of the network proxy \a other to this network proxy.
*/
QNetworkProxy &QNetworkProxy::operator=(const QNetworkProxy &other)
{
    *d_ptr = *other.d_ptr;
    return *this;
}

/*!
    Sets the proxy type for this instance to be \a type.

    \sa type()
*/
void QNetworkProxy::setType(QNetworkProxy::ProxyType type)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->type = type;
}

/*!
    Returns the proxy type for this instance.

    \sa setType()
*/
QNetworkProxy::ProxyType QNetworkProxy::type() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->type;
}

/*!
    Sets the user name for proxy authentication to be \a user.

    \sa user(), setPassword(), password()
*/
void QNetworkProxy::setUser(const QString &user)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->user = user;
}

/*!
    Returns the user name used for authentication.

    \sa setUser(), setPassword(), password()
*/
QString QNetworkProxy::user() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->user;
}

/*!
    Sets the password for proxy authentication to be \a password.

    \sa user(), setUser(), password()
*/
void QNetworkProxy::setPassword(const QString &password)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->password = password;
}

/*!
    Returns the password used for authentication.

    \sa user(), setPassword(), setUser()
*/
QString QNetworkProxy::password() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->password;
}

/*!
    Sets the host name of the proxy host to be \a hostName.

    \sa hostName(), setPort(), port()
*/
void QNetworkProxy::setHostName(const QString &hostName)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->hostName = hostName;
}

/*!
    Returns the host name of the proxy host.

    \sa setHostName(), setPort(), port()
*/
QString QNetworkProxy::hostName() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->hostName;
}

/*!
    Sets the port of the proxy host to be \a port.

    \sa hostName(), setHostName(), port()
*/
void QNetworkProxy::setPort(quint16 port)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->port = port;
}

/*!
    Returns the port of the proxy host.

    \sa setHostName(), setPort(), hostName()
*/
quint16 QNetworkProxy::port() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->port;
}

/*!
    Sets the application level network proxying to be \a networkProxy.

    If a QAbstractSocket or QTcpSocket has the
    QNetworkProxy::DefaultProxy type, then the QNetworkProxy set with
    this function is used.

    \sa applicationProxy(), QAbstractSocket::setProxy(), QTcpServer::setProxy()
*/
void QNetworkProxy::setApplicationProxy(const QNetworkProxy &networkProxy)
{
    if (globalNetworkProxy())
        globalNetworkProxy()->setApplicationProxy(networkProxy);
}

/*!
    Returns the application level network proxying.

    If a QAbstractSocket or QTcpSocket has the
    QNetworkProxy::DefaultProxy type, then the QNetworkProxy returned
    by this function is used.

    \sa setApplicationProxy(), QAbstractSocket::proxy(), QTcpServer::proxy()
*/
QNetworkProxy QNetworkProxy::applicationProxy()
{
    if (globalNetworkProxy())
        return globalNetworkProxy()->applicationProxy();
    return QNetworkProxy();
}

#endif // QT_NO_NETWORKPROXY
