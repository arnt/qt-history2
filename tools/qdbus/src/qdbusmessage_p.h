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
struct DBusMessage;
class QDBusConnectionPrivate;

class QDBusMessagePrivate
{
public:
    QDBusMessagePrivate();
    ~QDBusMessagePrivate();

    QList<QVariant> arguments;
    QString service, path, interface, name, message, signature;
    DBusMessage *msg;
    DBusMessage *reply;
    int type;
    int timeout;
    mutable QDBusMessage *localReply;
    QAtomic ref;

    mutable uint delayedReply : 1;
    uint localMessage : 1;

    static DBusMessage *toDBusMessage(const QDBusMessage &message);
    static QDBusMessage fromDBusMessage(DBusMessage *dmsg);
    static QDBusMessage fromError(const QDBusError& error);
    static QDBusMessage updateSignature(const QDBusMessage &message, DBusMessage *dmsg);

    static bool isLocal(const QDBusMessage &msg);
    static QDBusMessage makeLocal(const QDBusConnectionPrivate &conn,
                                  const QDBusMessage &asSent);
    static QDBusMessage makeLocalReply(const QDBusConnectionPrivate &conn,
                                       const QDBusMessage &asSent);
};

#endif
