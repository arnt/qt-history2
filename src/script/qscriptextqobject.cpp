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
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptcontext_p.h"
#include "qscriptfunction_p.h"
#include "qscriptable.h"
#include "qscriptable_p.h"

#include <QtCore/QtDebug>
#include <QtCore/QMetaMethod>
#include "qscriptextqobject_p.h"

// we use bits 15..12 of property flags
enum {
    ENUMERATOR_ID = 0 << 12,
    PROPERTY_ID = 1 << 12,
    DYNAPROPERTY_ID = 2 << 12,
    METHOD_ID = 3 << 12,
    CHILD_ID = 4 << 12,
    ID_MASK = 7 << 12,
    MAYBE_OVERLOADED = 8 << 12
};

namespace QScript {

static inline QByteArray methodName(const QMetaMethod &method)
{
    QByteArray signature = method.signature();
    return signature.left(signature.indexOf('('));
}

static inline QScriptValue valueFromVariant(QScriptEngine *eng, const QVariant &v)
{
    QScriptValue result = QScriptEnginePrivate::get(eng)->create(v.userType(), v.data());
    if (!result.isValid())
        result = eng->scriptValueFromVariant(v);
    return result;
}

static inline QVariant variantFromValue(int targetType, const QScriptValue &value)
{
    QVariant v(targetType, (void *)0);
    QScriptEngine *eng = value.engine();
    Q_ASSERT(eng);
    if (QScriptEnginePrivate::get(eng)->convert(value, targetType, v.data()))
        return v;
    if (uint(targetType) == QVariant::LastType)
        return value.toVariant();
    return QVariant();
}

ExtQObject::Instance *ExtQObject::Instance::get(const QScriptValue &object, QScriptClassInfo *klass)
{
    if (! klass || klass == object.impl()->classInfo())
        return static_cast<Instance*> (object.impl()->objectData().data());
    
    return 0;
}


void ExtQObject::Instance::execute(QScriptContext *context)
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
    ExtQObjectData(QScriptEngine *, QScriptClassInfo *classInfo)
        : m_classInfo(classInfo)
    {
    }

    virtual bool resolve(const QScriptValue &object, QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValue *)
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

    virtual bool get(const QScriptValue &obj, const QScript::Member &member, QScriptValue *result)
    {
        if (! member.isNativeProperty())
            return false;

        QScriptEngine *eng = obj.engine();
        QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

        ExtQObject::Instance *inst = ExtQObject::Instance::get(obj, m_classInfo);
        QObject *qobject = inst->value;

        switch (member.flags() & ID_MASK) {
        case ENUMERATOR_ID:
            eng_p->newNumber(result, member.id());
            break;

        case PROPERTY_ID: {
            const QMetaObject *meta = qobject->metaObject();
            QMetaProperty prop = meta->property(member.id());
            Q_ASSERT(prop.isScriptable());

            QScriptable *scriptable = scriptableFromQObject(qobject);
            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = eng;

            QVariant v = prop.read(qobject);

            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = 0;

            *result = valueFromVariant(eng, v);
        }   break;

        case DYNAPROPERTY_ID: {
            QByteArray name = qobject->dynamicPropertyNames().value(member.id());
            QVariant v = qobject->property(name);
            *result = valueFromVariant(eng, v);
        }   break;

        case METHOD_ID: {
            QScript::Member m;
            bool maybeOverloaded = (member.flags() & MAYBE_OVERLOADED) != 0;
            QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
            *result = eng_p->createFunction(new QtFunction(qobject, member.id(),
                                                           maybeOverloaded));
            // make it persist
            QScriptObject *instance = obj.impl()->objectValue();
            if (!instance->findMember(member.nameId(), &m)) {
                instance->createMember(member.nameId(), &m,
                                       QScriptValue::SkipInEnumeration);
            }
            instance->put(m, *result);
        }   break;

        case CHILD_ID: {
            QObject *child = qobject->children().at(member.id());
            *result = eng->scriptValueFromQObject(child);
        }   break;

        } // switch

        return true;
    }

    virtual bool put(QScriptValue *object, const QScript::Member &member, const QScriptValue &value)
    {
        if (! member.isNativeProperty() || ! member.isWritable())
            return false;

        ExtQObject::Instance *inst = ExtQObject::Instance::get(*object, m_classInfo);
        QObject *qobject = inst->value;

        switch (member.flags() & ID_MASK) {
        case ENUMERATOR_ID:
        case CHILD_ID:
            return false;

        case METHOD_ID: {
            QScript::Member m;
            QScriptObject *instance = object->impl()->objectValue();
            if (!instance->findMember(member.nameId(), &m)) {
                instance->createMember(member.nameId(), &m,
                                       QScriptValue::SkipInEnumeration);
            }
            instance->put(m, value);
            return true;
        }

        case PROPERTY_ID: {
            QScriptEngine *eng = object->engine();
            const QMetaObject *meta = qobject->metaObject();
            QMetaProperty prop = meta->property(member.id());
            Q_ASSERT(prop.isScriptable());
            QVariant v = variantFromValue(prop.type(), value);

            QScriptable *scriptable = scriptableFromQObject(qobject);
            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = eng;

            bool ok = prop.write(qobject, v);

            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = 0;

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

    virtual int extraMemberCount(const QScriptValue &object)
    {
        QObject *qobject = object.toQObject();
        if (! qobject)
            return 0;
        const QMetaObject *meta = qobject->metaObject();

        return qobject->children().count()
            + meta->propertyCount()
            + meta->methodCount();
    }

    virtual bool extraMember(const QScriptValue &object, int index, QScript::Member *member)
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

    virtual bool removeMember(const QScriptValue &object,
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

    virtual void mark(const QScriptValue &object, int generation)
    {
        ExtQObject::Instance *inst = ExtQObject::Instance::get(object, m_classInfo);
        inst->thisObject.mark(generation);
        if (inst->isConnection) {
            ConnectionQObject *connection = static_cast<ConnectionQObject*>((QObject*)inst->value);
            Q_ASSERT(connection != 0);
            connection->senderObject.mark(generation);
            connection->receiverObject.mark(generation);
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

bool ExtQClassData::resolve(const QScriptValue &object, QScriptNameIdImpl *nameId,
                                 QScript::Member *member, QScriptValue *base)
{
    QScript::ExtQClass *self = static_cast<QScript::ExtQClass*> (object.impl()->toFunction());
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

bool ExtQClassData::get(const QScriptValue &obj, const QScript::Member &member,
                             QScriptValue *result)
{
    if (! member.isNativeProperty())
        return false;

    *result = obj.engine()->scriptValue(member.id());
    return true;
}

void ExtQClassData::mark(const QScriptValue &object, int generation)
{
    QScript::ExtQClass *self = static_cast<QScript::ExtQClass*> (object.impl()->toFunction());
    if (self->m_ctor.isObject())
        self->m_ctor.mark(generation);
}

} // ::QScript



QScript::ExtQObject::ExtQObject(QScriptEngine *eng, QScriptClassInfo *classInfo):
    Ecma::Core(eng), m_classInfo(classInfo)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    publicPrototype.invalidate();
    newQObject(&publicPrototype, 0);

    eng_p->newConstructor(&ctor, this, publicPrototype);
    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 0, m_classInfo), flags);

    QExplicitlySharedDataPointer<QScriptClassData> data(new QScript::ExtQObjectData(eng, classInfo));
    m_classInfo->setData(data);
}

QScript::ExtQObject::~ExtQObject()
{
}

void QScript::ExtQObject::execute(QScriptContext *context)
{
    QScriptValue tmp;
    newQObject(&tmp, 0);
    context->setReturnValue(tmp);
}

void QScript::ExtQObject::newQObject(QScriptValue *result, QObject *value, bool isConnection)
{
    Instance *instance = new Instance();
    instance->value = value;
    instance->isConnection = isConnection;

    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    result->impl()->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

QScriptValue QScript::ExtQObject::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QObject *obj = instance->value;
        const QMetaObject *meta = obj ? obj->metaObject() : &QObject::staticMetaObject;
        QString name = obj ? obj->objectName() : QString::fromUtf8("unnamed");

        QString str = QString::fromUtf8("%0(name = \"%1\")")
                      .arg(QLatin1String(meta->className())).arg(name);
        return eng->scriptValue(str);
    }
    return eng->undefinedScriptValue();
}

QScript::ConnectionQObject::ConnectionQObject(const QMetaMethod &m, const QScriptValue &sender,
                                              const QScriptValue &receiver)
    : method(m), senderObject(sender), receiverObject(receiver)
{
    eng = receiver.engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    eng_p->qobjectConstructor->newQObject(&self, this, true);
    eng->addRootObject(self);

    QObject *qobject = static_cast<QtFunction*>(sender.impl()->toFunction())->object();
    Q_ASSERT(qobject);
    connect(qobject, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

QScript::ConnectionQObject::~ConnectionQObject()
{
    if (! eng->rootObjects().isEmpty())
        eng->removeRootObject(self);
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
    Q_ASSERT(receiverObject.isValid());

    QScriptEngine *eng = receiverObject.engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    QScriptFunction *fun = eng_p->convertToNativeFunction(receiverObject);
    Q_ASSERT(fun != 0);

    QList<QByteArray> parameterTypes = method.parameterTypes();
    int argc = parameterTypes.count();

    QScriptValue activation;
    QScriptEnginePrivate::get(eng)->newActivation(&activation);
    QScriptObject *activation_data = activation.impl()->objectValue();
    activation_data->m_scope = receiverObject.impl()->scope();

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
            activation_data->m_objects[i] = eng_p->create(argType, argv[i + 1]);
        } else {
            activation_data->m_objects[i] = eng->undefinedScriptValue();
        }
    }

    QScriptValue thisObject;
    eng_p->qobjectConstructor->newQObject(&thisObject, sender());

    QScriptContext *context = eng_p->pushContext();
    QScriptContextPrivate *context_data = QScriptContextPrivate::get(context);
    context_data->activation = activation;
    context_data->callee = receiverObject;
    context_data->thisObject = thisObject;
    context_data->argc = argc;
    context_data->args = const_cast<QScriptValue*> (activation_data->m_objects.constData());
    fun->execute(context);
    if (context->state() == QScriptContext::Exception) {
        qWarning() << "***" << context->returnValue().toString(); // ### fixme
    }
    eng_p->popContext();
}

void QScript::QtFunction::execute(QScriptContext *context)
{
    QScriptEngine *eng = context->engine();

    QScriptValue result = eng->undefinedScriptValue();

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

        int rtype = QMetaType::type(method.typeName());
        if (rtype == 0 && (qstrlen(method.typeName()) > 0)) {
            result = context->throwError(
                QScriptContext::TypeError,
                QString::fromUtf8("cannot call %0::%1(): unknown return type `%2'")
                .arg(QLatin1String(meta->className()))
                .arg(QLatin1String(funName))
                .arg(QLatin1String(method.typeName())));
            continue;
        }
        vlist.append(QVariant(rtype, (void *)0)); // the result

        for (int i = 0; converted && i < context->argumentCount(); ++i) {
            QByteArray argTypeName = parameterTypes.at(i);
            int atype = QMetaType::type(argTypeName);
            if (atype == 0 && !argTypeName.isEmpty()) {
                result = context->throwError(
                    QScriptContext::TypeError,
                    QString::fromUtf8("cannot call %0::%1(): unknown argument type `%2'")
                    .arg(QLatin1String(meta->className()))
                    .arg(QString::fromLatin1(funName))
                    .arg(QLatin1String(argTypeName)));
                converted = false;
                continue;
            }

            QScriptValue arg = context->argument(i);
            QVariant v(atype, (void *)0);
            if (arg.isVariant())
                v = arg.toVariant();
            else
                converted = QScriptEnginePrivate::get(eng)->convert(arg, atype, v.data());
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

            for (int i = 0; i < vlist.count(); ++i)
                params[i] = const_cast<void*>(vlist.at(i).constData());

            QScriptable *scriptable = scriptableFromQObject(thisQObject);
            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = eng;

            thisQObject->qt_metacall(QMetaObject::InvokeMetaMethod, index, params);

            if (scriptable)
                QScriptablePrivate::get(scriptable)->engine = 0;

            if (context->state() == QScriptContext::Exception) {
                result = context->returnValue(); // propagate
            } else {
                int returnType = QMetaType::type(method.typeName());
                if (returnType != 0) {
                    result = QScriptEnginePrivate::get(eng)->create(returnType, params[0]);
                    if (!result.isValid())
                        result = eng->scriptValueFromVariant(QVariant(returnType, params[0]));
                } else {
                    result = eng->undefinedScriptValue();
                }
            }

            delete[] params;
            break;
        } else if (! m_maybeOverloaded) {
            break;
        }
    }

    QScriptContextPrivate::get(context)->result = result;
}

bool QScript::QtFunction::createConnection(const QScriptValue &sender, const QScriptValue &receiver)
{
    Q_ASSERT(sender.isFunction());
    Q_ASSERT(receiver.isFunction());
    QObject *qobject = static_cast<QtFunction*>(sender.impl()->toFunction())->object();
    Q_ASSERT(qobject);

    const QMetaObject *meta = qobject->metaObject();
    int index = m_initialIndex;
    QMetaMethod method = meta->method(index);
    if (maybeOverloaded() && (method.attributes() & QMetaMethod::Cloned)) {
        // find the most general method
        do {
            method = meta->method(--index);
        } while (method.attributes() & QMetaMethod::Cloned);
    }

    QObject *slot = new ConnectionQObject(method, sender, receiver);
    m_slotObjects.append(slot);
    return QMetaObject::connect(qobject, index, slot, slot->metaObject()->methodOffset());
}

bool QScript::QtFunction::destroyConnection(const QScriptValue &sender, const QScriptValue &receiver)
{
    Q_ASSERT(sender.isFunction());
    Q_ASSERT(receiver.isFunction());
    QObject *qobject = static_cast<QtFunction*>(sender.impl()->toFunction())->object();
    Q_ASSERT(qobject);

    // find the slot with the given receiver
    QObject *slot = 0;
    for (int i = 0; i < m_slotObjects.count(); ++i) {
        ConnectionQObject *conn = static_cast<ConnectionQObject*>((QObject*)m_slotObjects.at(i));
        if (conn->receiverObject.impl()->objectValue() == receiver.impl()->objectValue()) {
            slot = conn;
            m_slotObjects.removeAt(i);
            break;
        }
    }
    if (! slot)
        return false;

    const QMetaObject *meta = qobject->metaObject();
    int index = m_initialIndex;
    QMetaMethod method = meta->method(index);
    if (maybeOverloaded() && (method.attributes() & QMetaMethod::Cloned)) {
        // find the most general method
        do {
            method = meta->method(--index);
        } while (method.attributes() & QMetaMethod::Cloned);
    }

    bool ok = QMetaObject::disconnect(qobject, index, slot, slot->metaObject()->methodOffset());
    delete slot;
    return ok;
}

QScript::QtFunction::~QtFunction()
{
    qDeleteAll(m_slotObjects);
}

QScript::ExtQClass::ExtQClass(const QMetaObject *meta, const QScriptValue &ctor):
    m_meta(meta), m_ctor(ctor)
{
}

void QScript::ExtQClass::execute(QScriptContext *context)
{
    if (m_ctor.isFunction()) {
        QScriptValueList args;
        for (int i = 0; i < context->argumentCount(); ++i)
            args << context->argument(i);
        QScriptValue result = m_ctor.call(context->thisObject(), args);
        QScriptContextPrivate::get(context)->thisObject = result;
        QScriptContextPrivate::get(context)->result = result;
    } else {
        QScriptContextPrivate::get(context)->result = context->throwError(
            QScriptContext::TypeError,
            QString::fromUtf8("no constructor for %s")
            .arg(QLatin1String(m_meta->className())));
    }
}
