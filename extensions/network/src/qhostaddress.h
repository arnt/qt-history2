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


class Q_EXPORT QHostAddress
{
public:
    QHostAddress();
    QHostAddress( uint ip4Addr );
    QHostAddress( const QHostAddress & );
   ~QHostAddress();

    QHostAddress & operator=( const QHostAddress & );

    uint	 ip4Addr()	 const;
    QString	 string() const;

    bool	 operator==( const QHostAddress & );

private:
    void * d;
    uint a;
};



#endif
