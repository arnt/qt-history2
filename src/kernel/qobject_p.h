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

#include "qlist.h"
#include "qobject.h"
#include "qpointer.h"

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
#ifndef QT_NO_USERDATA
	userData.setAutoDelete(true);
#endif
    }
    virtual ~QObjectPrivate() {}

#if defined(QT_THREAD_SUPPORT)
    // id of the thread that owns the object
    Qt::HANDLE thread;
#endif

    // signal connections
    struct Connections {
	int count;
	struct Connection{
	    int signal;
	    union {
		QObject *receiver;
		QObject **guarded;
	    };
	    int member;
	    int *types;
	} connections[1];
    };
    Connections *connections;
    void addConnection(int signal, QObject *receiver, int member, int *types = 0);
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

    static QObject *setCurrentSender(Senders *senders, QObject *sender);
    static void resetCurrentSender(Senders *senders, QObject *sender);
    static void derefSenders(Senders *senders);


    QObjectList children;
    QList<QPointer<QObject> > eventFilters;

#ifndef QT_NO_USERDATA
    QList<QObjectUserData *> userData;
#endif

    mutable const QMetaObject *polished;
    const char *objectName;
    uint ownObjectName : 1;
};



#endif
