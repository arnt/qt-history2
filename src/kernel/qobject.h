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
#include "qnamespace.h"
#include "qstring.h"
#include "qbytearray.h"
#include "qlist.h"
#endif // QT_H

#ifdef QT_INCLUDE_COMPAT
#include "qcoreevent.h"
#endif

#define QT_TR_NOOP(x) (x)
#define QT_TRANSLATE_NOOP(scope,x) (x)

class QEvent;
class QTimerEvent;
class QChildEvent;
class QCustomEvent;
struct QMetaObject;
class QCoreVariant;
class QObjectPrivate;
class QWidgetPrivate;
#ifndef QT_NO_REGEXP
class QRegExp;
#endif
#ifndef QT_NO_USERDATA
class QObjectUserData;
#endif

typedef QList<QObject*> QObjectList;

class Q_CORE_EXPORT QObject: public Qt
{
    Q_OBJECT
    Q_PROPERTY( QByteArray objectName READ objectName WRITE setObjectName )

public:
    QObject(QObject *parent=0);
    QObject(QObject *parent, const char *name); // deprecated
    virtual ~QObject();

    const char *className() const;
#ifdef Q_QDOC
    static QString tr(const char *, const char *);
    static QString trUtf8(const char *, const char *);
    virtual const QMetaObject *metaObject() const;
#endif
#ifdef QT_NO_TRANSLATION
    static QString tr( const char *sourceText, const char * = 0)
	{ return QString::fromLatin1(sourceText); }
#ifndef QT_NO_TEXTCODEC
    static QString trUtf8( const char *sourceText, const char * = 0)
	{ return QString::fromUtf8(sourceText); }
#endif
#endif //QT_NO_TRANSLATION

    const char *objectName() const;
    const char *objectName(const char *defaultName) const;
    void setObjectName(const char *name);
    void setObjectNameConst(const char *name);

    inline bool isWidgetType() const { return isWidget; }

    inline bool signalsBlocked() const { return blockSig; }
    bool blockSignals(bool b);

#if defined(QT_THREAD_SUPPORT)
    Qt::HANDLE thread() const;
    void setThread(Qt::HANDLE thread);
#endif

    int startTimer(int interval);
    void killTimer(int id);

    QObject *findChild(const char *name) const;
    QObjectList findChildren(const char *name) const;

    // MOC_SKIP_BEGIN
#ifndef QT_MOC_CPP
    template<typename T>
    T findChild(const char *name, T* = 0) const
    { return static_cast<T>(findChild_helper(name, ((T)0)->staticMetaObject)); }

    template<typename T>
    QList<T> findChildren(const char *name, T* = 0) const
    {
	QList<T> list;
	findChildren_helper(name, 0, ((T)0)->staticMetaObject,
			    reinterpret_cast<QList<void *>*>(&list));
	return list;
    }
#endif //QT_MOC_CPP
    // MOC_SKIP_END    
#ifndef QT_NO_REGEXP
    QObjectList findChildren(const QRegExp &re) const;
    // MOC_SKIP_BEGIN
#ifndef QT_MOC_CPP
    template<typename T>
    QList<T> findChildren(const QRegExp &re, T* = 0) const
    {
	QList<T> list;
	findChildren_helper(0, &re, ((T)0)->staticMetaObject,
			    reinterpret_cast<QList<void*>*>(&list));
	return list;
    }
#endif
    // MOC_SKIP_END
#endif
    
#ifdef QT_COMPAT
    QObject *child(const char *objName, const char *inheritsClass = 0,
		   bool recursiveSearch = true) const;
    QObjectList queryList(const char *inheritsClass = 0,
			  const char *objName = 0,
			  bool regexpMatch = true,
			  bool recursiveSearch = true) const;
#endif
    const QObjectList &children() const;

    void setParent(QObject *);
    void installEventFilter(const QObject *);
    void removeEventFilter(const QObject *);


    static bool connect(const QObject *sender, const char *signal,
			const QObject *receiver, const char *member,
			ConnectionType = AutoConnection);
    inline bool connect(const QObject *sender, const char *signal,
			const char *member, ConnectionType type = AutoConnection) const
	{ return connect(sender, signal, this, member, type); }

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
    bool setProperty(const char *name, const QCoreVariant &value);
    QCoreVariant property(const char *name) const;
#endif // QT_NO_PROPERTIES

#ifndef QT_NO_USERDATA
    static uint registerUserData();
    void setUserData(uint id, QObjectUserData* data);
    QObjectUserData* userData(uint id) const;
#endif // QT_NO_USERDATA

    bool isAncestorOf(const QObject *child) const;

    void ensurePolished() const;

signals:
    void destroyed(QObject * = 0);

public:
    inline QObject *parent() const { return parentObj; }

    inline bool inherits(const char *classname) const
	{ return qt_metacast(classname) != 0; }

public slots:
    void deleteLater();

protected:
    const QObject *sender();
    int receivers(const char* signal ) const;

    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual void timerEvent(QTimerEvent *);
    virtual void childEvent(QChildEvent *);
    virtual void polishEvent(QEvent *);
    virtual void customEvent(QCustomEvent *);

    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

#ifdef QT_COMPAT
public:
    inline QT_COMPAT void insertChild(QObject *o)
	{ if (o) o->setParent(this); }
    inline QT_COMPAT void removeChild(QObject *o)
	{ if (o) o->setParent(0); }
    inline QT_COMPAT bool isA(const char *classname) const
	{ return qstrcmp(classname, className() ) == 0; }
    inline QT_COMPAT const char *name() const { return objectName(); }
    inline QT_COMPAT const char *name(const char *defaultName) const { return objectName(defaultName); }
    inline QT_COMPAT void setName(const char *name) { setObjectName(name); }
protected:
    inline QT_COMPAT bool checkConnectArgs(const char *signal,
				  const QObject *,
				  const char *member)
	{ return QMetaObject::checkConnectArgs(signal, member); }
    static inline QT_COMPAT QByteArray normalizeSignalSlot(const char *signalSlot)
	{ return QMetaObject::normalizedSignature(signalSlot); }
#endif

protected:
    QObject(QObjectPrivate &d, QObject *parent);
private:
    QObject(QWidgetPrivate &d, QObject *parent);
    void findChildren_helper(const char *name, const QRegExp *re,
                             const QMetaObject &mo, QList<void*> *list) const;
    QObject *findChild_helper(const char *name, const QMetaObject &mo) const;
    virtual void setParent_helper(QObject *);
    uint isWidget : 1;
    uint pendTimer : 1;
    uint blockSig : 1;
    uint wasDeleted : 1;
    uint ownObjectName : 1;
    uint hasPostedEvents : 1;
#ifdef QT_COMPAT
    uint hasPostedChildInsertedEvents : 1;
    uint unused : 25;
#else
    uint unused : 26;
#endif

    QObject *parentObj;

protected:
    QObjectPrivate *d_ptr;
    Q_DECL_PRIVATE( QObject );

    static const QMetaObject staticQtMetaObject;

    friend struct QMetaObject;
    friend class QApplication;
    friend class QCoreApplication;
    friend class QWidget;

private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QObject(const QObject &);
    QObject &operator=(const QObject &);
#endif
};

inline bool QObject::isAncestorOf(const QObject *child) const
{
    while (child) {
	child = child->parentObj;
	if (child == this)
	    return true;
    }
    return false;
}


#ifndef QT_NO_USERDATA
class Q_CORE_EXPORT QObjectUserData {
public:
    virtual ~QObjectUserData();
};
#endif


template <typename T>
inline T qt_cast(const QObject *object)
{ return (T) ((T)0)->staticMetaObject.cast(object); }


#define Q_DECLARE_INTERFACE(IFace) \
inline IFace *qt_cast(const QObject *object, IFace * = 0) \
{ return (IFace *)(object ? object->qt_metacast(#IFace) : 0); }

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug, QObject *);
#endif

#endif // QOBJECT_H
