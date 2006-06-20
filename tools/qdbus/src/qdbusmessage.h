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

#ifndef QDBUSMESSAGE_H
#define QDBUSMESSAGE_H

#include <QtDBus/qdbusmacros.h>
#include <QtDBus/qdbuserror.h>
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>

#include <limits.h>

QT_BEGIN_HEADER


class QDBusMessagePrivate;
class QDBusConnection;
class QDBusConnectionPrivate;
struct DBusMessage;

class QDBUS_EXPORT QDBusMessage: public QList<QVariant>
{
    //friend class QDBusConnection;
    friend class QDBusConnectionPrivate;
public:
    enum { DefaultTimeout = -1, NoTimeout = INT_MAX};
    enum MessageType { InvalidMessage, MethodCallMessage, ReplyMessage,
                       ErrorMessage, SignalMessage };

    QDBusMessage();
    QDBusMessage(const QDBusMessage &other);
    ~QDBusMessage();

    QDBusMessage &operator=(const QDBusMessage &other);

    static QDBusMessage signal(const QString &path, const QString &interface,
                               const QString &name);
    static QDBusMessage methodCall(const QString &destination, const QString &path,
                                   const QString &interface, const QString &method);
    static QDBusMessage methodReply(const QDBusMessage &other);
    static QDBusMessage error(const QDBusMessage &other, const QString &name,
                              const QString &message = QString());
    static QDBusMessage error(const QDBusMessage &other, const QDBusError &error);

    QString path() const;
    QString interface() const;
    QString name() const;
    inline QString member() const { return name(); }
    inline QString method() const { return name(); }
    QString service() const;
    inline QString sender() const { return service(); }
    MessageType type() const;

    int timeout() const;
    void setTimeout(int ms);

    bool noReply() const;

    QString signature() const;

    QDBusConnection connection() const;

    int serialNumber() const;
    int replySerialNumber() const;
    bool wasRepliedTo() const;

private:
    friend class QDBusError;
    DBusMessage *toDBusMessage() const;
    static QDBusMessage fromDBusMessage(DBusMessage *dmsg, const QDBusConnection &connection);
    static QDBusMessage fromError(const QDBusError& error);
    QDBusMessagePrivate *d_ptr;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const QDBusMessage &);
#endif

QT_END_HEADER

#endif

