/****************************************************************************
**
** Definition of QObject class.
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

#ifndef QOBJECT_H
#define QOBJECT_H

#ifndef QT_H
#include "qobjectdefs.h"
#include "qwindowdefs.h"
#include "qnamespace.h"
#include "qstring.h"
#include "qbytearray.h"
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
class QEvent;
class QTimerEvent;
class QChildEvent;
class QCustomEvent;
template<typename T>class QList;
typedef QList<QObject *> QObjectList;

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

    bool signalsBlocked()  const { return blockSig; }
    bool blockSignals(bool b);

    int startTimer(int interval);
    void killTimer(int id);

    QObject *child(const char *objName, const char *inheritsClass = 0,
		   bool recursiveSearch = true) const;
    QObjectList children() const;

    QObjectList queryList(const char *inheritsClass = 0,
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

    bool isAncestorOf(QObject *child) const;

signals:
    void destroyed(QObject * = 0);

public:
    inline QObject *parent() const { return parentObj; }

public slots:
    void deleteLater();

private slots:
    // ### replace by QGuardedPtr
    void cleanupEventFilter(QObject*);

protected:
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
	{ return QMetaObject::normalizedSignature(signalSlot); }
#endif

protected:
    explicit QObject(QObjectPrivate *d, QObject *parent, const char *name);
private:
    uint isSignal : 1;
    uint isWidget : 1;
    uint pendTimer : 1;
    uint blockSig : 1;
    uint wasDeleted : 1;
    uint unused : 27;

    const char *objname;
    QObject *parentObj;
    QPostEventList *postedEvents;
    QObjectPrivate *d;

    static const QMetaObject staticQtMetaObject;

    friend struct QMetaObject;
    friend class QApplication;
    friend class QBaseApplication;
    friend class QWidget;
    friend class QSignal;

private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QObject(const QObject &);
    QObject &operator=(const QObject &);
#endif
};

inline bool QObject::isAncestorOf(QObject *child) const
{
    while (child) {
	child = child->parentObj;
	if (child == this)
	    return true;
    }
    return false;
}


#ifndef QT_NO_USERDATA
class Q_EXPORT QObjectUserData {
public:
    virtual ~QObjectUserData();
};
#endif

template <class T>
class QPointer
{
    QObject *o;
public:
    inline QPointer() : o(0) {}
    inline QPointer(T *obj) : o(obj)
	{ QMetaObject::addGuard(&o); }
    inline QPointer(const QPointer<T> &p) : o(p.o)
	{ QMetaObject::addGuard(&o); }
    inline ~QPointer()
	{ QMetaObject::removeGuard(&o); }
    inline QPointer<T> &operator=(const QPointer<T> &p)
	{ QMetaObject::changeGuard(&o, p.o); return *this; }
    inline QPointer<T> &operator=(T* obj)
	{ QMetaObject::changeGuard(&o, obj); return *this; }

    inline bool operator==( const QPointer<T> &p ) const
	{ return (o == p.o); }
    inline bool operator!= ( const QPointer<T>& p ) const
	{ return (o != p.o); }

    inline bool isNull() const
	{ return !o; }

    inline T* operator->() const
	{ return static_cast<T*>(const_cast<QObject*>(o)); }
    inline T& operator*() const
	{ return *static_cast<T*>(const_cast<QObject*>(o)); }
    inline operator T*() const
	{ return static_cast<T*>(const_cast<QObject*>(o)); }
};

typedef QPointer<QObject> QObjectPointer;

#define Q_DEFINED_QOBJECT
#include "qwinexport.h"
#endif // QOBJECT_H
