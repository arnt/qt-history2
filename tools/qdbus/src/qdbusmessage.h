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
#include <QtDBus/qdbusconnection.h>
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>

#include <limits.h>

QT_BEGIN_HEADER

class QDBusConnection;
class QDBusConnectionPrivate;

class QDBusMessagePrivate;
class QDBUS_EXPORT QDBusMessage
{
public:
    enum MessageType { InvalidMessage, MethodCallMessage, ReplyMessage,
                       ErrorMessage, SignalMessage };

    QDBusMessage();
    QDBusMessage(const QDBusMessage &other);
    QDBusMessage &operator=(const QDBusMessage &other);
    ~QDBusMessage();

    static QDBusMessage signal(const QString &path, const QString &interface,
                               const QString &name,
                               const QDBusConnection &connection /*= QDBus::sessionBus()*/);
    static QDBusMessage methodCall(const QString &destination, const QString &path,
                                   const QString &interface, const QString &method,
                                   const QDBusConnection &connection /*= QDBus::sessionBus()*/);

    QDBusConnection connection() const;
    QString service() const;
    QString path() const;
    QString interface() const;
    QString member() const;
    MessageType type() const;

    bool isReplyRequired() const;
    QString signature() const;

    void setDelayedReply(bool enable) const;
    bool isDelayedReply() const;

    void setArguments(const QList<QVariant> &arguments);
    const QList<QVariant> &arguments() const;

    int count() const;
    inline bool isEmpty() const { return count() == 0; }
    const QVariant &at(int index) const;

    inline QDBusMessage &operator<<(const QVariant &arg)
    { append(arg); return *this; }
    inline QDBusMessage &operator+=(const QVariant &arg)
    { append(arg); return *this; }
    void append(const QVariant &arg);

    bool send();
    bool sendError(const QString &name, const QString &message = QString()) const;
    bool sendError(const QDBusError &error) const;
    bool sendReply(const QVariantList &arguments = QVariantList()) const;
    bool sendReply(const QVariant &returnValue) const;

private:
#ifndef Q_QDOC
    template<typename T> inline QVariant qvfv(const T &t);
#ifndef QT_NO_CAST_FROM_ASCII
    inline QVariant qvfv(const char *t)
    { return QVariant(t); }
#endif
#endif

    friend class QDBusMessagePrivate;
    QDBusMessagePrivate *d_ptr;
};

#ifndef QT_NO_DEBUG_STREAM
QDBUS_EXPORT QDebug operator<<(QDebug, const QDBusMessage &);
#endif

QT_END_HEADER

#endif

