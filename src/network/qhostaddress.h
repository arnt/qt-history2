/****************************************************************************
** $Id: //depot/qt/main/src/network/qhostaddress.h#1 $
**
** Definition of QHostAddress class.
**
** Created : 979899
**
** Copyright (C) 1997-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the network
** module and therefore may only be used if the network module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

class QHostAddressPrivate;

class Q_EXPORT QHostAddress
{
public:
    QHostAddress();
    QHostAddress( Q_UINT32 ip4Addr );
    QHostAddress( Q_UINT8 *ip6Addr );
    QHostAddress( const QHostAddress & );
    virtual ~QHostAddress();

    QHostAddress & operator=( const QHostAddress & );

    void setAddress( Q_UINT32 ip4Addr );
    void setAddress( Q_UINT8 *ip6Addr );
#ifndef QT_NO_STRINGLIST
    bool setAddress( const QString& address );
#endif
    bool	 isIp4Addr()	 const;
    Q_UINT32	 ip4Addr()	 const;
    QString	 toString() const;

    bool	 operator==( const QHostAddress & ) const;

private:
    QHostAddressPrivate* d;
};


#endif
