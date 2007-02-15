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

#include "qscriptextqobject_p.h"
#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"
#include "qscriptable.h"
#include "qscriptable_p.h"

#include <QtCore/QBitArray>
#include <QtCore/QtDebug>
#include <QtCore/QMetaMethod>
#include <QtCore/QRegExp>
#include "qscriptextqobject_p.h"

// we use bits 15..12 of property flags
enum {
    PROPERTY_ID      = 0 << 12,
    DYNAPROPERTY_ID  = 1 << 12,
    METHOD_ID        = 2 << 12,
    CHILD_ID         = 3 << 12,
    ID_MASK          = 7 << 12,
    MAYBE_OVERLOADED = 8 << 12
};

static const bool GeneratePropertyFunctions = true;

namespace QScript {

static inline QByteArray methodName(const QMetaMethod &method)
{
    QByteArray signature = method.signature();
    return signature.left(signature.indexOf('('));
}

static inline QVariant variantFromValue(int targetType, const QScriptValueImpl &value)
{
    QVariant v(targetType, (void *)0);
    QScriptEngine *eng = value.engine();
    Q_ASSERT(eng);
    if (QScriptEnginePrivate::get(eng)->convert(value, targetType, v.data()))
        return v;
    if (uint(targetType) == QVariant::LastType)
        return value.toVariant();
    if (value.isVariant()) {
        v = value.toVariant();
        if (v.canConvert(QVariant::Type(targetType))) {
            v.convert(QVariant::Type(targetType));
            return v;
        }
    }

    return QVariant();
}

ExtQObject::Instance *ExtQObject::Instance::get(const QScriptValueImpl &object, QScriptClassInfo *klass)
{
    if (! klass || klass == object.classInfo())
        return static_cast<Instance*> (object.objectData().data());

    return 0;
}


void ExtQObject::Instance::execute(QScriptContextPrivate *context)
{
    const QMetaObject *meta = value->metaObject();
    // look for qscript_call()
    QByteArray qscript_call = QByteArray("qscript_call");
    int index;
    for (index = meta->methodCount() - 1; index >= 0; --index) {
        if (methodName(meta->method(index)) == qscript_call)
            break;
    }
    if (index < 0) {
        context->throwError(QScriptContext::TypeError,
                            QLatin1String("not a function"));
        return;
    }

    QtFunction fun(value, index, /*maybeOverloaded=*/true);
    fun.execute(context);
}

static inline QScriptable *scriptableFromQObject(QObject *qobj)
{
    void *ptr = qobj->qt_metacast("QScriptable");
    return reinterpret_cast<QScriptable*>(ptr);
}

class ExtQObjectData: public QScriptClassData
{
public:
    ExtQObjectData(QScriptEnginePrivate *, QScriptClassInfo *classInfo)
        : m_classInfo(classInfo)
    {
    }

    virtual bool resolve(const QScriptValueImpl &object, QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValueImpl *)
    {
        QScriptEnginePrivate *eng = QScriptEnginePrivate::get(object.engine());

        QObject *qobject = object.toQObject();
        if (! qobject)
            return false;

        const QMetaObject *meta = qobject->metaObject();
        QString memberName = eng->toString(nameId);
        QByteArray name = memberName.toLatin1();

        int index = -1;

        if (name.contains('(')) {
            QByteArray normalized = QMetaObject::normalizedSignature(name);

            if (-1 != (index = meta->indexOfMethod(normalized))) {
                member->native(nameId, index,
                               QScriptValue::Undeletable
                               | QScriptValue::SkipInEnumeration
                               | QScriptValue::ReadOnly
                               | METHOD_ID);
                return true;
            }
        }

        index = meta->indexOfProperty(name);
        if (index != -1) {
            QMetaProperty prop = meta->property(index);
            if (prop.isScriptable()) {
                member->native(nameId, index,
                               QScriptValue::Undeletable
                               | QScriptValue::SkipInEnumeration
                               | (GeneratePropertyFunctions
                                  ? (QScriptValue::PropertyGetter
                                     | QScriptValue::PropertySetter)
                                  : QScriptValue::PropertyFlag(0))
                               | PROPERTY_ID);
                return true;
            }
        }

        index = qobject->dynamicPropertyNames().indexOf(name);
        if (index != -1) {
            member->native(nameId, index,
                           QScriptValue::SkipInEnumeration
                           | DYNAPROPERTY_ID);
            return true;
        }

        for (index = meta->methodCount() - 1; index >= 0; --index) {
            QMetaMethod method = meta->method(index);
            if (methodName(method) == name) {
                member->native(nameId, index,
                               QScriptValue::SkipInEnumeration
                               | METHOD_ID
                               | MAYBE_OVERLOADED);
                return true;
            }
        }

        // maybe a child ?
        QList<QObject*> children = qobject->children();
        for (index = 0; index < children.count(); ++index) {
            QObject *child = children.at(index);

            if (child->objectName() == memberName) {
                member->native(nameId, index, QScriptValue::ReadOnly | CHILD_ID); // ### flags
                return true;
            }
        }

        return false;
     }

    virtual bool get(const QScriptValueImpl &obj, const QScript::Member &member, QScriptValueImpl *result)
    {
        if (! member.isNativeProperty())
            return false;

        QScriptEnginePrivate *eng = QScriptEnginePrivate::get(obj.engine());

        ExtQObject::Instance *inst = ExtQObject::Instance::get(obj, m_classInfo);
        QObject *qobject = inst->value;

        switch (member.flags() & ID_MASK) {
        case PROPERTY_ID: {
            if (GeneratePropertyFunctions) {
                const int propertyIndex = member.id();
                *result = eng->createFunction(new QtPropertyFunction(qobject, propertyIndex));

                // make it persist
                QScript::Member m;
                QScriptObject *instance = obj.objectValue();
                if (!instance->findMember(member.nameId(), &m)) {
                    instance->createMember(member.nameId(), &m,
                                           QScriptValue::Undeletable
                                           | QScriptValue::SkipInEnumeration
                                           | QScriptValue::PropertyGetter
                                           | QScriptValue::PropertySetter);
                }
                instance->put(m, *result);
            } else {
                const QMetaObject *meta = qobject->metaObject();
                QMetaProperty prop = meta->property(member.id());
                Q_ASSERT(prop.isScriptable());
                QVariant v = prop.read(qobject);
                *result = eng->valueFromVariant(v);
            }
        }   break;

        case DYNAPROPERTY_ID: {
            QByteArray name = qobject->dynamicPropertyNames().value(member.id());
            QVariant v = qobject->property(name);
            *result = eng->valueFromVariant(v);
        }   break;

        case METHOD_ID: {
            QScript::Member m;
            bool maybeOverloaded = (member.flags() & MAYBE_OVERLOADED) != 0;
            *result = eng->createFunction(new QtFunction(qobject, member.id(),
                                                           maybeOverloaded));
            // make it persist
            QScriptObject *instance = obj.objectValue();
            if (!instance->findMember(member.nameId(), &m)) {
                instance->createMember(member.nameId(), &m,
                                       QScriptValue::SkipInEnumeration);
            }
            instance->put(m, *result);
        }   break;

        case CHILD_ID: {
            QObject *child = qobject->children().at(member.id());
            *result = eng->newQObject(child);
        }   break;

        } // switch

        return true;
    }

    virtual bool put(QScriptValueImpl *object, const QScript::Member &member, const QScriptValueImpl &value)
    {
        if (! member.isNativeProperty() || ! member.isWritable())
            return false;

        ExtQObject::Instance *inst = ExtQObject::Instance::get(*object, m_classInfo);
        QObject *qobject = inst->value;

        switch (member.flags() & ID_MASK) {
        case CHILD_ID:
            return false;

        case METHOD_ID: {
            QScript::Member m;
            QScriptObject *instance = object->objectValue();
            if (!instance->findMember(member.nameId(), &m)) {
                instance->createMember(member.nameId(), &m,
                                       QScriptValue::SkipInEnumeration);
            }
            instance->put(m, value);
            return true;
        }

        case PROPERTY_ID:
            if (GeneratePropertyFunctions) {
                Q_ASSERT(0);
                return false;
            } else {
                const QMetaObject *meta = qobject->metaObject();
                QMetaProperty prop = meta->property(member.id());
                Q_ASSERT(prop.isScriptable());
                QVariant v = variantFromValue(prop.userType(), value);
                bool ok = prop.write(qobject, v);
                return ok;
            }

        case DYNAPROPERTY_ID: {
            QByteArray name = qobject->dynamicPropertyNames().value(member.id());
            QVariant v = value.toVariant();
            return ! qobject->setProperty(name, v);
        }

        } // switch
        return false;
    }

    virtual int extraMemberCount(const QScriptValueImpl &object)
    {
        QObject *qobject = object.toQObject();
        if (! qobject)
            return 0;
        const QMetaObject *meta = qobject->metaObject();

        return qobject->children().count()
            + meta->propertyCount()
            + meta->methodCount();
    }

    virtual bool extraMember(const QScriptValueImpl &object, int index, QScript::Member *member)
    {
        QScriptEngine *eng = object.engine();
        QObject *qobject = object.toQObject();
        const QMetaObject *meta = qobject->metaObject();

        const QObjectList children = qobject->children();
        if (index < children.count()) {
            QScriptNameIdImpl *nameId = eng->nameId(children.at(index)->objectName());
            member->native(nameId, index, QScriptValue::ReadOnly | CHILD_ID);
            return true;
        }

        index -= children.count();
        if (index < meta->propertyCount()) {
            QScriptNameIdImpl *nameId = eng->nameId(QLatin1String(meta->property(index).name()));
            member->native(nameId, index,
                           QScriptValue::Undeletable
                           | QScriptValue::SkipInEnumeration
                           | PROPERTY_ID);
            return true;
        }

        index -= meta->propertyCount();
        if (index < meta->methodCount()) {
            QScriptNameIdImpl *nameId = eng->nameId(QLatin1String(meta->method(index).signature()));
            member->native(nameId, index,
                           QScriptValue::Undeletable
                           | QScriptValue::SkipInEnumeration
                           | QScriptValue::ReadOnly
                           | METHOD_ID);
            return true;
        }

        return false;
    }

    virtual bool removeMember(const QScriptValueImpl &object,
                              const QScript::Member &member)
    {
        QObject *qobject = object.toQObject();
        if (!qobject || !member.isNativeProperty() || !member.isDeletable())
            return false;

        if ((member.flags() & ID_MASK) == DYNAPROPERTY_ID) {
            QByteArray name = qobject->dynamicPropertyNames().value(member.id());
            qobject->setProperty(name, QVariant());
            return true;
        }

        return false;
    }

    virtual void mark(const QScriptValueImpl &object, int generation)
    {
        ExtQObject::Instance *inst = ExtQObject::Instance::get(object, m_classInfo);
        inst->thisObject.mark(generation);
        if (inst->isConnection) {
            ConnectionQObject *connection = static_cast<ConnectionQObject*>((QObject*)inst->value);
            Q_ASSERT(connection != 0);
            connection->mark(generation);
        }
    }

private:
    QScriptClassInfo *m_classInfo;
};


struct StaticQtMetaObject : public QObject
{
    static const QMetaObject *get()
        { return &static_cast<StaticQtMetaObject*> (0)->staticQtMetaObject; }
};

bool ExtQMetaObjectData::resolve(const QScriptValueImpl &object, QScriptNameIdImpl *nameId,
                                 QScript::Member *member, QScriptValueImpl *base)
{
    QScript::ExtQMetaObject *self = static_cast<QScript::ExtQMetaObject*> (object.toFunction());
    const QMetaObject *meta = self->m_meta;

    QScriptEngine *eng = object.engine();

    QByteArray name = QScriptEnginePrivate::get(eng)->toString(nameId).toLatin1();

 again:
    for (int i = 0; i < meta->enumeratorCount(); ++i) {
        QMetaEnum e = meta->enumerator(i);

        for (int j = 0; j < e.keyCount(); ++j) {
            const char *key = e.key(j);

            if (! qstrcmp (key, name.constData())) {
                member->native(nameId, e.value(j), QScriptValue::ReadOnly);
                *base = object;
                return true;
            }
        }
    }

    if (meta != StaticQtMetaObject::get()) {
        meta = StaticQtMetaObject::get();
        goto again;
    }

    return false;
}

bool ExtQMetaObjectData::get(const QScriptValueImpl &obj, const QScript::Member &member,
                             QScriptValueImpl *result)
{
    if (! member.isNativeProperty())
        return false;

    *result = QScriptValueImpl(QScriptEnginePrivate::get(obj.engine()), member.id());
    return true;
}

void ExtQMetaObjectData::mark(const QScriptValueImpl &object, int generation)
{
    QScript::ExtQMetaObject *self = static_cast<QScript::ExtQMetaObject*> (object.toFunction());
    if (self->m_ctor.isObject())
        self->m_ctor.mark(generation);
}

} // ::QScript



QScript::ExtQObject::ExtQObject(QScriptEnginePrivate *eng, QScriptClassInfo *classInfo):
    Ecma::Core(eng), m_classInfo(classInfo)
{
    publicPrototype.invalidate();
    newQObject(&publicPrototype, 0);

    eng->newConstructor(&ctor, this, publicPrototype);
    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("findChild"),
                                eng->createFunction(method_findChild, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("findChildren"),
                                eng->createFunction(method_findChildren, 1, m_classInfo), flags);

    QExplicitlySharedDataPointer<QScriptClassData> data(new QScript::ExtQObjectData(eng, classInfo));
    m_classInfo->setData(data);
}

QScript::ExtQObject::~ExtQObject()
{
}

void QScript::ExtQObject::execute(QScriptContextPrivate *context)
{
    QScriptValueImpl tmp;
    newQObject(&tmp, 0);
    context->setReturnValue(tmp);
}

void QScript::ExtQObject::newQObject(QScriptValueImpl *result, QObject *value, bool isConnection)
{
    Instance *instance = new Instance();
    instance->value = value;
    instance->isConnection = isConnection;

    engine()->newObject(result, publicPrototype, classInfo());
    result->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

QScriptValueImpl QScript::ExtQObject::method_findChild(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QObject *obj = instance->value;
        QString name = context->argument(0).toString();
        QObject *child = qFindChild<QObject*>(obj, name);
        if (! child)
            return eng->nullValue();
        return eng->newQObject(child);
    }
    return eng->undefinedValue();
}

QScriptValueImpl QScript::ExtQObject::method_findChildren(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QObject *obj = instance->value;
        QList<QObject*> found;
        QScriptValueImpl arg = context->argument(0);
#ifndef QT_NO_REGEXP
        if (arg.isRegExp()) {
            QRegExp re = arg.toRegExp();
            found = qFindChildren<QObject*>(obj, re);
        } else
#endif
        {
            QString name = arg.toString();
            found = qFindChildren<QObject*>(obj, name);
        }
        QScriptValueImpl result = eng->newArray(found.size());
        for (int i = 0; i < found.size(); ++i) {
            QScriptValueImpl value = eng->newQObject(found.at(i));
            result.setProperty(i, value);
        }
        return result;
    }
    return eng->undefinedValue();
}

QScriptValueImpl QScript::ExtQObject::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QObject *obj = instance->value;
        const QMetaObject *meta = obj ? obj->metaObject() : &QObject::staticMetaObject;
        QString name = obj ? obj->objectName() : QString::fromUtf8("unnamed");

        QString str = QString::fromUtf8("%0(name = \"%1\")")
                      .arg(QLatin1String(meta->className())).arg(name);
        return QScriptValueImpl(eng, str);
    }
    return eng->undefinedValue();
}

QScript::ConnectionQObject::ConnectionQObject(const QMetaMethod &method,
                                              const QScriptValueImpl &sender,
                                              const QScriptValueImpl &receiver,
                                              const QScriptValueImpl &slot)
    : m_method(method), m_sender(sender),
      m_receiver(receiver)
{
    m_slot = slot;
    m_hasReceiver = (sender.isObject() && receiver.isObject()
                     && sender.objectValue() != receiver.objectValue());

    QScriptEngine *eng = m_slot.engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    QScriptValueImpl me;
    eng_p->qobjectConstructor->newQObject(&me, this, true);
    QScriptValuePrivate::init(m_self, eng_p->registerValue(me));

    QObject *qobject = static_cast<QtFunction*>(sender.toFunction())->object();
    Q_ASSERT(qobject);
    connect(qobject, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

QScript::ConnectionQObject::~ConnectionQObject()
{
}

static const uint qt_meta_data_ConnectionQObject[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ConnectionQObject[] = {
    "ConnectionQObject\0\0execute()\0"
};

const QMetaObject QScript::ConnectionQObject::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ConnectionQObject,
      qt_meta_data_ConnectionQObject, 0 }
};

const QMetaObject *QScript::ConnectionQObject::metaObject() const
{
    return &staticMetaObject;
}

void *QScript::ConnectionQObject::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ConnectionQObject))
        return static_cast<void*>(const_cast<ConnectionQObject*>(this));
    return QObject::qt_metacast(_clname);
}

int QScript::ConnectionQObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: execute(_a); break;
        }
        _id -= 1;
    }
    return _id;
}

void QScript::ConnectionQObject::execute(void **argv)
{
    Q_ASSERT(m_slot.isValid());

    QScriptEnginePrivate *eng = QScriptEnginePrivate::get(m_slot.engine());

    QScriptFunction *fun = eng->convertToNativeFunction(m_slot);
    Q_ASSERT(fun != 0);

    QList<QByteArray> parameterTypes = m_method.parameterTypes();
    int argc = parameterTypes.count();

    QScriptValueImpl activation;
    eng->newActivation(&activation);
    QScriptObject *activation_data = activation.objectValue();
    activation_data->m_scope = m_slot.scope();

    int formalCount = fun->formals.count();
    int mx = qMax(formalCount, argc);
    activation_data->m_members.resize(mx);
    activation_data->m_objects.resize(mx);
    for (int i = 0; i < mx; ++i) {
        QScriptNameIdImpl *nameId;
        if (i < formalCount)
            nameId = fun->formals.at(i);
        else
            nameId = 0;
        activation_data->m_members[i].object(nameId, i,
                                             QScriptValue::Undeletable
                                             | QScriptValue::SkipInEnumeration);
        if (i < argc) {
            int argType = QMetaType::type(parameterTypes.at(i));
            activation_data->m_objects[i] = eng->create(argType, argv[i + 1]);
        } else {
            activation_data->m_objects[i] = eng->undefinedValue();
        }
    }

    QScriptValueImpl senderObject;
    eng->qobjectConstructor->newQObject(&senderObject, sender());

    QScriptValueImpl thisObject;
    if (m_hasReceiver)
        thisObject = m_receiver;
    else
        thisObject = senderObject;

    QScriptContext *context = eng->pushContext();
    QScriptContextPrivate *context_data = QScriptContextPrivate::get(context);
    context_data->m_activation = activation;
    context_data->m_callee = m_slot;
    context_data->m_thisObject = thisObject;
    context_data->argc = argc;
    context_data->args = const_cast<QScriptValueImpl*> (activation_data->m_objects.constData());

    QScriptValueImpl tmp;
    if (m_hasReceiver) {
        tmp = m_receiver.property(QLatin1String("sender"));
        m_receiver.setProperty(QLatin1String("sender"), senderObject);
    }

    fun->execute(context_data);

    if (m_hasReceiver)
        m_receiver.setProperty(QLatin1String("sender"), tmp);

    if (context->state() == QScriptContext::ExceptionState) {
        qWarning() << "***" << context->returnValue().toString(); // ### fixme
    }

    eng->popContext();
}

void QScript::ConnectionQObject::mark(int generation)
{
    if (m_sender.isValid())
        m_sender.mark(generation);
    if (m_receiver.isValid())
        m_receiver.mark(generation);
    if (m_slot.isValid())
        m_slot.mark(generation);
}

bool QScript::ConnectionQObject::hasTarget(const QScriptValueImpl &receiver,
                                           const QScriptValueImpl &slot) const
{
    return ((receiver.objectValue() == m_receiver.objectValue())
            && (slot.objectValue() == m_slot.objectValue()));
}

void QScript::QtPropertyFunction::execute(QScriptContextPrivate *context)
{
    QScriptEngine *eng = context->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    QScriptValueImpl result = eng_p->undefinedValue();

    QMetaProperty prop = m_object->metaObject()->property(m_index);
    Q_ASSERT(prop.isScriptable());

    if (context->argumentCount() == 0) {
        // get
        if (prop.isValid()) {
            QScriptable *scriptable = scriptableFromQObject(m_object);
            QScriptEngine *oldEngine = 0;
            if (scriptable) {
                oldEngine = QScriptablePrivate::get(scriptable)->engine;
                QScriptablePrivate::get(scriptable)->engine = eng;
            }
            
            QVariant v = prop.read(m_object);
            
            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = oldEngine;
            
            result = eng_p->valueFromVariant(v);
        }
    } else {
        // set
        QVariant v = variantFromValue(prop.userType(), context->argument(0));

        QScriptable *scriptable = scriptableFromQObject(m_object);
        QScriptEngine *oldEngine = 0;
        if (scriptable) {
            oldEngine = QScriptablePrivate::get(scriptable)->engine;
            QScriptablePrivate::get(scriptable)->engine = eng;
        }

        prop.write(m_object, v);

        if (scriptable)
            QScriptablePrivate::get(scriptable)->engine = oldEngine;

        result = context->argument(0);
    }
    context->m_result = result;
}

void QScript::QtFunction::execute(QScriptContextPrivate *context)
{
    QScriptEngine *eng = context->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(context->engine());

    QScriptValueImpl result = eng_p->undefinedValue();

    Q_ASSERT(m_object);
    const QMetaObject *meta = m_object->metaObject();

    QObject *thisQObject = context->thisObject().toQObject();
    if (!thisQObject) // ### TypeError
        thisQObject = m_object;

    QByteArray funName = methodName(meta->method(m_initialIndex));
    if (!meta->cast(thisQObject)) {
        // ### find common superclass, see if initialIndex is
        //     in that class (or a superclass of that class),
        //     then it's still safe to execute it
        context->throwError(
            QString::fromUtf8("cannot execute %0: %1 does not inherit %2")
            .arg(QLatin1String(funName))
            .arg(QLatin1String(thisQObject->metaObject()->className()))
            .arg(QLatin1String(meta->className())));
        return;
    }

    QList<QVariant> vlist; // ### use QVector
    QBitArray argIsVariant;
    for (int index = m_initialIndex; index >= 0; --index) {
        QMetaMethod method = meta->method(index);
        QList<QByteArray> parameterTypes = method.parameterTypes();

        if ((methodName(method) != funName)
            || (context->argumentCount() != parameterTypes.count())) {
            if (index == 0) {
                if (!result.isError()) {
                    result = context->throwError(
                        QScriptContext::SyntaxError,
                        QString::fromUtf8("incorrect number or type of arguments in call to %0::%1()")
                        .arg(QLatin1String(meta->className()))
                        .arg(QLatin1String(funName)));
                }
            }
            continue;
        }

        bool converted = true;
        vlist.clear();
        argIsVariant.resize(1 + parameterTypes.count());

        QByteArray returnTypeName = method.typeName();
        int rtype = QMetaType::type(returnTypeName);
        if (rtype == 0 && !returnTypeName.isEmpty()) {
            if (returnTypeName == "QVariant") {
                argIsVariant.setBit(0, true);
            } else {
                result = context->throwError(
                    QScriptContext::TypeError,
                    QString::fromUtf8("cannot call %0::%1(): unknown return type `%2'")
                    .arg(QLatin1String(meta->className()))
                    .arg(QLatin1String(funName))
                    .arg(QLatin1String(method.typeName())));
                continue;
            }
        } else {
            argIsVariant.setBit(0, false);
        }
        vlist.append(QVariant(rtype, (void *)0)); // the result

        for (int i = 0; converted && i < context->argumentCount(); ++i) {
            QScriptValueImpl arg = context->argument(i);
            QByteArray argTypeName = parameterTypes.at(i);
            int atype = QMetaType::type(argTypeName);
            QVariant v(atype, (void *)0);

            if (atype == 0) {
                // either void (OK), QVariant (OK) or some other, unknown type (not OK)
                if (!argTypeName.isEmpty()) {
                    if (argTypeName == "QVariant") {
                        argIsVariant.setBit(1 + i, true);
                        v = arg.toVariant();
                    } else {
                        result = context->throwError(
                            QScriptContext::TypeError,
                            QString::fromUtf8("cannot call %0::%1(): unknown argument type `%2'")
                            .arg(QLatin1String(meta->className()))
                            .arg(QString::fromLatin1(funName))
                            .arg(QLatin1String(argTypeName)));
                        converted = false;
                        continue;
                    }
                }
            } else {
                argIsVariant.setBit(1 + i, false);
                converted = eng_p->convert(arg, atype, v.data());
                if (!converted && arg.isVariant()) {
                    QVariant vv = arg.toVariant();
                    if (vv.canConvert(QVariant::Type(atype))) {
                        v = vv;
                        converted = v.convert(QVariant::Type(atype));
                    }
                }
            }

            if (!converted) {
                if ((atype >= 256) && arg.isNumber()) {
                    // see if it's an enum value
                    int ival = arg.toInt32();
                    for (int e = 0; e < meta->enumeratorCount(); ++e) {
                        QMetaEnum m = meta->enumerator(e);
                        if (m.name() == argTypeName) {
                            if (m.valueToKey(ival) != 0) {
                                qVariantSetValue(v, ival);
                                converted = true;
                            }
                            break;
                        }
                    }
                }
            }
            vlist.append(v);
        }

        if (converted) {
            void **params = new void*[vlist.count()];

            for (int i = 0; i < vlist.count(); ++i) {
                const QVariant &v = vlist.at(i);
                if (argIsVariant.at(i))
                    params[i] = const_cast<QVariant*>(&v);
                else
                    params[i] = const_cast<void*>(v.constData());
            }

            QScriptable *scriptable = scriptableFromQObject(thisQObject);
            QScriptEngine *oldEngine = 0;
            if (scriptable) {
                oldEngine = QScriptablePrivate::get(scriptable)->engine;
                QScriptablePrivate::get(scriptable)->engine = eng;
            }

            thisQObject->qt_metacall(QMetaObject::InvokeMetaMethod, index, params);

            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = oldEngine;

            if (context->state() == QScriptContext::ExceptionState) {
                result = context->returnValue(); // propagate
            } else {
                if (rtype != 0) {
                    result = eng_p->create(rtype, params[0]);
                    if (!result.isValid())
                        result = eng_p->newVariant(QVariant(rtype, params[0]));
                } else if (returnTypeName == "QVariant") {
                    result = eng_p->newVariant(*(QVariant *)params[0]);
                } else {
                    result = eng_p->undefinedValue();
                }
            }

            delete[] params;
            break;
        } else if (! m_maybeOverloaded) {
            break;
        }
    }

    context->m_result = result;
}

bool QScript::QtFunction::createConnection(const QScriptValueImpl &self,
                                           const QScriptValueImpl &receiver,
                                           const QScriptValueImpl &slot)
{
    Q_ASSERT(slot.isFunction());

    const QMetaObject *meta = m_object->metaObject();
    int index = m_initialIndex;
    QMetaMethod method = meta->method(index);
    if (maybeOverloaded() && (method.attributes() & QMetaMethod::Cloned)) {
        // find the most general method
        do {
            method = meta->method(--index);
        } while (method.attributes() & QMetaMethod::Cloned);
    }

    QObject *conn = new ConnectionQObject(method, self, receiver, slot);
    m_connections.append(conn);
    return QMetaObject::connect(m_object, index, conn, conn->metaObject()->methodOffset());
}

bool QScript::QtFunction::destroyConnection(const QScriptValueImpl &,
                                            const QScriptValueImpl &receiver,
                                            const QScriptValueImpl &slot)
{
    Q_ASSERT(slot.isFunction());
    // find the connection with the given receiver+slot
    QObject *conn = 0;
    for (int i = 0; i < m_connections.count(); ++i) {
        ConnectionQObject *candidate = static_cast<ConnectionQObject*>((QObject*)m_connections.at(i));
        if (candidate->hasTarget(receiver, slot)) {
            conn = candidate;
            m_connections.removeAt(i);
            break;
        }
    }
    if (! conn)
        return false;

    const QMetaObject *meta = m_object->metaObject();
    int index = m_initialIndex;
    QMetaMethod method = meta->method(index);
    if (maybeOverloaded() && (method.attributes() & QMetaMethod::Cloned)) {
        // find the most general method
        do {
            method = meta->method(--index);
        } while (method.attributes() & QMetaMethod::Cloned);
    }

    bool ok = QMetaObject::disconnect(m_object, index, conn, conn->metaObject()->methodOffset());
    delete conn;
    return ok;
}

QScript::QtFunction::~QtFunction()
{
    qDeleteAll(m_connections);
}

QScript::ExtQMetaObject::ExtQMetaObject(const QMetaObject *meta, const QScriptValueImpl &ctor):
    m_meta(meta), m_ctor(ctor)
{
}

void QScript::ExtQMetaObject::execute(QScriptContextPrivate *context)
{
    if (m_ctor.isFunction()) {
        QScriptValueImplList args;
        for (int i = 0; i < context->argumentCount(); ++i)
            args << context->argument(i);
        QScriptValueImpl result = m_ctor.call(context->thisObject(), args);
        context->m_thisObject = result;
        context->m_result = result;
    } else {
        context->m_result = context->throwError(
            QScriptContext::TypeError,
            QString::fromUtf8("no constructor for %s")
            .arg(QLatin1String(m_meta->className())));
    }
}
