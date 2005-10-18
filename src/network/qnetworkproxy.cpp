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


/*! \class QNetworkProxy

    \brief The QNetworkProxy class provides a network layer proxying.

    \reentrant
    \ingroup io
    \module network

    \sa QAbstractSocket, QTcpSocket
*/

/*!
    \enum QNetworkProxy::ProxyType

    This enum describes the types of network proxying provided in Qt.

    \value AutoProxy Proxying is determind based on application settings
    \value Socks5Proxy Socks 5 proxying is used
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
    QBasicAtomic ref;
    QNetworkProxy::ProxyType type;
    QString user;
    QString password;
    QString hostName;
    quint16 port;
};

QNetworkProxy::QNetworkProxy()
 : d_ptr(new QNetworkProxyPrivate)
{
    Q_D(QNetworkProxy);
    d->ref.init(1);
    d->type = DefaultProxy;
    d->port = 0;
}

QNetworkProxy::QNetworkProxy(ProxyType type, const QString &hostName, quint16 port,
                  const QString &user, const QString &password)
 : d_ptr(new QNetworkProxyPrivate)
{
    Q_D(QNetworkProxy);
    d->ref.init(1);
    setType(type);
    setHostName(hostName);
    setPort(port);
    setUser(user);
    setPassword(password);
}

QNetworkProxy::QNetworkProxy(const QNetworkProxy &other)
    : d_ptr(other.d_ptr)
{

    d_ptr->ref.ref();
}

QNetworkProxy::~QNetworkProxy()
{
    Q_D(QNetworkProxy);
    if (!d->ref.deref())
        delete d_ptr;
}

QNetworkProxy &QNetworkProxy::operator=(const QNetworkProxy &other)
{
    qAtomicAssign(d_ptr, other.d_ptr);
    return *this;
}

/*!
    Sets the method of proxying fro this instance to be \a type.

    \sa type()
*/
void QNetworkProxy::setType(QNetworkProxy::ProxyType type)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->type = type;
}

/*!
    Returns the method of proxying used for this instance.

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

    \sa setuser(), setPassword(), password()
*/
QString QNetworkProxy::user() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->user;
}

/*!
    Sets the password for proxy authentication to be \a password.

    \sa user(), setuser(), password()
*/
void QNetworkProxy::setPassword(const QString &password)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->password = password;
}

/*!
    Returns the password used for authentication.

    \sa user(), setPassword(), setuser()
*/
QString QNetworkProxy::password() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->password;
}

/*!
    Sets the host name of the proxy host to be \a hostname.

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

    \sa setHostMame(), setPort(), hostName()
*/
quint16 QNetworkProxy::port() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->port;
}

/*!
    Sets the application level netwok proxying to be \networkProxy.

    \sa applicationProxy(), QAbstractSocket::setProxy(), QTcpServer::setProxy()
*/
void QNetworkProxy::setApplicationProxy(const QNetworkProxy &networkProxy)
{
    if (globalNetworkProxy())
        globalNetworkProxy()->setApplicationProxy(networkProxy);
}

/*!
    Returns the application level network proxying.

    \sa setApplicationProxy(), QAbstractSocket::proxy(), QTcpServer::proxy()
*/
QNetworkProxy QNetworkProxy::applicationProxy()
{
    if (globalNetworkProxy())
        return globalNetworkProxy()->applicationProxy();
    return QNetworkProxy();
}

#endif // QT_NO_NETWORKPROXY
