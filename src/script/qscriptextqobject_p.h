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

#ifndef QSCRIPTEXTQOBJECT_P_H
#define QSCRIPTEXTQOBJECT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QPointer>

#include "qscriptecmacore_p.h"
#include "qscriptclassdata_p.h"
#include "qscriptfunction_p.h"

#ifndef QT_NO_QOBJECT

#include <QtCore/QObject>

namespace QScript {

class ExtQObject: public Ecma::Core
{
public:
    ExtQObject(QScriptEnginePrivate *engine, QScriptClassInfo *classInfo);
    virtual ~ExtQObject();

    inline QScriptClassInfo *classInfo() const { return m_classInfo; }

    virtual void execute(QScriptContextPrivate *context);

    class Instance: public QScriptFunction {
    public:
        Instance() { thisObject.invalidate(); }
        virtual ~Instance() { }

        static Instance *get(const QScriptValueImpl &object, QScriptClassInfo *klass);

        virtual void execute(QScriptContextPrivate *context);

    public:
        QScriptValueImpl thisObject;
        QPointer<QObject> value;
        bool isConnection;
    };

    inline Instance *get(const QScriptValueImpl &object) const
        { return Instance::get(object, classInfo()); }

    void newQObject(QScriptValueImpl *result, QObject *value, bool isConnection = false);

protected:
    static QScriptValueImpl method_findChild(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
    static QScriptValueImpl method_findChildren(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);

    QScriptClassInfo *m_classInfo;
};

class ConnectionQObject: public QObject
{
public:
    ConnectionQObject(const QMetaMethod &m, const QScriptValueImpl &sender,
                      const QScriptValueImpl &receiver, const QScriptValueImpl &slot);
    ~ConnectionQObject();

    static const QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **argv);

    void execute(void **argv);

    void mark(int generation);
    bool hasTarget(const QScriptValueImpl &, const QScriptValueImpl &) const;

private:
    QMetaMethod m_method;
    QScriptValue m_self;
    QScriptValueImpl m_sender;
    QScriptValueImpl m_receiver;
    QScriptValueImpl m_slot;
    bool m_hasReceiver;
};

class QtFunction: public QScriptFunction
{
public:
    QtFunction(QObject *object, int initialIndex, bool maybeOverloaded)
        : m_object(object), m_initialIndex(initialIndex),
          m_maybeOverloaded(maybeOverloaded)
        { }

    virtual ~QtFunction();

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::Qt; }

    inline QObject *object() const { return m_object; }
    inline const QMetaObject *metaObject() const { return m_object->metaObject(); }
    inline int initialIndex() const { return m_initialIndex; }
    inline bool maybeOverloaded() const { return m_maybeOverloaded; }
    bool createConnection(const QScriptValueImpl &self,
                          const QScriptValueImpl &receiver,
                          const QScriptValueImpl &slot);
    bool destroyConnection(const QScriptValueImpl &self,
                           const QScriptValueImpl &receiver,
                           const QScriptValueImpl &slot);

private:
    QPointer<QObject> m_object;
    QList<QPointer<QObject> > m_connections;
    int m_initialIndex;
    bool m_maybeOverloaded;
};

class QtPropertyFunction: public QScriptFunction
{
public:
    QtPropertyFunction(QObject *object, int index)
        : m_object(object), m_index(index)
        { }

    ~QtPropertyFunction() { }

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::QtProperty; }

private:
    QObject *m_object;
    int m_index;
};

class ExtQMetaObjectData: public QScriptClassData
{
public:
    virtual bool resolve(const QScriptValueImpl &object, QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValueImpl *base);
    virtual bool get(const QScriptValueImpl &obj, const QScript::Member &member,
                     QScriptValueImpl *result);
    virtual void mark(const QScriptValueImpl &object, int generation);
};

class ExtQMetaObject: public QScriptFunction
{
public:
    ExtQMetaObject(const QMetaObject *meta, const QScriptValueImpl &ctor);

    virtual void execute(QScriptContextPrivate *context);

    const QMetaObject *m_meta;
    QScriptValueImpl m_ctor;
};

} // namespace QScript

#endif // QT_NO_QOBJECT

#endif // QSCRIPTEXTQOBJECT_P_H
