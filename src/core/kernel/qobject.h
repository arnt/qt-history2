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

#ifndef QOBJECT_H
#define QOBJECT_H

#ifndef QT_NO_QOBJECT

#include "QtCore/qobjectdefs.h"
#include "QtCore/qnamespace.h"
#include "QtCore/qstring.h"
#include "QtCore/qbytearray.h"
#include "QtCore/qlist.h"

#ifdef QT_INCLUDE_COMPAT
#include "QtCore/qcoreevent.h"
#endif

class QEvent;
class QTimerEvent;
class QChildEvent;
struct QMetaObject;
class QCoreVariant;
class QObjectPrivate;
class QObject;
class QThread;
class QWidget;
#ifndef QT_NO_REGEXP
class QRegExp;
#endif
#ifndef QT_NO_USERDATA
class QObjectUserData;
#endif

typedef QList<QObject*> QObjectList;

class QObjectData {
public:
    virtual ~QObjectData() = 0;
    QObject *q_ptr;
    QObject *parent;
    QObjectList children;

    uint isWidget : 1;
    uint pendTimer : 1;
    uint blockSig : 1;
    uint wasDeleted : 1;
    uint ownObjectName : 1;
    uint unused : 27;
    int postedEvents;
#ifdef QT_COMPAT
    int postedChildInsertedEvents;
#else
    int reserved;
#endif
};


class Q_CORE_EXPORT QObject
{
    Q_OBJECT
    Q_PROPERTY(QString objectName READ objectName WRITE setObjectName)
    Q_DECLARE_PRIVATE(QObject)

public:
    explicit QObject(QObject *parent=0);
    virtual ~QObject();

    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

#ifdef qdoc
    static QString tr(const char *, const char *);
    static QString trUtf8(const char *, const char *);
    virtual const QMetaObject *metaObject() const;
#endif
#ifdef QT_NO_TRANSLATION
    static QString tr(const char *sourceText, const char * = 0)
        { return QString::fromLatin1(sourceText); }
#ifndef QT_NO_TEXTCODEC
    static QString trUtf8(const char *sourceText, const char * = 0)
        { return QString::fromUtf8(sourceText); }
#endif
#endif //QT_NO_TRANSLATION

    QString objectName() const;
    void setObjectName(const QString &name);

    inline bool isWidgetType() const { return d_ptr->isWidget; }

    inline bool signalsBlocked() const { return d_ptr->blockSig; }
    bool blockSignals(bool b);

    QThread *thread() const;

    int startTimer(int interval);
    void killTimer(int id);

    QObject *findChild(const QString &name) const;
    QObjectList findChildren(const QString &name) const;

#ifndef QT_NO_REGEXP
    QObjectList findChildren(const QRegExp &re) const;
#endif

#ifdef QT_COMPAT
    QT_COMPAT QObject *child(const char *objName, const char *inheritsClass = 0,
                   bool recursiveSearch = true) const;
    QT_COMPAT QObjectList queryList(const char *inheritsClass = 0,
                          const char *objName = 0,
                          bool regexpMatch = true,
                          bool recursiveSearch = true) const;
#endif
    const QObjectList &children() const { return d_ptr->children; }

    void setParent(QObject *);
    void installEventFilter(const QObject *);
    void removeEventFilter(const QObject *);


    static bool connect(const QObject *sender, const char *signal,
                        const QObject *receiver, const char *member,
                        Qt::ConnectionType = Qt::AutoConnection);
    inline bool connect(const QObject *sender, const char *signal,
                        const char *member, Qt::ConnectionType type = Qt::AutoConnection) const
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

signals:
    void destroyed(QObject * = 0);

public:
    inline QObject *parent() const { return d_ptr->parent; }

    inline bool inherits(const char *classname) const
        { return const_cast<QObject *>(this)->qt_metacast(classname) != 0; }

public slots:
    void deleteLater();

protected:
    QObject *sender() const;
    int receivers(const char* signal) const;

    virtual void timerEvent(QTimerEvent *);
    virtual void childEvent(QChildEvent *);
    virtual void customEvent(QEvent *);

    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QObject(QObject *parent, const char *name);
    inline QT_COMPAT void insertChild(QObject *o)
        { if (o) o->setParent(this); }
    inline QT_COMPAT void removeChild(QObject *o)
        { if (o) o->setParent(0); }
    inline QT_COMPAT bool isA(const char *classname) const
        { return qstrcmp(classname, metaObject()->className()) == 0; }
    inline QT_COMPAT const char *className() const { return metaObject()->className(); }
    inline QT_COMPAT const char *name() const { return objectName().latin1_helper(); }
    inline QT_COMPAT const char *name(const char *defaultName) const
        { QString s = objectName(); return s.isEmpty()?defaultName:s.latin1_helper(); }
    inline QT_COMPAT void setName(const char *name) { setObjectName(QLatin1String(name)); }
protected:
    inline QT_COMPAT bool checkConnectArgs(const char *signal,
                                  const QObject *,
                                  const char *member)
        { return QMetaObject::checkConnectArgs(signal, member); }
    static inline QT_COMPAT QByteArray normalizeSignalSlot(const char *signalSlot)
        { return QMetaObject::normalizedSignature(signalSlot); }
#endif

protected:
    explicit QObject(QObjectPrivate &d, QObject *parent);

protected:
    QObjectData *d_ptr;

    static const QMetaObject staticQtMetaObject;

    friend struct QMetaObject;
    friend class QApplication;
    friend class QCoreApplication;
    friend class QCoreApplicationPrivate;
    friend class QWidget;

private:
    Q_DISABLE_COPY(QObject)
};

inline bool QObject::isAncestorOf(const QObject *child) const
{
    while (child) {
        if (child == this)
            return true;
        child = child->d_ptr->parent;
    }
    return false;
}


#ifndef QT_NO_USERDATA
class Q_CORE_EXPORT QObjectUserData {
public:
    virtual ~QObjectUserData();
};
#endif


Q_CORE_EXPORT void qt_qFindChildren_helper(const QObject *parent, const QString &name, const QRegExp *re,
                         const QMetaObject &mo, QList<void*> *list);
Q_CORE_EXPORT QObject *qt_qFindChild_helper(const QObject *parent, const QString &name, const QMetaObject &mo);


#if defined Q_CC_MSVC && _MSC_VER < 1300

template<typename T>
inline T qFindChild(const QObject *o, const QString &name = QString(), T = 0)
{ return static_cast<T>(qt_qFindChild_helper(o, name, ((T)0)->staticMetaObject)); }

template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QString &name = QString(), T = 0)
{
    QList<T> list;
    qt_qFindChildren_helper(o, name, 0, ((T)0)->staticMetaObject,
                        reinterpret_cast<QList<void *>*>(&list));
    return list;
}

#ifndef QT_NO_REGEXP
template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QRegExp &re, T = 0)
{
    QList<T> list;
    qt_qFindChildren_helper(o, 0, &re, ((T)0)->staticMetaObject,
                        reinterpret_cast<QList<void*>*>(&list));
    return list;
}

#endif

template <class T> inline T qt_cast_helper(QObject *object, T)
{ return static_cast<T>(((T)0)->staticMetaObject.cast(object)); }

template <class T> inline T qt_cast_helper(const QObject *object, T)
{ return static_cast<T>(const_cast<const QObject *>(((T)0)->staticMetaObject.cast(const_cast<QObject *>(object)))); }

template <class T>
inline T qt_cast(QObject *object)
{ return qt_cast_helper<T>(object, T(0)); }

template <class T>
inline T qt_cast(const QObject *object)
{ return qt_cast_helper<T>(object, T(0)); }

#define Q_DECLARE_INTERFACE(IFace, IId) \
const char * const IFace##_iid = IId; \
template <> inline IFace *qt_cast_helper<IFace *>(QObject *object, IFace *) \
{ return (IFace *)(object ? object->qt_metacast(IFace##_iid) : 0); } \
template <> inline IFace *qt_cast_helper<IFace *>(const QObject *object, IFace *) \
{ return (IFace *)(object ? const_cast<QObject *>(object)->qt_metacast(IFace##_iid) : 0); }

#else

template<typename T>
inline T qFindChild(const QObject *o, const QString &name = QString())
{ return static_cast<T>(qt_qFindChild_helper(o, name, reinterpret_cast<T>(0)->staticMetaObject)); }

template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QString &name = QString())
{
    QList<T> list;
    qt_qFindChildren_helper(o, name, 0, reinterpret_cast<T>(0)->staticMetaObject,
                         reinterpret_cast<QList<void *>*>(&list));
    return list;
}

#ifndef QT_NO_REGEXP
template<typename T>
inline QList<T> qFindChildren(const QObject *o, const QRegExp &re)
{
    QList<T> list;
    qt_qFindChildren_helper(o, 0, &re, reinterpret_cast<T>(0)->staticMetaObject,
                        reinterpret_cast<QList<void*>*>(&list));
    return list;
}
#endif

#if !defined(Q_OS_WIN) && !defined(QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION)

template <class T> inline T *qt_cast_helper(QObject *object, T *)
{
    extern void qt_cast_to_class_without_Q_OBJECT(T *);
    qt_cast_to_class_without_Q_OBJECT(reinterpret_cast<T *>(0));
    return static_cast<T *>(T::staticMetaObject.cast(object));
}

template <class T> inline const T *qt_cast_helper(QObject *object, const T *)
{
    extern void qt_cast_to_class_without_Q_OBJECT(T *);
    qt_cast_to_class_without_Q_OBJECT(reinterpret_cast<T *>(0));
    return static_cast<const T *>(const_cast<const QObject *>(T::staticMetaObject.cast(object)));
}

template <class T> inline T *qt_cast_helper(const QObject *object, T *)
{
    extern void qt_cast_to_class_without_Q_OBJECT(T *);
    qt_cast_to_class_without_Q_OBJECT(reinterpret_cast<T *>(0));
    return static_cast<T *>(const_cast<const QObject *>( (T::staticMetaObject.cast(const_cast<QObject *>(object)))));
}

template <class T> inline const T *qt_cast_helper(const QObject *object, const T *)
{
    extern void qt_cast_to_class_without_Q_OBJECT(T *);
    qt_cast_to_class_without_Q_OBJECT(reinterpret_cast<T *>(0));
    return static_cast<const T *>(const_cast<const QObject *>( (T::staticMetaObject.cast(const_cast<QObject *>(object)))));
}

template <class T>
inline T qt_cast(QObject *object)
{
    return qt_cast_helper(object, T(0));
}

template <class T>
inline T qt_cast(const QObject *object)
{
    return qt_cast_helper(object, T(0));
}

#define Q_DECLARE_INTERFACE(IFace, IId) \
const char * const IFace##_iid = IId; \
template <> inline IFace *qt_cast_helper<IFace>(QObject *object, IFace *) \
{ return (IFace *)(object ? object->qt_metacast(IFace##_iid) : 0); } \
template <> inline const IFace *qt_cast_helper<IFace>(const QObject *object, const IFace *) \
{ return (IFace *)(object ? const_cast<QObject *>(object)->qt_metacast(IFace##_iid) : 0); }

#else

template <class T>
inline T qt_cast(QObject *object)
{ return static_cast<T>(reinterpret_cast<T>(0)->staticMetaObject.cast(object)); }

template <class T>
inline T qt_cast(const QObject *object)
{ return static_cast<T>(const_cast<const QObject *>(reinterpret_cast<T>(0)->staticMetaObject.cast(const_cast<QObject *>(object)))); }


#define Q_DECLARE_INTERFACE(IFace, IId) \
const char * const IFace##_iid = IId; \
template <> inline IFace *qt_cast<IFace *>(QObject *object) \
{ return reinterpret_cast<IFace *>((object ? object->qt_metacast(IFace##_iid) : 0)); } \
template <> inline IFace *qt_cast<IFace *>(const QObject *object) \
{ return reinterpret_cast<IFace *>((object ? const_cast<QObject *>(object)->qt_metacast(IFace##_iid) : 0)); }

#endif

#endif

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QObject *);
#endif

#endif

#endif // QOBJECT_H
