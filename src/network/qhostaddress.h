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

#include "QtCore/qstring.h"
#include "QtNetwork/qabstractsocket.h"

class QHostAddressPrivate;

class Q_NETWORK_EXPORT QIPv6Address
{
public:
    inline quint8 &operator [](int index) { return c[index]; }
    inline quint8 operator [](int index) const { return c[index]; }
    quint8 c[16];
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
    explicit QHostAddress(quint32 ip4Addr);
    explicit QHostAddress(quint8 *ip6Addr);
    explicit QHostAddress(const Q_IPV6ADDR &ip6Addr);
    explicit QHostAddress(const QString &address);
    QHostAddress(const QHostAddress &copy);
    QHostAddress(SpecialAddress address);
    ~QHostAddress();

    QHostAddress &operator=(const QHostAddress &);

    void setAddress(quint32 ip4Addr);
    void setAddress(quint8 *ip6Addr);
    void setAddress(const Q_IPV6ADDR &ip6Addr);
    bool setAddress(const QString &address);

    QAbstractSocket::NetworkLayerProtocol protocol() const;
    quint32 toIPv4Address() const;
    Q_IPV6ADDR toIPv6Address() const;

    QString toString() const;

    bool operator ==(const QHostAddress &address) const;
    bool operator ==(SpecialAddress address) const;
    bool isNull() const;
    void clear();

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT quint32 ip4Addr() const { return toIPv4Address(); }
    inline QT3_SUPPORT bool isIPv4Address() const { return protocol() == QAbstractSocket::IPv4Protocol
                                                      || protocol() == QAbstractSocket::UnknownNetworkLayerProtocol; }
    inline QT3_SUPPORT bool isIp4Addr() const  { return protocol() == QAbstractSocket::IPv4Protocol
                                                      || protocol() == QAbstractSocket::UnknownNetworkLayerProtocol; }
    inline QT3_SUPPORT bool isIPv6Address() const { return protocol() == QAbstractSocket::IPv6Protocol; }
#endif

private:
    QHostAddressPrivate *d;
};

#endif // QHOSTADDRESS_H
