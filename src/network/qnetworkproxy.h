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

#ifndef QNETWORKPROXY_H
#define QNETWORKPROXY_H

#include <QtNetwork/qhostaddress.h>

QT_MODULE(Network)

class QNetworkProxyPrivate;

class Q_NETWORK_EXPORT QNetworkProxy
{
    Q_DECLARE_PRIVATE(QNetworkProxy)

public:
    enum ProxyType {
        AutoProxy,
        Socks5Proxy,
        NoProxy
    };

    QNetworkProxy();

    void setType(QNetworkProxy::ProxyType type);
    QNetworkProxy::ProxyType type() const;

    void setUserName(const QString &userName);
    QString userName() const;

    void setPassword(const QString &password);
    QString password() const;

    void setAddress(const QHostAddress & address);
    QHostAddress address() const;
    
    void setPort(quint16 port);
    quint16 port() const;

    static void setProxy(const QNetworkProxy &networkProxy);
    static QNetworkProxy proxy();
   
private:
    QNetworkProxyPrivate *d_ptr;
};

#endif // QHOSTINFO_H
