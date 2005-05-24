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

#include "qhostaddress.h"
#include "qstringlist.h"

#define QT_ENSURE_PARSED(a) \
    do { \
        if (!(a)->d->isParsed) \
            (a)->d->parse(); \
    } while (0)

class QHostAddressPrivate
{
public:
    QHostAddressPrivate();

    void setAddress(quint32 a_ = 0);
    void setAddress(const quint8 *a_);
    void setAddress(const Q_IPV6ADDR &a_);

    bool parse();
    void clear();

private:
    quint32 a;    // IPv4 address
    Q_IPV6ADDR a6; // IPv6 address
    QAbstractSocket::NetworkLayerProtocol protocol;

    QString ipString;
    bool isParsed;

    friend class QHostAddress;
};

QHostAddressPrivate::QHostAddressPrivate()
    : a(0), protocol(QAbstractSocket::UnknownNetworkLayerProtocol), isParsed(true)
{
    memset(&a6, 0, sizeof(a6));
}

void QHostAddressPrivate::setAddress(quint32 a_)
{
    a = a_;
    protocol = QAbstractSocket::IPv4Protocol;
    isParsed = true;
}

void QHostAddressPrivate::setAddress(const quint8 *a_)
{
    for (int i = 0; i < 16; i++)
        a6[i] = a_[i];
    protocol = QAbstractSocket::IPv6Protocol;
    isParsed = true;
}

void QHostAddressPrivate::setAddress(const Q_IPV6ADDR &a_)
{
    a6 = a_;
    a = 0;
    protocol = QAbstractSocket::IPv6Protocol;
    isParsed = true;
}

static bool parseIp4(const QString& address, quint32 *addr)
{
    QStringList ipv4 = address.split(".");
    if (ipv4.count() != 4)
        return false;

    quint32 ipv4Address = 0;
    for (int i = 0; i < 4; ++i) {
        bool ok = false;
        uint byteValue = ipv4.at(i).toUInt(&ok);
        if (!ok || byteValue > 255)
            return false;

        ipv4Address <<= 8;
        ipv4Address += byteValue;
    }

    *addr = ipv4Address;
    return true;
}

static bool parseIp6(const QString &address, quint8 *addr)
{
    QStringList ipv6 = address.split(":");
    int count = ipv6.count();
    if (count < 3 || count > 8)
        return false;

    int mc = 16;
    int fillCount = 9 - count;
    for (int i = count - 1; i >= 0; --i) {
        if (mc <= 0)
            return false;

        if (ipv6.at(i).isEmpty()) {
            if (i == count - 1) {
                // special case: ":" is last character
                if (!ipv6.at(i - 1).isEmpty())
                    return false;
                addr[--mc] = 0;
                addr[--mc] = 0;
            } else if (i == 0) {
                // special case: ":" is first character
                if (!ipv6.at(i + 1).isEmpty())
                    return false;
                addr[--mc] = 0;
                addr[--mc] = 0;
            } else {
                for (int j = 0; j < fillCount; ++j) {
                    if (mc <= 0)
                        return false;
                    addr[--mc] = 0;
                    addr[--mc] = 0;
                }
            }
        } else {
            bool ok = false;
            uint byteValue = ipv6.at(i).toUInt(&ok, 16);
            if (ok && byteValue <= 0xffff) {
                addr[--mc] = byteValue & 0xff;
                addr[--mc] = (byteValue >> 8) & 0xff;
            } else {
                if (i != count - 1)
                    return false;

                // parse the ipv4 part of a mixed type
                quint32 maybeIp4;
                if (!parseIp4(ipv6.at(i), &maybeIp4))
                    return false;

                addr[--mc] = maybeIp4 & 0xff;
                addr[--mc] = (maybeIp4 >> 8) & 0xff;
                addr[--mc] = (maybeIp4 >> 16) & 0xff;
                addr[--mc] = (maybeIp4 >> 24) & 0xff;
                --fillCount;
            }
        }
    }

    return true;
}

bool QHostAddressPrivate::parse()
{
    isParsed = true;
    protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
    QString a = ipString.simplified();

    // All IPv6 addresses contain a ':', and may contain a '.'.
    if (a.contains(':')) {
        quint8 maybeIp6[16];
        if (parseIp6(a, maybeIp6)) {
            setAddress(maybeIp6);
            protocol = QAbstractSocket::IPv6Protocol;
            return true;
        }
    }

    // All IPv4 addresses contain a '.'.
    if (a.contains('.')) {
        quint32 maybeIp4 = 0;
        if (parseIp4(a, &maybeIp4)) {
            setAddress(maybeIp4);
            protocol = QAbstractSocket::IPv4Protocol;
            return true;
        }
    }

    return false;
}

void QHostAddressPrivate::clear()
{
    a = 0;
    protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
    isParsed = true;
    memset(&a6, 0, sizeof(a6));
}

/*!
    \class QHostAddress
    \brief The QHostAddress class provides an IP address.
    \ingroup io
    \module network

    This class holds an IPv4 or IPv6 address in a platform- and
    protocol-independent manner.

    QHostAddress is normally used with the QTcpSocket, QTcpServer,
    and QUdpSocket to connect to a host or to set up a server.

    A host address is set with setAddress(), checked for its type
    using isIPv4Address() or isIPv6Address(), and retrieved with
    toIPv4Address(), toIPv6Address(), or toString().

    The class also supports common predefined addresses: \l Null, \l
    LocalHost, \l LocalHostIPv6, \l Broadcast, and \l Any.

    \sa QTcpSocket, QTcpServer, QUdpSocket
*/

/*! \enum QHostAddress::SpecialAddress

    \value Null The null address object. Equivalent to QHostAddress().
    \value LocalHost The IPv4 localhost address. Equivalent to QHostAddress("127.0.0.1").
    \value LocalHostIPv6 The IPv6 localhost address. Equivalent to QHostAddress("::1").
    \value Broadcast The IPv4 broadcast address. Equivalent to QHostAddress("255.255.255.255").
    \value Any The IPv4 any-address. Equivalent to QHostAddress("0.0.0.0").
    \value AnyIPv6 The IPv6 any-address. Equivalent to QHostAddress("::").
*/

/*!  Constructs a host address object with the IP address 0.0.0.0.

    \sa clear()
*/
QHostAddress::QHostAddress()
    : d(new QHostAddressPrivate)
{
}

/*!
    Constructs a host address object with the IPv4 address \a ip4Addr.
*/
QHostAddress::QHostAddress(quint32 ip4Addr)
    : d(new QHostAddressPrivate)
{
    setAddress(ip4Addr);
}

/*!
    Constructs a host address object with the IPv6 address \a ip6Addr.

    \a ip6Addr must be a 16-byte array in network byte order (big
    endian).
*/
QHostAddress::QHostAddress(quint8 *ip6Addr)
    : d(new QHostAddressPrivate)
{
    setAddress(ip6Addr);
}

/*!
    Constructs a host address object with the IPv6 address \a ip6Addr.
*/
QHostAddress::QHostAddress(const Q_IPV6ADDR &ip6Addr)
    : d(new QHostAddressPrivate)
{
    setAddress(ip6Addr);
}

/*!
    Constructs an IPv4 or IPv6 address based on the string \a address
    (e.g., "127.0.0.1").

    \sa setAddress()
*/
QHostAddress::QHostAddress(const QString &address)
    : d(new QHostAddressPrivate)
{
    d->ipString = address;
    d->isParsed = false;
}

/*!
    Constructs a copy of the given \a address.
*/
QHostAddress::QHostAddress(const QHostAddress &address)
    : d(new QHostAddressPrivate(*address.d))
{
}

/*!
    Constructs a QHostAddress object for \a address.
*/
QHostAddress::QHostAddress(SpecialAddress address)
    : d(new QHostAddressPrivate)
{
    switch (address) {
    case Null:
        break;
    case Broadcast:
        setAddress("255.255.255.255");
        break;
    case LocalHost:
        setAddress("127.0.0.1");
        break;
    case LocalHostIPv6:
        setAddress("::1");
        break;
    case Any:
        setAddress("0.0.0.0");
        break;
    case AnyIPv6:
        setAddress("::");
        break;
    }
}

/*!
    Destroys the host address object.
*/
QHostAddress::~QHostAddress()
{
    delete d;
}

/*!
    Assigns another host \a address to this object, and returns a reference
    to this object.
*/
QHostAddress &QHostAddress::operator=(const QHostAddress &address)
{
    *d = *address.d;
    return *this;
}

/*!
    Sets the host address to 0.0.0.0.
*/
void QHostAddress::clear()
{
    d->clear();
}

/*!
    Set the IPv4 address specified by \a ip4Addr.
*/
void QHostAddress::setAddress(quint32 ip4Addr)
{
    d->setAddress(ip4Addr);
}

/*!
    \overload

    Set the IPv6 address specified by \a ip6Addr.

    \a ip6Addr must be an array of 16 bytes in network byte order
    (high-order byte first).
*/
void QHostAddress::setAddress(quint8 *ip6Addr)
{
    d->setAddress(ip6Addr);
}

/*!
    \overload

    Set the IPv6 address specified by \a ip6Addr.
*/
void QHostAddress::setAddress(const Q_IPV6ADDR &ip6Addr)
{
    d->setAddress(ip6Addr);
}

/*!
    \overload

    Sets the IPv4 or IPv6 address specified by the string
    representation specified by \a address (e.g. "127.0.0.1").
    Returns true and sets the address if the address was successfully
    parsed; otherwise returns false.
*/
bool QHostAddress::setAddress(const QString &address)
{
    d->ipString = address;
    return d->parse();
}

/*!
    Returns the IPv4 address as a number.

    For example, if the address is 127.0.0.1, the returned value is
    2130706433 (i.e. 0x7f000001).

    This value is only valid if isIp4Addr() returns true.

    \sa toString()
*/
quint32 QHostAddress::toIPv4Address() const
{
    QT_ENSURE_PARSED(this);
    return d->a;
}

/*!
    Returns the network layer protocol of the host address.
*/
QAbstractSocket::NetworkLayerProtocol QHostAddress::protocol() const
{
    QT_ENSURE_PARSED(this);
    return d->protocol;
}

/*!
    Returns the IPv6 address as a Q_IPV6ADDR structure. The structure
    consists of 16 unsigned characters.

    \code
        Q_IPV6ADDR addr = hostAddr.ip6Addr();
        // addr contains 16 unsigned characters

        for (int i = 0; i < 16; ++i) {
            // process addr[i]
        }
    \endcode

    This value is only valid if isIPv6Address() returns true.

    \sa toString()
*/
Q_IPV6ADDR QHostAddress::toIPv6Address() const
{
    QT_ENSURE_PARSED(this);
    return d->a6;
}

#ifndef QT_NO_SPRINTF
/*!
    Returns the address as a string.

    For example, if the address is the IPv4 address 127.0.0.1, the
    returned string is "127.0.0.1".

    \sa toIPv4Address()
*/
QString QHostAddress::toString() const
{
    QT_ENSURE_PARSED(this);
    if (d->protocol == QAbstractSocket::IPv4Protocol) {
        quint32 i = toIPv4Address();
        QString s;
        s.sprintf("%d.%d.%d.%d", (i>>24) & 0xff, (i>>16) & 0xff,
                (i >> 8) & 0xff, i & 0xff);
        return s;
    }

    if (d->protocol == QAbstractSocket::IPv6Protocol) {
        quint16 ugle[8];
        for (int i = 0; i < 8; i++) {
            ugle[i] = (quint16(d->a6[2*i]) << 8) | quint16(d->a6[2*i+1]);
        }
        QString s;
        s.sprintf("%X:%X:%X:%X:%X:%X:%X:%X",
                  ugle[0], ugle[1], ugle[2], ugle[3], ugle[4], ugle[5], ugle[6], ugle[7]);
        return s;
    }

    return QString();
}
#endif

/*!
    Returns true if this host address is the same as the \a other address
    given; otherwise returns false.
*/
bool QHostAddress::operator==(const QHostAddress &other) const
{
    QT_ENSURE_PARSED(this);
    QT_ENSURE_PARSED(&other);

    if (d->protocol == QAbstractSocket::IPv4Protocol)
        return other.d->protocol == QAbstractSocket::IPv4Protocol && d->a == other.d->a;
    if (d->protocol == QAbstractSocket::IPv6Protocol) {
        return other.d->protocol == QAbstractSocket::IPv6Protocol
               && memcmp(&d->a6, &other.d->a6, sizeof(Q_IPV6ADDR)) == 0;
    }
    return true;
}

/*!
    Returns true if this host address is the same as the \a other
    address given; otherwise returns false.
*/
bool QHostAddress::operator ==(SpecialAddress other) const
{
    QT_ENSURE_PARSED(this);
    QHostAddress otherAddress(other);
    QT_ENSURE_PARSED(&otherAddress);

    if (d->protocol == QAbstractSocket::IPv4Protocol)
        return otherAddress.d->protocol == QAbstractSocket::IPv4Protocol && d->a == otherAddress.d->a;
    if (d->protocol == QAbstractSocket::IPv6Protocol) {
        return otherAddress.d->protocol == QAbstractSocket::IPv6Protocol
               && memcmp(&d->a6, &otherAddress.d->a6, sizeof(Q_IPV6ADDR)) == 0;
    }
    return true;
}

/*!
    Returns true if this host address is null (INADDR_ANY or in6addr_any).
    The default constructor creates a null address, and that address is
    not valid for any host or interface.
*/
bool QHostAddress::isNull() const
{
    QT_ENSURE_PARSED(this);
    return d->protocol == QAbstractSocket::UnknownNetworkLayerProtocol;
}

/*!
    \fn quint32 QHostAddress::ip4Addr() const

    Use toIPv4Address() instead.
*/

/*!
    \fn bool QHostAddress::isIp4Addr() const

    Use protocol() instead.
*/

/*!
    \fn bool QHostAddress::isIPv4Address() const

    Use protocol() instead.
*/

/*!
    \fn bool QHostAddress::isIPv6Address() const

    Use protocol() instead.
*/
