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
    QHostAddressPrivate(const QHostAddressPrivate &other) { *this = other; }
    QHostAddressPrivate &operator=(const QHostAddressPrivate &other)
    {
        a = other.a;
        a6 = other.a6;
        isIp4 = other.isIp4;
        ipString = other.ipString;
        isParsed = other.isParsed;
        return *this;
    }

    void setAddress(Q_UINT32 a_ = 0);
    void setAddress(const Q_UINT8 *a_);
    void setAddress(const Q_IPV6ADDR &a_);

    bool parse();
    void clear();

private:
    Q_UINT32 a;    // IPv4 address
    Q_IPV6ADDR a6; // IPv6 address
    bool isIp4;

    QString ipString;
    bool isParsed;

    friend class QHostAddress;
};

QHostAddressPrivate::QHostAddressPrivate()
    : a(0), isIp4(true), isParsed(true)
{
    memset(&a6, 0, sizeof(a6));
}

void QHostAddressPrivate::setAddress(Q_UINT32 a_)
{
    a = a_;
    isIp4 = true;
    isParsed = true;
}

void QHostAddressPrivate::setAddress(const Q_UINT8 *a_)
{
    for (int i = 0; i < 16; i++)
        a6.c[i] = a_[i];
    isIp4 = false;
    isParsed = true;
}

void QHostAddressPrivate::setAddress(const Q_IPV6ADDR &a_)
{
    a6 = a_;
    a = 0;
    isIp4 = false;
    isParsed = true;
}

static bool parseIp4(const QString& address, Q_UINT32 *addr)
{
    QStringList ipv4 = address.split(".");
    if (ipv4.count() != 4)
        return false;

    Q_UINT32 ipv4Address = 0;
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

static bool parseIp6(const QString &address, Q_UINT8 *addr)
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
                Q_UINT32 maybeIp4;
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
    QString a = ipString.simplified();

    // All IPv6 addresses contain a ':', and may contain a '.'.
    if (a.contains(':')) {
        Q_UINT8 maybeIp6[16];
        if (parseIp6(a, maybeIp6)) {
            setAddress(maybeIp6);
            return true;
        }
    }

    // All IPv4 addresses contain a '.'.
    if (a.contains('.')) {
        Q_UINT32 maybeIp4 = 0;
        if (parseIp4(a, &maybeIp4)) {
            setAddress(maybeIp4);
            return true;
        }
    }

    return false;
}

void QHostAddressPrivate::clear()
{
    a = 0;
    isIp4 = true;
    isParsed = true;
    memset(&a6, 0, sizeof(a6));
}

/*!
    \class QHostAddress qhostaddress.h
    \brief The QHostAddress class provides an IP address.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class holds an IP address in a platform and protocol
    independent manner. It stores both IPv4 and IPv6 addresses in a
    way that you can easily access on any platform.

    QHostAddress is normally used with the classes QSocket,
    QServerSocket, and QSocketDevice to set up a server, or to connect
    to a host.

    A host address is set with setAddress(), checked for its type
    using isIPv4Address() or isIPv6Address(), and retrieved with
    toIPv4Address(), toIPv6Address(), or toString().

    \sa QSocket, QServerSocket, QSocketDevice
*/


/*!
    Creates a host address object with the IP address 0.0.0.0.

    \sa clear()
*/
QHostAddress::QHostAddress()
    : d(new QHostAddressPrivate)
{
}

/*!
    \internal

    DOC: We can only make this public if we specify precisely the
    format of the address string.
*/
QHostAddress::QHostAddress(const QString &address)
    : d(new QHostAddressPrivate)
{
    d->ipString = address;
    d->isParsed = false;
}

/*!
    Creates a copy of the given \a address.
*/
QHostAddress::QHostAddress(const QHostAddress &address)
    : d(new QHostAddressPrivate(*address.d))
{
}

/*!
    Constructs the special host address.

    \sa SpecialAddress
*/
QHostAddress::QHostAddress(QHostAddress::SpecialAddress addressType)
    : d(new QHostAddressPrivate)
{
    switch (addressType) {
    case NullAddress:
        break;
    case LocalhostAddress:
        setAddress("127.0.0.1");
        break;
    case LocalhostIPv6Address:
        setAddress("::1");
        break;
    case AnyAddress:
        setAddress("0.0.0.0");
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
QHostAddress &QHostAddress::operator =(const QHostAddress &address)
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
void QHostAddress::setAddress(Q_UINT32 ip4Addr)
{
    d->setAddress(ip4Addr);
}

/*!
    \overload

    Set the IPv6 address specified by \a ip6Addr.

    \a ip6Addr must be an array of 16 bytes in network byte order
    (high-order byte first).
*/
void QHostAddress::setAddress(Q_UINT8 *ip6Addr)
{
    d->setAddress(ip6Addr);
}

/*!
    \overload

    Set the IPv6 address specified by \a ip6Addr.

    ### Add more docs later
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
    Returns true if this host address represents an IPv4 address;
    otherwise returns false.
*/
bool QHostAddress::isIPv4Address() const
{
    QT_ENSURE_PARSED(this);
    return d->isIp4;
}

/*!
    Returns the IPv4 address as a number.

    For example, if the address is 127.0.0.1, the returned value is
    2130706433 (i.e. 0x7f000001).

    This value is only valid if isIp4Addr() returns true.

    \sa toString()
*/
Q_UINT32 QHostAddress::toIPv4Address() const
{
    QT_ENSURE_PARSED(this);
    return d->a;
}

/*!
    Returns true if this host address represents an IPv6 address;
    otherwise returns false.
*/
bool QHostAddress::isIPv6Address() const
{
    QT_ENSURE_PARSED(this);
    return !d->isIp4;
}

/*!
    Returns the IPv6 address as a Q_IPV6ADDR structure. The structure
    consists of 16 unsigned characters.

    \code
        Q_IPV6ADDR addr = hostAddr.ip6Addr();
        // addr.c[] contains 16 unsigned characters

        for (int i = 0; i < 16; ++i) {
            // process addr.c[i]
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
    if (d->isIp4) {
        Q_UINT32 i = toIPv4Address();
        QString s;
        s.sprintf("%d.%d.%d.%d", (i>>24) & 0xff, (i>>16) & 0xff,
                (i >> 8) & 0xff, i & 0xff);
        return s;
    } else {
        Q_UINT16 ugle[8];
        for (int i=0; i<8; i++) {
            ugle[i] = ((Q_UINT16)(d->a6.c[2*i]) << 8) |
                ((Q_UINT16)(d->a6.c[2*i+1]));
        }
        QString s;
        s.sprintf("%X:%X:%X:%X:%X:%X:%X:%X",
                ugle[0], ugle[1], ugle[2], ugle[3],
                ugle[4], ugle[5], ugle[6], ugle[7]);
        return s;
    }
}
#endif


/*!
    Returns true if this host address is the same as the \a other address
    given; otherwise returns false.
*/
bool QHostAddress::operator ==(const QHostAddress &other) const
{
    QT_ENSURE_PARSED(this);
    QT_ENSURE_PARSED(&other);

    if (d->isIp4)
        return other.d->isIp4 && d->a == other.d->a;

    return memcmp(&d->a6, &other.d->a6, sizeof(Q_IPV6ADDR)) == 0;
}
/*
bool QHostAddress::operator !=(const QHostAddress &other) const
{
    return !(*this == other);
}
*/

bool QHostAddress::operator ==(SpecialAddress other) const
{
    QT_ENSURE_PARSED(this);
    QHostAddress otherAddress(other);
    QT_ENSURE_PARSED(&otherAddress);

    if (d->isIp4)
        return otherAddress.d->isIp4 && d->a == otherAddress.d->a;

    return memcmp(&d->a6, &otherAddress.d->a6, sizeof(Q_IPV6ADDR)) == 0;
}
/*
bool QHostAddress::operator !=(SpecialAddress other) const
{
    return !(*this == other);
}
*/

/*!
    Returns true if this host address is null (INADDR_ANY or in6addr_any).
    The default constructor creates a null address, and that address is
    not valid for any host or interface.
*/
bool QHostAddress::isNull() const
{
    QT_ENSURE_PARSED(this);
    if (d->isIp4)
        return d->a == 0;
    for (int i = 0; i < 16; ++i) {
        if (d->a6.c[i] != 0)
            return false;
    }
    return true;
}

/*!
    \fn bool QHostAddress::isIp4Addr() const

    Use isIPv4Address() instead.
*/

/*!
    \fn Q_UINT32 QHostAddress::ip4Addr() const

    Use toIPv4Address() instead.
*/

