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

#ifndef REMOTECTRLIMPL_H
#define REMOTECTRLIMPL_H

#include "remotectrl.h"

class QSocket;

class RemoteCtrlImpl : public RemoteCtrl
{
    Q_OBJECT

public:
    RemoteCtrlImpl( QSocket * );

private slots:
    void sendImage();
    void sendText();
    void sendPalette();

private:
    void sendPacket( const QVariant & );

    QSocket *socket;
};

#endif // REMOTECTRLIMPL_H
