/****************************************************************************
** $Id: .emacs,v 1.5 1999/05/06 19:35:46 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

class QHostAddressPrivate;

class QHostAddress
{
public:
    QHostAddress();
    QHostAddress( Q_UINT32 ip4Addr );
    QHostAddress( Q_UINT8 *ip6Addr );
    QHostAddress( const QHostAddress & );
   ~QHostAddress();

    QHostAddress & operator=( const QHostAddress & );

    void setAddress( Q_UINT32 ip4Addr );
    void setAddress( Q_UINT8 *ip6Addr );
    bool setAddress( const QString& address );

    bool	 isIp4Addr()	 const;
    Q_UINT32	 ip4Addr()	 const;
    QString	 toString() const;

    bool	 operator==( const QHostAddress & ) const;

private:
    QHostAddressPrivate* d;
};



#endif
