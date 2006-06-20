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

#ifndef QDBUSREPLY_H
#define QDBUSREPLY_H

#include <QtCore/qglobal.h>
#include <QtCore/qvariant.h>

#include "qdbusmacros.h"
#include "qdbusmessage.h"
#include "qdbuserror.h"
#include "qdbusextratypes.h"

QDBUS_EXPORT void qDBusReplyFill(const QDBusMessage &reply, QDBusError &error, QVariant &data);

template<typename T>
class QDBusReply
{
    typedef T Type;
public:
    inline QDBusReply(const QDBusMessage &reply)
    {
        *this = reply;
    }
    inline QDBusReply& operator=(const QDBusMessage &reply)
    {
        void *null = 0;
        QVariant data(qt_variant_metatype_id(&m_data), null);
        qDBusReplyFill(reply, m_error, data);
        m_data = qvariant_cast<Type>(data);
        return *this;
    }

    inline QDBusReply(const QDBusError &dbusError = QDBusError())
        : m_error(dbusError), m_data(Type())
    {
    }
    inline QDBusReply& operator=(const QDBusError& dbusError)
    {
        m_error = dbusError;
        m_data = Type();
        return *this;
    }

    inline QDBusReply& operator=(const QDBusReply& other)
    {
        m_error = other.m_error;
        m_data = other.m_data;
        return *this;
    }

    inline bool isError() const { return m_error.isValid(); }
    inline bool isSuccess() const { return !m_error.isValid(); }

    inline const QDBusError& error() { return m_error; }

    inline Type value() const
    {
        return m_data;
    }

    inline operator Type () const
    {
        return m_data;
    }

#if 0
    static QDBusReply<T> fromVariant(const QDBusReply<QVariant> &variantReply)
    {
        QDBusReply<T> retval;
        retval.m_error = variantReply.m_error;
        if (retval.isSuccess()) {
            retval.m_data = qvariant_cast<Type>(variantReply.m_data);
            if (!qVariantCanConvert<Type>(variantReply.m_data))
                retval.m_error = QDBusError(QDBusError::InvalidSignature,
                                            QLatin1String("Unexpected reply signature"));
        }
        return retval;
    }
#endif

private:
    QDBusError m_error;
    Type m_data;
};

# ifndef Q_QDOC
// specialize for QVariant:
template<> inline QDBusReply<QVariant>&
QDBusReply<QVariant>::operator=(const QDBusMessage &reply)
{
    void *null = 0;
    QVariant data(qMetaTypeId<QDBusVariant>(), null);
    qDBusReplyFill(reply, m_error, data);
    m_data = qvariant_cast<QDBusVariant>(data).value;
    return *this;
}

// specialize for void:
template<>
class QDBusReply<void>
{
public:
    inline QDBusReply(const QDBusMessage &reply)
        : m_error(reply)
    {
    }
    inline QDBusReply(const QDBusError &dbusError)
        : m_error(dbusError)
    {
    }

    inline bool isError() const { return m_error.isValid(); }
    inline bool isSuccess() const { return !m_error.isValid(); }

    inline const QDBusError& error() { return m_error; }

private:
    QDBusError m_error;
};
# endif

#endif
