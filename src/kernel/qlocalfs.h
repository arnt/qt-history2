/****************************************************************************
**
** Definition of QLocalFs class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
