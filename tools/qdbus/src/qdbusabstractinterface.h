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
 
#ifndef QDBUSABSTRACTINTERFACE_H
#define QDBUSABSTRACTINTERFACE_H

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>

#include <QtDBus/qdbusmessage.h>
#include <QtDBus/qdbusextratypes.h>

QT_BEGIN_HEADER


class QDBusConnection;
class QDBusError;

class QDBusAbstractInterfacePrivate;
class QDBUS_EXPORT QDBusAbstractInterface: public QObject
{
    Q_OBJECT

public:
    enum CallMode {
        NoWaitForReply,
        UseEventLoop,
        NoUseEventLoop,
        AutoDetect
    };

public:
    virtual ~QDBusAbstractInterface();
    bool isValid() const;

    QDBusConnection connection() const;

    QString service() const;
    QString path() const;
    QString interface() const;

    QDBusError lastError() const;

    QDBusMessage callWithArgs(const QString &method, const QList<QVariant> &args = QList<QVariant>(),
                              CallMode mode = AutoDetect);
    bool callWithArgs(const QString &method, QObject *receiver, const char *slot,
                      const QList<QVariant> &args = QList<QVariant>());

    inline QDBusMessage call(const QString &m)
    {
        return callWithArgs(m);
    }

    inline QDBusMessage call(CallMode mode, const QString &m)
    {
        return callWithArgs(m, QList<QVariant>(), mode);
    }
    
#ifndef Q_QDOC
private:
    template<typename T> inline QVariant qvfv(const T &t);
#ifndef QT_NO_CAST_FROM_ASCII
    inline QVariant qvfv(const char *t)
    { return QVariant(t); }
#endif

public:
    template<typename T1>
    inline QDBusMessage call(const QString &m, const T1 &t1)
    {
        QList<QVariant> args;
        args << qvfv(t1);
        return callWithArgs(m, args);
    }

    template<typename T1, typename T2>
    inline QDBusMessage call(const QString &m, const T1 &t1, const T2 &t2)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2);
        return callWithArgs(m, args);
    }

    template<typename T1, typename T2, typename T3>
    inline QDBusMessage call(const QString &m, const T1 &t1, const T2 &t2, const T3 &t3)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3);
        return callWithArgs(m, args);
    }
      
    template<typename T1, typename T2, typename T3, typename T4>
    inline QDBusMessage call(const QString &m, const T1 &t1, const T2 &t2, const T3 &t3,
                             const T4 &t4)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4);
        return callWithArgs(m, args);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    inline QDBusMessage call(const QString &m, const T1 &t1, const T2 &t2, const T3 &t3,
                             const T4 &t4, const T5 &t5)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5);
        return callWithArgs(m, args);
    }
  
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    inline QDBusMessage call(const QString &m, const T1 &t1, const T2 &t2, const T3 &t3,
                             const T4 &t4, const T5 &t5, const T6 &t6)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5) << qvfv(t6);
        return callWithArgs(m, args);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    inline QDBusMessage call(const QString &m, const T1 &t1, const T2 &t2, const T3 &t3,
                             const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5) << qvfv(t6)
             << qvfv(t7);
        return callWithArgs(m, args);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
             typename T6, typename T7, typename T8>
    inline QDBusMessage call(const QString &m, const T1 &t1, const T2 &t2, const T3 &t3,
                             const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5) << qvfv(t6)
             << qvfv(t7) << qvfv(t8);
        return callWithArgs(m, args);
    }

    template<typename T1>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1)
    {
        QList<QVariant> args;
        args << qvfv(t1);
        return callWithArgs(m, args, mode);
    }

    template<typename T1, typename T2>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1, const T2 &t2)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2);
        return callWithArgs(m, args, mode);
    }

    template<typename T1, typename T2, typename T3>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1, const T2 &t2,
                             const T3 &t3)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3);
        return callWithArgs(m, args, mode);
    }
      
    template<typename T1, typename T2, typename T3, typename T4>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1, const T2 &t2,
                             const T3 &t3, const T4 &t4)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4);
        return callWithArgs(m, args, mode);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1, const T2 &t2,
                             const T3 &t3, const T4 &t4, const T5 &t5)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5);
        return callWithArgs(m, args, mode);
    }
  
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1, const T2 &t2,
                             const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5) << qvfv(t6);
        return callWithArgs(m, args, mode);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1, const T2 &t2,
                             const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5) << qvfv(t6)
             << qvfv(t7);
        return callWithArgs(m, args, mode);
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5,
             typename T6, typename T7, typename T8>
    inline QDBusMessage call(CallMode mode, const QString &m, const T1 &t1, const T2 &t2,
                             const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7,
                             const T8 &t8)
    {
        QList<QVariant> args;
        args << qvfv(t1) << qvfv(t2) << qvfv(t3)
             << qvfv(t4) << qvfv(t5) << qvfv(t6)
             << qvfv(t7) << qvfv(t8);
        return callWithArgs(m, args, mode);
    }
#endif

protected:
    QDBusAbstractInterface(QDBusAbstractInterfacePrivate *);
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
    QVariant internalPropGet(const char *propname) const;
    void internalPropSet(const char *propname, const QVariant &value);

private:
    friend class QDBusInterface;

    Q_DECLARE_PRIVATE(QDBusAbstractInterface)
    Q_DISABLE_COPY(QDBusAbstractInterface)
};

template<typename T> inline QVariant QDBusAbstractInterface::qvfv(const T &t)
{ return qVariantFromValue(t); }

template<> inline QVariant QDBusAbstractInterface::qvfv(const QVariant &t)
{ return qVariantFromValue(QDBusVariant(t)); }

template<> inline QVariant QDBusAbstractInterface::qvfv(const QLatin1String &str)
{ return QVariant(str); }

QT_END_HEADER

#endif
