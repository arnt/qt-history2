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

#ifndef QDBUSERROR_H
#define QDBUSERROR_H

#include <QtDBus/qdbusmacros.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER


struct DBusError;
class QDBusMessage;

class QDBUS_EXPORT QDBusError
{
public:
    enum KnownErrors {
        NoError = 0,
        Other = 1,
        Failed,
        NoMemory,
        ServiceUnknown,
        NoReply,
        BadAddress,
        NotSupported,
        LimitsExceeded,
        AccessDenied,
        NoServer,
        Timeout,
        NoNetwork,
        AddressInUse,
        Disconnected,
        InvalidArgs,
        UnknownMethod,
        TimedOut,
        InvalidSignature,
        UnknownInterface,
        InternalError,

#ifndef Q_QDOC        
        // don't use this one!
        qKnownErrorsMax = InternalError
#endif
    };
    
    QDBusError(const DBusError *error = 0);
    QDBusError(const QDBusMessage& msg);
    QDBusError(KnownErrors error, const QString &message);

    inline QString name() const { return nm; }
    inline QString message() const { return msg; }
    inline bool isValid() const { return !nm.isNull() && !msg.isNull(); }

    inline bool operator==(KnownErrors error) const
    { return code == error; }

private:
    KnownErrors code;
    QString nm, msg;
};

inline bool operator==(QDBusError::KnownErrors p1, const QDBusError &p2)
{ return p2 == p1; }
inline bool operator!=(QDBusError::KnownErrors p1, const QDBusError &p2)
{ return !(p2 == p1); }
inline bool operator!=(const QDBusError &p1, QDBusError::KnownErrors p2)
{ return !(p1 == p2); }

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const QDBusError &);
#endif

QT_END_HEADER

#endif
