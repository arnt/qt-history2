/****************************************************************************
**
** Definition of QHostAddress class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#if !defined( QT_MODULE_NETWORK ) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_NETWORK )
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_EXPORT
#endif

#ifndef QT_NO_NETWORK
class QHostAddressPrivate;

typedef struct {
    union {
	Q_UINT8 c[16];
	Q_UINT16 s[8];
	Q_UINT32 u[4];
    };
} Q_IPV6ADDR;

class QM_EXPORT_NETWORK QHostAddress
{
public:
    QHostAddress();
    QHostAddress( Q_UINT32 ip4Addr );
    QHostAddress( Q_UINT8 *ip6Addr );
    QHostAddress(const Q_IPV6ADDR &ip6Addr);
#ifndef QT_NO_STRINGLIST
    QHostAddress(const QString &address);
#endif
    QHostAddress( const QHostAddress & );
    virtual ~QHostAddress();

    QHostAddress & operator=( const QHostAddress & );

    void setAddress( Q_UINT32 ip4Addr );
    void setAddress( Q_UINT8 *ip6Addr );
#ifndef QT_NO_STRINGLIST
    bool setAddress( const QString& address );
#endif
    bool	 isIp4Addr()	 const; // obsolete
    Q_UINT32	 ip4Addr()	 const; // obsolete

    bool	 isIPv4Address() const;
    Q_UINT32	 toIPv4Address() const;
    bool	 isIPv6Address() const;
    Q_IPV6ADDR	 toIPv6Address() const;

#ifndef QT_NO_SPRINTF
    QString	 toString() const;
#endif

    bool	 operator==( const QHostAddress & ) const;
    bool	 isNull() const;

private:
    QHostAddressPrivate* d;
};

#endif //QT_NO_NETWORK
#endif
