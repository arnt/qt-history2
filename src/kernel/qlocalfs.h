/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlocalfs.h#6 $
**
** Definition of QLocalFs class
**
** Created : 950429
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QLOCALFS_H
#define QLOCALFS_H

#ifndef QT_H
#include "qnetworkprotocol.h"
#include "qdir.h"
#endif // QT_H

#ifndef QT_NO_NETWORKPROTOCOL

class Q_EXPORT QLocalFs : public QNetworkProtocol
{
    Q_OBJECT

public:
    QLocalFs();
    virtual int supportedOperations() const;

protected:
    virtual void operationListChildren( QNetworkOperation *op );
    virtual void operationMkDir( QNetworkOperation *op );
    virtual void operationRemove( QNetworkOperation *op );
    virtual void operationRename( QNetworkOperation *op );
    virtual void operationGet( QNetworkOperation *op );
    virtual void operationPut( QNetworkOperation *op );

private:
    int calcBlockSize( int totalSize ) const;
    QDir dir;

};

#endif // QT_NO_NETWORKPROTOCOL

#endif // QLOCALFS_H
