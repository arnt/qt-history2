/****************************************************************************
** $Id: $
**
** Definition of QObject class
**
** Created : 930418
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QOBJECT_H
#define QOBJECT_H

#ifndef QT_H
#include "qobjectdefs.h"
#include "qwindowdefs.h"
#include "qevent.h"
#include "qnamespace.h"
#endif // QT_H

#define QT_TR_NOOP(x) (x)
#define QT_TRANSLATE_NOOP(scope,x) (x)

struct QMetaObject;
class QVariant;
class QPostEventList;
struct QObjectPrivate;
#ifndef QT_NO_USERDATA
class QObjectUserData;
#endif
class QObjectList;
class QGuardedPtrData;

class Q_EXPORT QObject: public Qt
{
    Q_OBJECT
    Q_PROPERTY( QByteArray name READ name WRITE setName )

public:
    QObject(QObject *parent=0, const char *name=0);
    virtual ~QObject();

    const char *className() const;
#ifdef Q_QDOC
    static QString tr(const char *, const char *);
    static QString trUtf8(const char *, const char *);
    virtual QMetaObject *metaObject() const;
#endif

    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    const char *name() const;
    const char *name(const char *defaultName) const;

    virtual void setName(const char *name);
    bool isWidgetType()	  const { return isWidget; }
    bool highPriority()	  const { return FALSE; }

    bool signalsBlocked()  const { return blockSig; }
    bool blockSignals(bool b);

    int startTimer(int interval);
    void killTimer(int id);
    void killTimers();

    QObject *child(const char *objName, const char *inheritsClass = 0,
		   bool recursiveSearch = true); //### const in 4.0
    const QObjectList *children() const { return childObjects; }

    static const QObjectList *objectTrees();

    QObjectList *queryList(const char *inheritsClass = 0,
			   const char *objName = 0,
			   bool regexpMatch = true,
			   bool recursiveSearch = true) const;

    virtual void insertChild(QObject *);
    virtual void removeChild(QObject *);

    void installEventFilter(const QObject *);
    void removeEventFilter(const QObject *);

    static bool connect(const QObject *sender, const char *signal,
			const QObject *receiver, const char *member);
    inline bool connect(const QObject *sender, const char *signal,
			const char *member) const
    { return connect(sender, signal, this, member); }

    static bool disconnect(const QObject *sender, const char *signal,
			   const QObject *receiver, const char *member);
    inline bool disconnect(const char *signal = 0,
			   const QObject *receiver = 0, const char *member = 0)
    { return disconnect(this, signal, receiver, member); }
    inline bool disconnect(const QObject *receiver, const char *member = 0)
    { return disconnect(this, 0, receiver, member); }

    void dumpObjectTree();
    void dumpObjectInfo();

#ifndef QT_NO_PROPERTIES
    bool setProperty(const char *name, const QVariant &value);
    QVariant property(const char *name) const;
#endif // QT_NO_PROPERTIES

#ifndef QT_NO_USERDATA
    static uint registerUserData();
    void setUserData(uint id, QObjectUserData* data);
    QObjectUserData* userData(uint id) const;
#endif // QT_NO_USERDATA

signals:
    void destroyed(QObject * = 0);

public:
    inline QObject *parent() const { return parentObj; }

public slots:
    void deleteLater();

private slots:
    void cleanupEventFilter(QObject*);

protected:
    bool activate_filters(QEvent *);
    const QObject *sender();
    int receivers(const char* signal ) const;

    virtual void timerEvent(QTimerEvent *);
    virtual void childEvent(QChildEvent *);
    virtual void customEvent(QCustomEvent *);

    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

#ifndef QT_NO_COMPAT
public:
    inline bool isA(const char *classname) const
    { return qstrcmp(classname, className() ) == 0; }
    inline bool inherits(const char *classname) const
    { return metaObject()->inherits(classname); }
protected:
    inline bool checkConnectArgs(const char *signal,
				  const QObject *,
				  const char *member)
	{ return QMetaObject::checkConnectArgs(signal, member); }
    static inline QByteArray normalizeSignalSlot(const char *signalSlot)
	{ return QMetaObject::normalizeSignature(signalSlot); }
#endif

private:
    uint isSignal : 1;
    uint isWidget : 1;
    uint pendTimer : 1;
    uint blockSig : 1;
    uint wasDeleted : 1;
    uint isTree : 1;

    const char *objname;
    QObject *parentObj;
    QObjectList *childObjects;
    QObjectList *eventFilters;
    QPostEventList *postedEvents;
    QObjectPrivate *d;

    static const QMetaObject staticQtMetaObject;

    friend struct QMetaObject;
    friend class QApplication;
    friend class QBaseApplication;
    friend class QWidget;
    friend class QSignal;
    friend class QGuardedPtrData;

private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QObject(const QObject &);
    QObject &operator=(const QObject &);
#endif
};

#ifndef QT_NO_USERDATA
class Q_EXPORT QObjectUserData {
public:
    virtual ~QObjectUserData();
};
#endif

#define Q_DEFINED_QOBJECT
#include "qwinexport.h"
#endif // QOBJECT_H
