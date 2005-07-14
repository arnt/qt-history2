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

#ifndef Q3LOCALFS_H
#define Q3LOCALFS_H

#ifndef QT_H
#include "Qt3Support/q3networkprotocol.h"
#include "QtCore/qdir.h"
#endif // QT_H

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_NETWORKPROTOCOL

class Q_COMPAT_EXPORT Q3LocalFs : public Q3NetworkProtocol
{
    Q_OBJECT

public:
    Q3LocalFs();
    virtual int supportedOperations() const;

protected:
    virtual void operationListChildren( Q3NetworkOperation *op );
    virtual void operationMkDir( Q3NetworkOperation *op );
    virtual void operationRemove( Q3NetworkOperation *op );
    virtual void operationRename( Q3NetworkOperation *op );
    virtual void operationGet( Q3NetworkOperation *op );
    virtual void operationPut( Q3NetworkOperation *op );

private:
    int calcBlockSize( int totalSize ) const;
    QDir dir;

};

#endif // QT_NO_NETWORKPROTOCOL

#endif // Q3LOCALFS_H
