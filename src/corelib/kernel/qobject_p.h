/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QOBJECT_P_H
#define QOBJECT_P_H

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

#include "qobject.h"
#include "qpointer.h"
#include "qcoreevent.h"
#include "qlist.h"
#include "qvector.h"
#include "qreadwritelock.h"

class QThreadData;

struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
    typedef void (*EndCallback)(QObject *caller, int method_index);
    BeginCallback signal_begin_callback,
                    slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
void Q_CORE_EXPORT qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &callback_set);

extern QSignalSpyCallbackSet Q_CORE_EXPORT qt_signal_spy_callback_set;

inline QObjectData::~QObjectData() {}

enum { QObjectPrivateVersion = QT_VERSION };

class Q_CORE_EXPORT QObjectPrivate : public QObjectData
{
    Q_DECLARE_PUBLIC(QObject)

public:
    // use this lock when implementing thread-safe QObject things (e.g. postEvent())
    static QReadWriteLock *readWriteLock();
    // note: must lockForRead() before calling isValidObject()
    static bool isValidObject(QObject *object);

    QObjectPrivate(int version = QObjectPrivateVersion);
    virtual ~QObjectPrivate();

    // id of the thread that owns the object
    int thread;
    void moveToThread_helper(QThread *targetThread);
    void setThreadId_helper(QThreadData *currentData, QThreadData *targetData, int id);
    void reregisterTimers(void *pointer);

    // object currently activating the object
    QObject *currentSender;

    bool isSender(const QObject *receiver, const char *signal) const;
    QObjectList receiverList(const char *signal) const;
    QObjectList senderList() const;

    QList<QPointer<QObject> > eventFilters;

    void setParent_helper(QObject *);

#ifndef QT_NO_USERDATA
    QVector<QObjectUserData *> userData;
#endif

    QString objectName;
};

class Q_CORE_EXPORT QMetaCallEvent : public QEvent
{
public:
    QMetaCallEvent(int id, int nargs = 0, int *types = 0, void **args = 0);
    ~QMetaCallEvent();

    inline int id() const { return id_; }
    inline void **args() const { return args_; }

private:
    int id_;
    int nargs_;
    int *types_;
    void **args_;
};

class Q_CORE_EXPORT QBoolBlocker
{
public:
    inline QBoolBlocker(bool &b):block(b), reset(b){block = true;}
    inline ~QBoolBlocker(){block = reset; }
private:
    bool &block;
    bool reset;
};

#endif // QOBJECT_P_H
