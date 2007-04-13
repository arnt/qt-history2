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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSCONNECTION_P_H
#define QDBUSCONNECTION_P_H

#include <qdbuserror.h>
#include <qdbusconnection.h>

#include <QtCore/qatomic.h>
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvector.h>

#include <dbus/dbus.h>

#include <qdbusmessage.h>

class QDBusMessage;
class QSocketNotifier;
class QTimerEvent;
class QDBusObjectPrivate;
class CallDeliveryEvent;
class ActivateObjectEvent;
class QMetaMethod;
class QDBusInterfacePrivate;
struct QDBusMetaObject;
class QDBusAbstractInterface;
class QDBusConnectionInterface;

class QDBusErrorInternal
{
    mutable DBusError error;
    Q_DISABLE_COPY(QDBusErrorInternal);
public:
    inline QDBusErrorInternal() { dbus_error_init(&error); }
    inline ~QDBusErrorInternal() { dbus_error_free(&error); }
    inline bool operator !() const { return !dbus_error_is_set(&error); }
    inline operator DBusError *() { dbus_error_free(&error); return &error; }
    inline operator QDBusError() const { QDBusError err(&error); dbus_error_free(&error); return err; }
};

// QDBusConnectionPrivate holds the DBusConnection and
// can have many QDBusConnection objects referring to it

class QDBusConnectionPrivate: public QObject
{
    Q_OBJECT
public:
    // structs and enums
    enum ConnectionMode { InvalidMode, ServerMode, ClientMode }; // LocalMode

    struct Watcher
    {
        Watcher(): watch(0), read(0), write(0) {}
        DBusWatch *watch;
        QSocketNotifier *read;
        QSocketNotifier *write;
    };

    struct SignalHook
    {
        inline SignalHook() : obj(0), midx(-1) { }
        QString owner, service, path, signature;
        QObject* obj;
        int midx;
        QList<int> params;
        QByteArray matchRule;
    };

    struct ObjectTreeNode
    {
        typedef QVector<ObjectTreeNode> DataList;

        inline ObjectTreeNode() : obj(0), flags(0) { }
        inline ObjectTreeNode(const QString &n) // intentionally implicit
            : name(n), obj(0), flags(0) { }
        inline ~ObjectTreeNode() { }
        inline bool operator<(const QString &other) const
            { return name < other; }
        inline bool operator<(const QStringRef &other) const
            { return QStringRef(&name) < other; }

        QString name;
        QObject* obj;
        int flags;
        DataList children;
    };

public:
    // typedefs
    typedef QMultiHash<int, Watcher> WatcherHash;
    typedef QHash<int, DBusTimeout *> TimeoutHash;
    typedef QMultiHash<QString, SignalHook> SignalHookHash;
    typedef QHash<QString, QDBusMetaObject* > MetaObjectHash;

public:
    // public methods are entry points from other objects
    explicit QDBusConnectionPrivate(QObject *parent = 0);
    ~QDBusConnectionPrivate();

    void setConnection(DBusConnection *connection, const QDBusErrorInternal &error);
    void setServer(DBusServer *server, const QDBusErrorInternal &error);
    void closeConnection();

    QString getNameOwner(const QString &service);

    int send(const QDBusMessage &message);
    QDBusMessage sendWithReply(const QDBusMessage &message, int mode, int timeout = -1);
    QDBusMessage sendWithReplyLocal(const QDBusMessage &message);
    int sendWithReplyAsync(const QDBusMessage &message, QObject *receiver,
                           const char *returnMethod, const char *errorMethod, int timeout = -1);
    void connectSignal(const QString &key, const SignalHook &hook);
    void disconnectSignal(SignalHookHash::Iterator &it);
    void registerObject(const ObjectTreeNode *node);
    void connectRelay(const QString &service, const QString &currentOwner,
                      const QString &path, const QString &interface,
                      QDBusAbstractInterface *receiver, const char *signal);
    void disconnectRelay(const QString &service, const QString &currentOwner,
                         const QString &path, const QString &interface,
                         QDBusAbstractInterface *receiver, const char *signal);

    bool handleMessage(const QDBusMessage &msg);

    QDBusMetaObject *findMetaObject(const QString &service, const QString &path,
                                    const QString &interface, QDBusError &error);

    void registerService(const QString &serviceName);
    void unregisterService(const QString &serviceName);

    void postEventToThread(int action, QObject *target, QEvent *event);

private:
    void checkThread();
    bool handleError(const QDBusErrorInternal &error);

    void handleSignal(const QString &key, const QDBusMessage &msg);
    void handleSignal(const QDBusMessage &msg);
    void handleObjectCall(const QDBusMessage &message);

    void activateSignal(const SignalHook& hook, const QDBusMessage &msg);
    void activateObject(ObjectTreeNode &node, const QDBusMessage &msg, int pathStartPos);
    bool activateInternalFilters(const ObjectTreeNode &node, const QDBusMessage &msg);
    bool activateCall(QObject *object, int flags, const QDBusMessage &msg);
    CallDeliveryEvent *prepareReply(QObject *object, int idx, const QList<int> &metaTypes,
                                    const QDBusMessage &msg);

    void sendError(const QDBusMessage &msg, QDBusError::ErrorType code);
    void deliverCall(QObject *object, int flags, const QDBusMessage &msg,
                     const QList<int> &metaTypes, int slotIdx);

    bool isServiceRegisteredByThread(const QString &serviceName) const;

protected:
    void customEvent(QEvent *e);
    void timerEvent(QTimerEvent *e);

public slots:
    // public slots
    void doDispatch();
    void socketRead(int);
    void socketWrite(int);
    void objectDestroyed(QObject *o);
    void relaySignal(QObject *obj, const QMetaObject *, int signalId, const QVariantList &args);
    void _q_serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

signals:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void callWithCallbackFailed(const QDBusError &error, const QDBusMessage &message);

public:
    QAtomic ref;
    QString name;               // this connection's name
    QString baseService;        // this connection's base service

    ConnectionMode mode;

    // members accessed in unlocked mode (except for deletion)
    // connection and server provide their own locking mechanisms
    // busService doesn't have state to be changed
    // watchers and timeouts are accessed only in the object's owning thread
    DBusConnection *connection;
    DBusServer *server;
    QDBusConnectionInterface *busService;
    WatcherHash watchers;
    TimeoutHash timeouts;

    // members accessed through a lock
    QMutex dispatchLock;
    QReadWriteLock lock;
    QDBusError lastError;

    QStringList serviceNames;
    SignalHookHash signalHooks;
    ObjectTreeNode rootNode;
    MetaObjectHash cachedMetaObjects;

    QMutex callDeliveryMutex;
    CallDeliveryEvent *callDeliveryState; // protected by the callDeliveryMutex mutex

public:
    // static methods
    static int findSlot(QObject *obj, const QByteArray &normalizedName, QList<int>& params);
    static bool prepareHook(QDBusConnectionPrivate::SignalHook &hook, QString &key,
                            const QString &service, const QString &owner,
                            const QString &path, const QString &interface, const QString &name,
                            QObject *receiver, const char *signal, int minMIdx,
                            bool buildSignature);
    static DBusHandlerResult messageFilter(DBusConnection *, DBusMessage *, void *);
    static void messageResultReceived(DBusPendingCall *, void *);

    static QDBusConnectionPrivate *d(const QDBusConnection& q) { return q.d; }

    static void setSender(const QDBusConnectionPrivate *s);

    friend class ActivateObjectEvent;
    friend class CallDeliveryEvent;
};

// in qdbusmisc.cpp
extern int qDBusParametersForMethod(const QMetaMethod &mm, QList<int>& metaTypes);
extern int qDBusNameToTypeId(const char *name);
extern bool qDBusCheckAsyncTag(const char *tag);

// in qdbusinternalfilters.cpp
extern QString qDBusIntrospectObject(const QDBusConnectionPrivate::ObjectTreeNode &node);
extern QDBusMessage qDBusPropertyGet(const QDBusConnectionPrivate::ObjectTreeNode &node,
                                     const QDBusMessage &msg);
extern QDBusMessage qDBusPropertySet(const QDBusConnectionPrivate::ObjectTreeNode &node,
                                     const QDBusMessage &msg);

// in qdbusxmlgenerator.cpp
extern QString qDBusInterfaceFromMetaObject(const QMetaObject *mo);

#endif
