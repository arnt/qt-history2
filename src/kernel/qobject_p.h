/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
#if defined(QT_THREAD_SUPPORT)
#  include <private/qspinlock_p.h>
#endif

#define Q_DECL_PUBLIC( Class ) \
private: \
    inline Class##Private* d_func() { return this; } \
    inline const Class##Private* d_func() const { return this; } \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class


class Q_CORE_EXPORT QObjectPrivate : public Qt
{
    Q_DECL_PUBLIC( QObject );
protected:
    QObject *q_ptr;

public:

    QObjectPrivate()
	:
#if defined(QT_THREAD_SUPPORT)
	thread(0),
#endif
	connections(0),
	senders(0),
	polished(0),
	objectName(0),
	ownObjectName(false)
    {
#if defined(QT_THREAD_SUPPORT)
	spinlock.initialize();
#endif
#ifndef QT_NO_USERDATA
	userData.setAutoDelete(true);
#endif
    }
    virtual ~QObjectPrivate() {}

#if defined(QT_THREAD_SUPPORT)
    // id of the thread that owns the object
    Qt::HANDLE thread;
    QSpinLock spinlock;
#endif

    // signal connections
    struct Connections {
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
	} connections[1];
    };
    Connections *connections;
    static int *queuedConnectionTypes(const char *signal);
    void addConnection(int signal, QObject *receiver, int member, int type = 0, int *types = 0);
    Connections::Connection *findConnection(int signal, int &i) const;
    void removeReceiver(QObject *receiver);

    // slot connections
    struct Senders
    {
	int ref;
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
    void derefSenders();

    static QObject *setCurrentSender(QObject *receiver, QObject *sender);
    static void resetCurrentSender(QObject *receiver, QObject *sender);


    QObjectList children;
    QList<QPointer<QObject> > eventFilters;

#ifndef QT_NO_USERDATA
    QList<QObjectUserData *> userData;
#endif

    mutable const QMetaObject *polished;
    const char *objectName;
    uint ownObjectName : 1;
};

class QMetaCallEvent : public QEvent
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
