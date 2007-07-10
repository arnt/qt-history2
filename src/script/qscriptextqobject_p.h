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

#ifndef QT_NO_QOBJECT

#include "qscriptecmacore_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptclassdata_p.h"
#include "qscriptfunction_p.h"
#include "qscriptengine.h"
#include "qscriptmemberfwd_p.h"

#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QVarLengthArray>
#include <QtCore/QVector>

namespace QScript {

class ExtQObject: public Ecma::Core
{
public:
    ExtQObject(QScriptEnginePrivate *engine, QScriptClassInfo *classInfo);
    virtual ~ExtQObject();

    virtual void execute(QScriptContextPrivate *context);

    class Instance: public QScriptFunction {
    public:
        Instance() { }
        virtual ~Instance()
        {
            switch (ownership) {
            case QScriptEngine::QtOwnership:
                break;
            case QScriptEngine::ScriptOwnership:
                delete value;
                break;
            case QScriptEngine::AutoOwnership:
                if (value && !value->parent())
                    delete value;
                break;
            }
        }

        static Instance *get(const QScriptValueImpl &object, QScriptClassInfo *klass);

        virtual void execute(QScriptContextPrivate *context);

    public:
        QPointer<QObject> value;
        bool isConnection;
        QScriptEngine::ValueOwnership ownership;
        QScriptEngine::QObjectWrapOptions options;
    };

    inline Instance *get(const QScriptValueImpl &object) const
        { return Instance::get(object, classInfo()); }

    void newQObject(QScriptValueImpl *result, QObject *value,
                    QScriptEngine::ValueOwnership ownership = QScriptEngine::QtOwnership,
                    const QScriptEngine::QObjectWrapOptions &options = 0,
                    bool isConnection = false);

protected:
    static QScriptValueImpl method_findChild(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
    static QScriptValueImpl method_findChildren(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
};

class ConnectionQObject: public QObject
{
public:
    ConnectionQObject(const QMetaMethod &m,
                      QObject *sender,
                      const QScriptValueImpl &signal,
                      const QScriptValueImpl &receiver,
                      const QScriptValueImpl &slot,
                      QScriptEngine::ValueOwnership ownership);
    ~ConnectionQObject();

    static const QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **argv);

    void execute(void **argv);

    void mark(int generation);
    bool hasTarget(const QScriptValueImpl &, const QScriptValueImpl &) const;
    QObject *senderQObject() const;

    void senderDestroyed();

private:
    QMetaMethod m_method;
    QObject *m_sender;
    QScriptValueImpl m_signal;
    QScriptValueImpl m_receiver;
    QScriptValueImpl m_slot;
    QScriptValue m_self;
};

class QtFunction: public QScriptFunction
{
public:
    QtFunction(const QScriptValueImpl &object, int initialIndex, bool maybeOverloaded)
        : m_object(object), m_initialIndex(initialIndex),
          m_maybeOverloaded(maybeOverloaded)
        { }

    virtual ~QtFunction();

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::Qt; }

    virtual QString functionName() const;

    virtual void mark(QScriptEnginePrivate *engine, int generation);

    inline QScriptValueImpl object() const { return m_object; }

    inline QObject *qobject() const {
        if (!m_object.isQObject())
            return 0;
        return m_object.toQObject();
    }

    inline const QMetaObject *metaObject() const {
        if (!m_object.isQObject())
            return 0;
        QObject *qobj = m_object.toQObject();
        if (!qobj)
            return 0;
        return qobj->metaObject();
    }

    int mostGeneralMethod(QMetaMethod *out = 0) const;

    inline int initialIndex() const { return m_initialIndex; }
    inline bool maybeOverloaded() const { return m_maybeOverloaded; }

    bool createConnection(const QScriptValueImpl &self,
                          const QScriptValueImpl &receiver,
                          const QScriptValueImpl &slot);
    bool destroyConnection(const QScriptValueImpl &self,
                           const QScriptValueImpl &receiver,
                           const QScriptValueImpl &slot);
    void removeConnection(QObject *connection);

private:
    QScriptValueImpl m_object;
    QList<QPointer<QObject> > m_connections;
    int m_initialIndex;
    bool m_maybeOverloaded;
};

class ExtQMetaObject: public Ecma::Core
{
public:
    ExtQMetaObject(QScriptEnginePrivate *engine, QScriptClassInfo *classInfo);
    virtual ~ExtQMetaObject();

    virtual void execute(QScriptContextPrivate *context);

    class Instance: public QScriptFunction {
    public:
        Instance() { value = 0; ctor.invalidate(); }
        virtual ~Instance() { }

        static Instance *get(const QScriptValueImpl &object, QScriptClassInfo *klass);

        virtual void execute(QScriptContextPrivate *context);

    public:
        const QMetaObject *value;
        QScriptValueImpl ctor;
    };

    inline Instance *get(const QScriptValueImpl &object) const
        { return Instance::get(object, classInfo()); }

    void newQMetaObject(QScriptValueImpl *result, const QMetaObject *value,
                        const QScriptValueImpl &ctor = QScriptValueImpl());

protected:
    static QScriptValueImpl method_className(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
};

} // namespace QScript

class QScriptMetaType
{
public:
    enum Kind {
        Invalid,
        Variant,
        MetaType,
        Unresolved,
        MetaEnum
    };

    inline QScriptMetaType()
        : m_kind(Invalid) { }

    inline Kind kind() const
    { return m_kind; }

    inline int typeId() const
    { return isMetaEnum() ? 2/*int*/ : m_typeId; }

    inline bool isValid() const
    { return (m_kind != Invalid); }

    inline bool isVariant() const
    { return (m_kind == Variant); }

    inline bool isMetaType() const
    { return (m_kind == MetaType); }

    inline bool isUnresolved() const
    { return (m_kind == Unresolved); }

    inline bool isMetaEnum() const
    { return (m_kind == MetaEnum); }

    inline QByteArray name() const
    { return m_name; }

    inline int enumeratorIndex() const
    { Q_ASSERT(isMetaEnum()); return m_typeId; }

    static inline QScriptMetaType variant()
    { return QScriptMetaType(Variant); }

    static inline QScriptMetaType metaType(int typeId, const QByteArray &name)
    { return QScriptMetaType(MetaType, typeId, name); }

    static inline QScriptMetaType metaEnum(int enumIndex, const QByteArray &name)
    { return QScriptMetaType(MetaEnum, enumIndex, name); }

    static inline QScriptMetaType unresolved(const QByteArray &name)
    { return QScriptMetaType(Unresolved, /*typeId=*/0, name); }

private:
    inline QScriptMetaType(Kind kind, int typeId = 0, const QByteArray &name = QByteArray())
        : m_kind(kind), m_typeId(typeId), m_name(name) { }

    Kind m_kind;
    int m_typeId;
    QByteArray m_name;
};

class QScriptMetaMethod
{
public:
    inline QScriptMetaMethod()
        { }
    inline QScriptMetaMethod(const QByteArray &name, const QVector<QScriptMetaType> &types)
        : m_name(name), m_types(types), m_firstUnresolvedIndex(-1)
    {
        QVector<QScriptMetaType>::const_iterator it;
        for (it = m_types.constBegin(); it != m_types.constEnd(); ++it) {
            if ((*it).kind() == QScriptMetaType::Unresolved) {
                m_firstUnresolvedIndex = it - m_types.constBegin();
                break;
            }
        }
    }
    inline bool isValid() const
    { return !m_types.isEmpty(); }

    QByteArray name() const
    { return m_name; }

    inline QScriptMetaType returnType() const
    { return m_types.at(0); }

    inline int argumentCount() const
    { return m_types.count() - 1; }

    inline QScriptMetaType argumentType(int arg) const
    { return m_types.at(arg + 1); }

    inline bool fullyResolved() const
    { return m_firstUnresolvedIndex == -1; }

    inline int firstUnresolvedIndex() const
    { return m_firstUnresolvedIndex; }

    inline int count() const
    { return m_types.count(); }

    inline QScriptMetaType type(int index) const
    { return m_types.at(index); }

private:
    QByteArray m_name;
    QVector<QScriptMetaType> m_types;
    int m_firstUnresolvedIndex;
};

struct QScriptMetaArguments
{
    int matchDistance;
    int index;
    QScriptMetaMethod method;
    QVarLengthArray<QVariant, 9> args;

    inline QScriptMetaArguments(int dist, int idx, const QScriptMetaMethod &mtd,
                                const QVarLengthArray<QVariant, 9> &as)
        : matchDistance(dist), index(idx), method(mtd), args(as) { }
    inline QScriptMetaArguments()
        : index(-1) { }

    inline bool isValid() const
    { return (index != -1); }
};

class QScriptMetaObject
{
public:
    inline QScriptMetaMethod findMethod(int index) const
    {
        return m_methods.value(index);
    }

    inline void registerMethod(int index, const QScriptMetaMethod &method)
    {
        m_methods.insert(index, method);
    }

    inline bool findMember(QScriptNameIdImpl *nameId, QScript::Member *member) const
    {
        QHash<QScriptNameIdImpl*, QScript::Member>::const_iterator it;
        it = m_members.constFind(nameId);
        if (it == m_members.constEnd())
            return false;
        *member = it.value();
        return true;
    }

    inline void registerMember(QScriptNameIdImpl *nameId, const QScript::Member &member)
    {
        m_members.insert(nameId, member);
    }

    inline QList<QScriptNameIdImpl*> registeredMemberNames() const
    {
        return m_members.keys();
    }

    inline QScriptValueImpl findPropertyAccessor(int index) const
    {
        return m_propertyAccessors.value(index);
    }

    inline void registerPropertyAccessor(int index, const QScriptValueImpl &accessor)
    {
        m_propertyAccessors.insert(index, accessor);
    }

    inline QList<QScriptValueImpl> registeredPropertyAccessors() const
    {
        return m_propertyAccessors.values();
    }

    inline int methodLowerBound(int index) const
    {
        return m_methodBounds.value(index, 0);
    }

    inline void setMethodLowerBound(int index, int bound)
    {
        m_methodBounds.insert(index, bound);
    }

private:
    QHash<int, QScriptValueImpl> m_propertyAccessors;
    QHash<int, QScriptMetaMethod> m_methods;
    QHash<int, int> m_methodBounds;
    QHash<QScriptNameIdImpl*, QScript::Member> m_members;
};

#endif // QT_NO_QOBJECT

#endif // QT_NO_SCRIPT
#endif // QSCRIPTEXTQOBJECT_P_H
