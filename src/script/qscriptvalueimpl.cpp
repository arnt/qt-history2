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

#include "qscriptvalueimpl_p.h"
#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

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

bool QScriptValueImpl::detectedCycle() const
{
    QHash<QScriptObject*, int> dfn;
    dfs(m_object_value, dfn, 0);
    return checkCycle(m_object_value, dfn);
}

bool QScriptValueImpl::instanceOf(const QScriptValueImpl &ctorValue) const
{
    if (! isObject() || ! ctorValue.isObject())
        return false;
    
    QScriptObject *instance = m_object_value;
    QScriptObject *ctor = ctorValue.m_object_value;
    
    if (instance == ctor)
        return false;
    
    while (instance != 0) {
        if (instance == ctor)
            return true;
        
        const QScriptValueImpl &proto = instance->m_prototype;
        
        if (! proto.isValid() || ! proto.isObject())
            break;
        
        instance = proto.m_object_value;
    }
    
    return false;
}

bool QScriptValueImpl::resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                                      QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const
{
    QScriptObject *object_data = m_object_value;
    
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    
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
    
    if (mode & QScriptValue::ResolvePrototype) {
        // For values and other non object based types, search in class's prototype
        const QScriptValueImpl &proto = object_data->m_prototype;
        
        if (proto.isValid() && proto.isObject()
            && proto.resolve(nameId, member, object, mode)) {
            return true;
        }
    }
    
    if ((mode & QScriptValue::ResolveScope) && object_data->m_scope.isValid())
        return object_data->m_scope.resolve(nameId, member, object, mode);
    
    return false;
}

void QScriptValueImpl::setProperty(QScriptNameIdImpl *nameId,
                                   const QScriptValueImpl &value,
                                   const QScriptValue::PropertyFlags &flags)
{
    if (!isObject())
        return;

    QScriptValueImpl base;
    QScript::Member member;

    QScriptValue::ResolveFlags mode = QScriptValue::ResolveLocal;
    // if we are not setting a setter or getter, look in prototype too
    if (!(flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter)))
        mode |= QScriptValue::ResolvePrototype;

    if (resolve(nameId, &member, &base, mode)) {
        // we resolved an existing property with that name
        if (flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter)) {
            // setting the getter or setter of a property in this object
            if (member.isSetter()) {
                // the property we resolved is a setter
                if (!(flags & QScriptValue::PropertySetter)) {
                    // find the getter, if not, create one
                    if (!m_object_value->findGetter(&member))
                        createMember(nameId, &member, flags);
                }
            } else {
                // the property we resolved is a getter
                if (!(flags & QScriptValue::PropertyGetter)) {
                    // find the setter, if not, create one
                    if (!m_object_value->findSetter(&member))
                        createMember(nameId, &member, flags);
                }
            }
        } else {
            // setting the value
            if (member.isGetterOrSetter()) {
                // call the setter
                QScriptValueImpl setter;
                if (member.isObjectProperty() && !member.isSetter())
                    base.m_object_value->findSetter(&member);
                base.get(member, &setter);
                setter.call(*this, QScriptValueImplList() << value);
                return;
            } else {
                if (base.m_object_value != m_object_value) {
                    createMember(nameId, &member, flags);
                    base = *this;
                }
            }
        }
    } else {
        // did not find it, create
        createMember(nameId, &member, flags);
        base = *this;
    }

    base.put(member, value);
}

QDebug &operator<<(QDebug &d, const QScriptValueImpl &object)
{
    d.nospace() << "QScriptValue(";

    if (!object.isValid()) {
        d.nospace() << "Invalid)";
        return d;
    }

    switch (object.type()) {
    case QScript::BooleanType:
        d.nospace() << "bool=" << object.toBoolean();
        break;

    case QScript::IntegerType:
        d.nospace() << "int=" << object.toInt32();
        break;

    case QScript::NumberType:
        d.nospace() << "qsreal=" << object.toNumber();
        break;

    case QScript::StringType:
        d.nospace() << "string=" << object.toString();
        break;

    case QScript::FunctionType:
        d.nospace() << "function=" << object.toString();
        break;

    case QScript::VariantType:
        d.nospace() << "variant=" << object.toString();
        break;

    case QScript::ReferenceType:
        d.nospace() << "reference";
        break;

    default:
        if (object.isObject()) {
            d.nospace() << object.classInfo()->name() << ",{";
            QScriptObject *od = object.objectValue();
            for (int i=0; i<od->memberCount(); ++i) {
                if (i != 0)
                    d << ",";

                QScript::Member m;
                od->member(i, &m);

                if (m.isValid() && m.isObjectProperty()) {
                    d << QScriptEnginePrivate::get(object.engine())->toString(m.nameId());
                    QScriptValueImpl o;
                    od->get(m, &o);
                    d.nospace() << QLatin1String(":")
                                << (o.classInfo()
                                    ? o.classInfo()->name()
                                    : QLatin1String("?"));
                }
            }

            d.nospace() << "} scope={";
            QScriptValueImpl scope = object.scope();
            while (scope.isValid()) {
                Q_ASSERT(scope.isObject());
                d.nospace() << " " << scope.objectValue();
                scope = scope.scope();
            }
            d.nospace() << "}";
        } else {
            d << "n/a";
        }
        break;
    }

    d << ")";
    return d;
}
