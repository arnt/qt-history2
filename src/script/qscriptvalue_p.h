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

#ifndef QSCRIPTVALUE_P_H
#define QSCRIPTVALUE_P_H

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

#include "qscriptvalue.h"
#include "qscriptengine.h"
#include "qscriptobject_p.h"
#include "qscriptengine_p.h"

class QScriptValueImpl : public QScriptValue
{
public:
    static inline QScriptValueImpl *get(const QScriptValue &value)
        {  return const_cast<QScriptValueImpl*>(reinterpret_cast<const QScriptValueImpl*>(&value)); }

    inline QScriptEngine *engine() const
    {
        if (! m_class)
            return 0;

        return m_class->engine();
    }

    inline QScriptEnginePrivate *enginePrivate() const
        { return QScriptEnginePrivate::get(engine()); }

    inline void createMember(QScriptNameIdImpl *nameId,
                             QScript::Member *member, uint flags) // ### remove me
    {
        Q_ASSERT(isObject());

        QScriptObject *object_data = m_object_value;
        object_data->createMember(nameId, member, flags);
        Q_ASSERT(member->isObjectProperty());
    }

    quint32 length() const;

    QVariant variantValue() const;

    void setQObjectValue(QObject *object);

    QList<QScriptNameIdImpl*> propertyIds() const;

    inline QScriptClassInfo *classInfo() const
    {
        return m_class;
    }

    inline QScript::Type type() const
    {
        Q_ASSERT(isValid());

        return m_class->type();
    }

    inline QExplicitlySharedDataPointer<QScriptObjectData> objectData() const
    {
        Q_ASSERT(isObject());
        return m_object_value->m_data;
    }

    inline void setObjectData(QExplicitlySharedDataPointer<QScriptObjectData> data)
    {
        Q_ASSERT(isObject());
        m_object_value->m_data = data;
    }

    inline int memberCount() const
    {
        Q_ASSERT(isObject());

        int count = m_object_value->memberCount();

        if (m_class->data())
            count += m_class->data()->extraMemberCount(*this);

        return count;
    }

    inline void member(int index, QScript::Member *member) const
    {
        Q_ASSERT(isObject());

        if (QScriptClassData *data = m_class->data()) {
            int extra = data->extraMemberCount(*this);
            if (index < extra) {
                data->extraMember(*this, index, member);
                return;
            }
            index -= extra;
        }

        m_object_value->member(index, member);
    }

    void removeMember(const QScript::Member &member);

    inline bool isReference() const
    {
        Q_ASSERT(isValid());
        return m_class->type() == QScript::ReferenceType;
    }

    inline void setClassInfo(QScriptClassInfo *cls)
    {
        m_class = cls;
    }

    inline QScriptValue scope() const
    {
        Q_ASSERT(isObject());
        return m_object_value->m_scope;
    }

    inline void setScope(const QScriptValue &scope)
    {
        Q_ASSERT(isObject());
        m_object_value->m_scope = scope;
    }

    QScriptFunction *toFunction() const;

    inline QScriptValue internalValue() const
    {
        Q_ASSERT(isObject());
        return m_object_value->m_internalValue;
    }

    inline void setInternalValue(const QScriptValue &internalValue)
    {
        Q_ASSERT(isObject());
        m_object_value->m_internalValue = internalValue;
    }

    inline QScriptNameIdImpl *stringValue()
    {
        Q_ASSERT(isString());
        return m_string_value;
    }

    inline QScriptObject *objectValue()
    {
        Q_ASSERT(isObject());
        return m_object_value;
    }

    inline void setResolveMode(int mode)
    {
        Q_ASSERT(isReference());
        m_int_value = mode;
    }

    inline int resolveMode() const
    {
        Q_ASSERT(isReference());
        return m_int_value;
    }

    inline void incr() {
        ++m_number_value;
    }

    inline void decr() {
        --m_number_value;
    }

    inline void get(const QScript::Member &member, QScriptValue *obj) const
    {
        Q_ASSERT(obj);
        Q_ASSERT(isObject());
        Q_ASSERT(member.isValid());

        if (! member.isObjectProperty()) {
            get_helper(member, obj);
            return;
        }

        Q_ASSERT(member.id() >= 0);
        Q_ASSERT(member.id() < m_object_value->memberCount());
        Q_ASSERT(member.nameId());
        Q_ASSERT(member.nameId()->unique);

        m_object_value->get(member, obj);
    }

    inline void get(QScriptNameIdImpl *nameId, QScriptValue *out)
    {
        QScript::Member m;
        QScriptValue o;
        if (resolve(nameId, &m, &o, ResolvePrototype))
            o.impl()->get(m, out);
        else
            enginePrivate()->newUndefined(out);
    }

    void get_helper(const QScript::Member &member, QScriptValue *obj) const
    {
        QScriptEnginePrivate *eng = enginePrivate();

        if (member.nameId() == eng->idTable()->id___proto__) {
            *obj = prototype();

            if (!obj->isValid())
                eng->newUndefined(obj);

            return;
        }

        if (QScriptClassData *data = classInfo()->data()) {
            if (data->get(*this, member, obj))
                return;
        }

        obj->invalidate();

        if (! isFunction()) {
            return;
        } else if (member.nameId() == eng->idTable()->id_length) {
            QScriptFunction *foo = eng->convertToNativeFunction(*this);
            Q_ASSERT(foo != 0);
            eng->newNumber(obj, foo->length);
        } else if (member.nameId() == eng->idTable()->id_arguments) {
            eng->newNull(obj);
        }
    }

    void put(const QScript::Member &member, const QScriptValue &object)
    {
        Q_ASSERT(isObject());
        Q_ASSERT(member.isValid());
        // Q_ASSERT(member.isWritable());

        QScriptEnginePrivate *eng_p = enginePrivate();

        if (member.isObjectProperty()) {
            Q_ASSERT(member.nameId()->unique);
            Q_ASSERT(member.id() >= 0);
            Q_ASSERT(member.id() < m_object_value->memberCount());
            m_object_value->put(member, object);
        }

        else if (member.nameId() == eng_p->idTable()->id___proto__) {
            if (object.isNull()) // only Object.prototype.__proto__ can be null
                setPrototype(engine()->undefinedValue());
            else {
                QScriptValue was = prototype();
                setPrototype(object);
                if (detectedCycle()) {
                    qWarning() << "*** cycle detected"; // ### throw an exception
                    setPrototype(was);
                }
            }
        }

        else {
            Q_ASSERT(classInfo()->data());
            classInfo()->data()->put(this, member, object);
        }
    }

    inline bool resolve(QScriptNameIdImpl *nameId, QScript::Member *member,
                        QScriptValue *object, ResolveFlags mode)
    {
        Q_ASSERT(isValid());
        Q_ASSERT(isObject());
        Q_ASSERT(member);
        Q_ASSERT(object);

        Q_ASSERT(nameId->unique);

        QScriptObject *object_data = m_object_value;

        // Search in properties...
        if (object_data->findMember(nameId, member)) {
            *object = *this;
            return true;
        }

        return resolve_helper(nameId, member, object, mode);
    }

    bool resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                        QScriptValue *object, ResolveFlags mode)
    {
        QScriptObject *object_data = m_object_value;

        QScriptEnginePrivate *eng_p = enginePrivate();

        if (nameId == eng_p->idTable()->id___proto__) {
            member->native(nameId, /*id=*/0, QScriptValue::Undeletable);
            *object = *this;
            return true;
        }

        // If not found anywhere else, search in the extra members.
        if (QScriptClassData *odata = classInfo()->data()) {
            *object = *this;

            if (odata->resolve(*this, nameId, member, object))
                return true;
        }

        if (isFunction()) {
            if (nameId == eng_p->idTable()->id_length) {
                member->native(nameId, 0,
                               QScriptValue::Undeletable
                               | QScriptValue::ReadOnly
                               | QScriptValue::SkipInEnumeration);
                *object = *this;
                return true;
            } else if (nameId == eng_p->idTable()->id_arguments) {
                member->native(nameId, 0,
                               QScriptValue::Undeletable
                               | QScriptValue::ReadOnly
                               | QScriptValue::SkipInEnumeration);
                *object = *this;
                return true;
            }
        }

        if (mode & ResolvePrototype) {
            // For values and other non object based types, search in class's prototype
            const QScriptValue &proto = object_data->m_prototype;

            if (proto.isValid() && proto.isObject()
                && proto.impl()->resolve(nameId, member, object, mode)) {
                return true;
            }
        }

        if ((mode & ResolveScope) && object_data->m_scope.isValid())
            return object_data->m_scope.impl()->resolve(nameId, member, object, mode);

        return false;
    }

    inline bool detectedCycle()
    {
        QHash<QScriptObject*, int> dfn;
        dfs(m_object_value, dfn, 0);
        return checkCycle(m_object_value, dfn);
    }

    static void dfs(QScriptObject *instance, QHash<QScriptObject*, int> &dfn, int n)
    {
        if (dfn.contains(instance))
            return; // nothing to do

        dfn[instance] = n;

        if (instance->m_prototype.isValid() && instance->m_prototype.isObject())
            dfs (instance->m_prototype.m_object_value, dfn, ++n);

        if (instance->m_scope.isValid() && instance->m_scope.isObject())
            dfs (instance->m_scope.m_object_value, dfn, ++n);
    }

    static bool checkCycle(QScriptObject *instance, const QHash<QScriptObject*, int> &dfn)
    {
        int n = dfn.value(instance);

        if (instance->m_prototype.isValid() && instance->m_prototype.isObject()) {
            if (n >= dfn.value(instance->m_prototype.m_object_value))
                return true;
        }

        if (instance->m_scope.isValid() && instance->m_scope.isObject()) {
            if (n >= dfn.value(instance->m_scope.m_object_value))
                return true;
        }

        return false;
    }

};

QDebug &operator<<(QDebug &d, const QScriptValue &object);

#endif
