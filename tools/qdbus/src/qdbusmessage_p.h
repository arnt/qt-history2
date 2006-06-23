/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDBUSMESSAGE_P_H
#define QDBUSMESSAGE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <qatomic.h>
#include <qstring.h>
#include <qdbusconnection.h>
struct DBusMessage;

class QDBusMessagePrivate
{
public:
    QDBusMessagePrivate();
    ~QDBusMessagePrivate();

    QString service, path, interface, name, message, signature;
    QDBusConnection connection;
    DBusMessage *msg;
    DBusMessage *reply;
    int type;
    int timeout;
    QAtomic ref;

    mutable bool repliedTo : 1;
};

#endif
