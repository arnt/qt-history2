/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QOBJECT_P_H
#define QOBJECT_P_H

#ifndef QT_H
#endif // QT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//
//

#include "qobject.h"
#include "qpointer.h"
#include "qcoreevent.h"

#include <qlist.h>
#include <private/qspinlock_p.h>


inline QObjectData::~QObjectData() {}


class Q_CORE_EXPORT QObjectPrivate : public QObjectData
{
    Q_DECLARE_PUBLIC(QObject)

public:
    QObjectPrivate();
    virtual ~QObjectPrivate();

    // pointer to the thread that owns the object
    QThread *thread;

    // signal connections
    struct Connections {
        QSpinLock lock;
        uint active : 1;
        uint dirty : 1;
        uint orphaned : 1;
        int count;
        struct Connection {
            int signal;
            union {
                QObject *receiver;
                QObject **guarded;
            };
            int member;
            int type; // 0 == auto, 1 == direct, 2 == queued
            int *types;
        };
        Connection *connections;
        Connection stack[1];
    };
    Connections *connections;
    static int *queuedConnectionTypes(const char *signal);
    void addConnection(int signal, QObject *receiver, int member, int type = 0, int *types = 0);
    Connections::Connection *findConnection(int signal, int &i) const;
    void removeReceiver(QObject *receiver);

    static void setActive(Connections *connections, bool *was_active);
    static void resetActive(Connections *connections, bool was_active);

    // slot connections
    struct Senders
    {
        QSpinLock lock;
        uint active : 1;
        uint orphaned : 1;
        QObject *current;
        int count;
        struct Sender {
            int ref;
            QObject * sender;
        };
        Sender *senders;
        Sender stack[1];
    };
    Senders *senders;
    void refSender(QObject *sender);
    void derefSender(QObject *sender);
    void removeSender(QObject *sender);

    static QObject *setCurrentSender(Senders *senders, QObject *sender, bool *was_active);
    static void resetCurrentSender(Senders *senders, QObject *sender, bool was_active);

    QList<QPointer<QObject> > eventFilters;

#ifndef QT_NO_USERDATA
    QList<QObjectUserData *> userData;
#endif

    QString objectName;
};

class Q_CORE_EXPORT QMetaCallEvent : public QEvent
{
public:
    QMetaCallEvent(Type type, int id, const QObject *sender = 0, int nargs = 0,
                   int *types = 0, void **args = 0);
    ~QMetaCallEvent();

    inline int id() const { return id_; }
    inline void **args() const { return args_; }
    inline const QObject *sender() const { return sender_; }

private:
    int id_;
    const QObject *sender_;
    int nargs_;
    int *types_;
    void **args_;
};

#endif
