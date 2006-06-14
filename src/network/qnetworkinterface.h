/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QNETWORKINTERFACE_H
#define QNETWORKINTERFACE_H

#include <QtNetwork/qhostaddress.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

template<typename T> class QList;

class QNetworkAddressEntryPrivate;
class Q_NETWORK_EXPORT QNetworkAddressEntry
{
public:
    QNetworkAddressEntry();
    QNetworkAddressEntry(const QNetworkAddressEntry &other);
    QNetworkAddressEntry &operator=(const QNetworkAddressEntry &other);
    ~QNetworkAddressEntry();
    
    QHostAddress ip() const;
    void setIp(const QHostAddress &newIp);
    
    QHostAddress netmask() const;
    void setNetmask(const QHostAddress &newNetmask);
    
    QHostAddress broadcast() const;
    void setBroadcast(const QHostAddress &newBroadcast);

private:
    QNetworkAddressEntryPrivate *d;
};

class QNetworkInterfacePrivate;
class Q_NETWORK_EXPORT QNetworkInterface
{
public:
    enum InterfaceFlag {
        IsUp = 0x1,
        IsRunning = 0x2,
        CanBroadcast = 0x4,
        IsLoopBack = 0x8,
        IsPointToPoint = 0x10,
        CanMulticast = 0x20
    };
    Q_DECLARE_FLAGS(InterfaceFlags, InterfaceFlag)
    
    QNetworkInterface();
    QNetworkInterface(const QNetworkInterface &other);
    QNetworkInterface &operator=(const QNetworkInterface &other);
    ~QNetworkInterface();

    bool isValid() const;

    QString name() const;
    InterfaceFlags flags() const;
    QString hardwareAddress() const;
    QList<QNetworkAddressEntry> addressEntries() const;

    static QNetworkInterface interfaceFromName(const QString &name);
    static QNetworkInterface interfaceFromIndex(int index);
    static QList<QNetworkInterface> allInterfaces();
    static QList<QHostAddress> allAddresses();

private:
    QNetworkInterface(QNetworkInterfacePrivate *);

    friend class QNetworkInterfacePrivate;
    QNetworkInterfacePrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QNetworkInterface::InterfaceFlags)

#ifndef QT_NO_DEBUG
#include <QtCore/QDebug>
QDebug operator<<(QDebug debug, const QNetworkInterface &interface);
#endif

QT_END_HEADER

#endif
