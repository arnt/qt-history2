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


inline QObjectData::~QObjectData() {}

enum { QObjectPrivateVersion = QT_VERSION };

class Q_CORE_EXPORT QObjectPrivate : public QObjectData
{
    Q_DECLARE_PUBLIC(QObject)

public:
    QObjectPrivate(int version = QObjectPrivateVersion);
    virtual ~QObjectPrivate();

    // id of the thread that owns the object
    int thread;

    // object currently activating the object
    QObject *currentSender;

    bool isSender(const QObject *receiver, const char *signal) const;
    QObjectList receiverList(const char *signal) const;
    QObjectList senderList() const;

    QList<QPointer<QObject> > eventFilters;

    void setParent_helper(QObject *);

#ifndef QT_NO_USERDATA
    QList<QObjectUserData *> userData;
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

#endif // QOBJECT_P_H
