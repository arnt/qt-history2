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

class QGlobalNetworkProxy
{
public:
    QGlobalNetworkProxy()
#ifndef QT_NO_SOCKS5
        : socks5SocketEngineHandler(0)
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

    void setProxy(const QNetworkProxy &networkProxy)
    {
        QMutexLocker lock(&mutex);
        globalProxy = networkProxy;
    }

    QNetworkProxy proxy()
    {
        QMutexLocker lock(&mutex);
        return globalProxy;
    }

private:
    QMutex mutex;
    QNetworkProxy globalProxy;
#ifndef QT_NO_SOCKS5
    QSocks5SocketEngineHandler *socks5SocketEngineHandler;
#endif
};

Q_GLOBAL_STATIC(QGlobalNetworkProxy, globalNetworkProxy);

class QNetworkProxyPrivate
{
    Q_DECLARE_PUBLIC(QNetworkProxy)

public:
    QNetworkProxy::ProxyType type;
    QString userName;
    QString password;
    QHostAddress address;
    quint16 port;

    QNetworkProxy *q_ptr;
};

QNetworkProxy::QNetworkProxy()
 : d_ptr(new QNetworkProxyPrivate)
{
    Q_D(QNetworkProxy);
    d->q_ptr = this;
    d->type = AutoProxy;
    d->port = 0;
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
    Sets the user name for proxy authentication to be \a userName.

    \sa userName(), setPassword(), password()
*/
void QNetworkProxy::setUserName(const QString &userName)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->userName = userName;
}

/*!
    Returns the user name used for authentication.

    \sa setUserName(), setPassword(), password()
*/
QString QNetworkProxy::userName() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->userName;
}

/*!
    Sets the password for proxy authentication to be \a password.

    \sa userName(), setUserName(), password()
*/
void QNetworkProxy::setPassword(const QString &password)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->password = password;
}

/*!
    Returns the password used for authentication.

    \sa userName(), setPassword(), setUserName()
*/
QString QNetworkProxy::password() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->password;
}

/*!
    Sets the address of the proxy host to be \a address.

    \sa address(), setPort(), port()
*/
void QNetworkProxy::setAddress(const QHostAddress &address)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->address = address;
}

/*!
    Returns the address of the proxy host.

    \sa setAddress(), setPort(), port()
*/
QHostAddress QNetworkProxy::address() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->address;
}

/*!
    Sets the port of the proxy host to be \a port.

    \sa address(), setAddress(), port()
*/
void QNetworkProxy::setPort(quint16 port)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->port = port;
}

/*!
    Returns the port of the proxy host.

    \sa setAddress(), setPort(), address()
*/
quint16 QNetworkProxy::port() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->port;
}

/*!
    Sets the application level netwok proxying to be \networkProxy.

    \sa QAbstractSocket::setProxy(), QTcpServer::setProxy()
*/
void QNetworkProxy::setProxy(const QNetworkProxy &networkProxy)
{
    if (globalNetworkProxy())
        globalNetworkProxy()->setProxy(networkProxy);
}

/*!
    Returns the application level network proxying.

    \sa QAbstractSocket::proxy(), QTcpServer::proxy()
*/
QNetworkProxy QNetworkProxy::proxy()
{
    if (globalNetworkProxy())
        return globalNetworkProxy()->proxy();
    return QNetworkProxy();
}

#endif // QT_NO_NETWORKPROXY
