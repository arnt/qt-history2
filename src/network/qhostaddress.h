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

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#include "qstring.h"
#include "qabstractsocket.h"

class QHostAddressPrivate;

class Q_NETWORK_EXPORT QIPv6Address
{
public:
    inline Q_UINT8 &operator [](int index) { return c[index]; }
    inline Q_UINT8 operator [](int index) const { return c[index]; }
    Q_UINT8 c[16];
};

typedef QIPv6Address Q_IPV6ADDR;

class Q_NETWORK_EXPORT QHostAddress
{
public:
    enum SpecialAddress {
        Null,
        Broadcast,
        LocalHost,
        LocalHostIPv6,
        Any,
        AnyIPv6
    };

    QHostAddress();
    QHostAddress(Q_UINT32 ip4Addr);
    QHostAddress(Q_UINT8 *ip6Addr);
    QHostAddress(const Q_IPV6ADDR &ip6Addr);
    explicit QHostAddress(const QString &address);
    QHostAddress(const QHostAddress &copy);
    QHostAddress(SpecialAddress address);
    ~QHostAddress();

    QHostAddress &operator=(const QHostAddress &);

    void setAddress(Q_UINT32 ip4Addr);
    void setAddress(Q_UINT8 *ip6Addr);
    void setAddress(const Q_IPV6ADDR &ip6Addr);
    bool setAddress(const QString &address);

    Qt::NetworkLayerProtocol protocol() const;
    Q_UINT32 toIPv4Address() const;
    Q_IPV6ADDR toIPv6Address() const;

    QString toString() const;

    bool operator ==(const QHostAddress &address) const;
    bool operator ==(SpecialAddress address) const;
    bool isNull() const;
    void clear();

#ifdef QT_COMPAT
    inline QT_COMPAT Q_UINT32 ip4Addr() const { return toIPv4Address(); }
    inline QT_COMPAT bool isIPv4Address() const { return protocol() == Qt::IPv4Protocol
                                                      || protocol() == Qt::UnknownNetworkLayerProtocol; }
    inline QT_COMPAT bool isIp4Addr() const  { return protocol() == Qt::IPv4Protocol
                                                      || protocol() == Qt::UnknownNetworkLayerProtocol; }
    inline QT_COMPAT bool isIPv6Address() const { return protocol() == Qt::IPv6Protocol; }
#endif

private:
    QHostAddressPrivate *d;
};

#endif
