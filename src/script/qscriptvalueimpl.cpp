#include "qscriptvalueimpl_p.h"
#include "qscriptengine_p.h"
#include "qscriptobject_p.h"
#include "qscriptecmaregexp_p.h"
#include "qscriptecmadate_p.h"
#include "qscriptecmaerror_p.h"
#include "qscriptvalue_p.h"

#include <QtCore/QDateTime>
#include <QtCore/QRegExp>

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

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, QScriptValue::SpecialValue value)
{
    if (value == QScriptValue::NullValue)
        engine->newNull(this);
    else if (value == QScriptValue::UndefinedValue)
        engine->newUndefined(this);
}

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, bool val)
{
    engine->newBoolean(this, val);
}

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, int val)
{
    engine->newNumber(this, val);
}

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, uint val)
{
    engine->newNumber(this, val);
}

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, qlonglong val)
{
    engine->newNumber(this, val);
}

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, qulonglong val)
{
#if defined(Q_OS_WIN) && _MSC_FULL_VER <= 12008804
#pragma message("** NOTE: You need the Visual Studio Processor Pack to compile support for 64bit unsigned integers.")
    engine->newNumber(this, (qlonglong)val);
#else
    engine->newNumber(this, val);
#endif
}

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, qsreal val)
{
    engine->newNumber(this, val);
}

QScriptValueImpl::QScriptValueImpl(QScriptEnginePrivate *engine, const QString &val)
{
    engine->newString(this, val);
}

QScriptValueImpl::operator QScriptValue() const
{
    if (!isValid())
        return QScriptValue();

    QScriptValuePrivate *p = QScriptEnginePrivate::get(engine())->registerValue(*this);
    QScriptValue v;
    QScriptValuePrivate::init(v, p);
    return v;
}

bool QScriptValueImpl::isArray() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->arrayConstructor->classInfo();
}

 bool QScriptValueImpl::isDate() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->dateConstructor->classInfo();
}

bool QScriptValueImpl::isError() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->errorConstructor->classInfo();
}

 bool QScriptValueImpl::isRegExp() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->regexpConstructor->classInfo();
}

qsreal QScriptValueImpl::toNumber() const
{
    if (!isValid())
        return 0;
    return QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
}

bool QScriptValueImpl::toBoolean() const
{
    if (!isValid())
        return false;
    return QScriptEnginePrivate::get(engine())->convertToNativeBoolean(*this);
}

QString QScriptValueImpl::toString() const
{
    if (!isValid())
        return QString();
    return QScriptEnginePrivate::get(engine())->convertToNativeString(*this);
}

qint32 QScriptValueImpl::toInt32() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toInt32(d);
}

quint32 QScriptValueImpl::toUInt32() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toUint32(d);
}

quint16 QScriptValueImpl::toUInt16() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toUint16(d);
}

qsreal QScriptValueImpl::toInteger() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toInteger(d);
}

QVariant QScriptValueImpl::toVariant() const
{
    if (!isValid())
        return QVariant();
    switch (m_class->type()) {

    case QScript::UndefinedType:
    case QScript::PointerType:
    case QScript::FunctionType:
        break;

    case QScript::NullType:
        return QVariant(0); // ### hmm...

    case QScript::BooleanType:
        return QVariant(m_bool_value);

    case QScript::IntegerType:
        return QVariant(m_int_value);

    case QScript::NumberType:
        return QVariant(m_number_value);

    case QScript::StringType:
        return QVariant(m_string_value->s);

    case QScript::VariantType:
        return variantValue();

#ifndef QT_NO_QOBJECT
    case QScript::QObjectType:
        return qVariantFromValue(toQObject());
#endif

    default: {
        QScriptValue v = toPrimitive();

        if (!v.isObject())
            return v.toVariant();
    }

    } // switch
    return QVariant();
}

QScriptValueImpl QScriptValueImpl::toObject() const
{
    if (!isValid())
        return QScriptValueImpl();
    return QScriptEnginePrivate::get(engine())->toObject(*this);
}

QDateTime QScriptValueImpl::toDateTime() const
{
    if (!isDate())
        return QDateTime();
    return QScriptEnginePrivate::get(engine())->toDateTime(*this);
}

#ifndef QT_NO_REGEXP
QRegExp QScriptValueImpl::toRegExp() const
{
    if (!isRegExp())
        return QRegExp();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return eng_p->regexpConstructor->toRegExp(*this);
}
#endif // QT_NO_REGEXP

QScriptValueImpl QScriptValueImpl::toPrimitive(QScriptValue::TypeHint hint) const
{
    if (!isValid())
        return QScriptValueImpl();
    return QScriptEnginePrivate::get(engine())->toPrimitive(*this, hint);
}

QObject *QScriptValueImpl::toQObject() const
{
#ifndef QT_NO_QOBJECT
    if (isQObject()) {
        QScript::ExtQObject *ctor = QScriptEnginePrivate::get(engine())->qobjectConstructor;
        Q_ASSERT(ctor != 0);

        QScript::ExtQObject::Instance *data = ctor->get(*this);
        Q_ASSERT(data != 0);

        return data->value;
    }
#endif

    return 0;
}

QScriptValueImpl QScriptValueImpl::prototype() const
{
    if (!isObject())
        return QScriptValueImpl();
    return m_object_value->m_prototype;
}

void QScriptValueImpl::setPrototype(const QScriptValueImpl &prototype)
{
    if (isObject())
        m_object_value->m_prototype = prototype;
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

QExplicitlySharedDataPointer<QScriptObjectData> QScriptValueImpl::objectData() const
{
    Q_ASSERT(isObject());
    return m_object_value->m_data;
}

void QScriptValueImpl::setObjectData(QExplicitlySharedDataPointer<QScriptObjectData> data)
{
    Q_ASSERT(isObject());
    m_object_value->m_data = data;
}

bool QScriptValueImpl::resolve(QScriptNameIdImpl *nameId, QScript::Member *member,
                               QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const
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

void QScriptValueImpl::get(const QScript::Member &member, QScriptValueImpl *obj) const
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

void QScriptValueImpl::get(QScriptNameIdImpl *nameId, QScriptValueImpl *out)
{
    QScript::Member m;
    QScriptValueImpl o;
    if (resolve(nameId, &m, &o, QScriptValue::ResolvePrototype))
        o.get(m, out);
    else
        QScriptEnginePrivate::get(engine())->newUndefined(out);
}

void QScriptValueImpl::get_helper(const QScript::Member &member, QScriptValueImpl *obj) const
{
    QScriptEnginePrivate *eng = QScriptEnginePrivate::get(engine());

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

void QScriptValueImpl::put(const QScript::Member &member, const QScriptValueImpl &object)
{
    Q_ASSERT(isObject());
    Q_ASSERT(member.isValid());
    // Q_ASSERT(member.isWritable());

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());

    if (member.isObjectProperty()) {
        Q_ASSERT(member.nameId()->unique);
        Q_ASSERT(member.id() >= 0);
        Q_ASSERT(member.id() < m_object_value->memberCount());
        m_object_value->put(member, object);
    }

    else if (member.nameId() == eng_p->idTable()->id___proto__) {
        if (object.isNull()) // only Object.prototype.__proto__ can be null
            setPrototype(eng_p->undefinedValue());
        else {
            QScriptValueImpl was = prototype();
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

void QScriptValueImpl::setQObjectValue(QObject *object)
{
#ifndef QT_NO_QOBJECT
    Q_ASSERT(isQObject());

    QScript::ExtQObject *ctor = QScriptEnginePrivate::get(engine())->qobjectConstructor;
    Q_ASSERT(ctor != 0);

    QScript::ExtQObject::Instance *data = ctor->get(*this);
    Q_ASSERT(data != 0);

    data->value = object;
#else
    Q_UNUSED(object);
#endif
}

QVariant QScriptValueImpl::variantValue() const
{
    Q_ASSERT(isVariant());

    QScript::Ext::Variant *ctor = QScriptEnginePrivate::get(engine())->variantConstructor;
    Q_ASSERT(ctor != 0);

    QScript::Ext::Variant::Instance *data = ctor->get(*this);
    Q_ASSERT(data != 0);

    return data->value;
}

void QScriptValueImpl::setVariantValue(const QVariant &value)
{
    if (!isVariant())
        return;

    QScript::Ext::Variant *ctor = QScriptEnginePrivate::get(engine())->variantConstructor;
    Q_ASSERT(ctor != 0);

    QScript::Ext::Variant::Instance *data = ctor->get(*this);
    Q_ASSERT(data != 0);

    data->value = value;
}

QScriptValueImpl QScriptValueImpl::internalValue() const
{
    Q_ASSERT(isObject());
    return m_object_value->m_internalValue;
}

void QScriptValueImpl::setInternalValue(const QScriptValueImpl &internalValue)
{
    Q_ASSERT(isObject());
    m_object_value->m_internalValue = internalValue;
}

void QScriptValueImpl::removeMember(const QScript::Member &member)
{
    if (member.isObjectProperty())
        m_object_value->removeMember(member);

    else if (QScriptClassData *data = m_class->data())
        data->removeMember(*this, member);
}

void QScriptValueImpl::createMember(QScriptNameIdImpl *nameId,
                                    QScript::Member *member, uint flags)
{
    Q_ASSERT(isObject());
    
    QScriptObject *object_data = m_object_value;
    object_data->createMember(nameId, member, flags);
    Q_ASSERT(member->isObjectProperty());
}

QScriptValueImpl QScriptValueImpl::scope() const
{
    Q_ASSERT(isObject());
    return m_object_value->m_scope;
}

void QScriptValueImpl::setScope(const QScriptValueImpl &scope)
{
    Q_ASSERT(isObject());
    m_object_value->m_scope = scope;
}

int QScriptValueImpl::memberCount() const
{
    Q_ASSERT(isObject());

    int count = m_object_value->memberCount();
    
    if (m_class->data())
        count += m_class->data()->extraMemberCount(*this);
    
    return count;
}

void QScriptValueImpl::member(int index, QScript::Member *member) const
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

QScriptFunction *QScriptValueImpl::toFunction() const
{
    if (!isFunction())
        return 0;
    return QScriptEnginePrivate::get(engine())->convertToNativeFunction(*this);
}

QScriptValueImpl QScriptValueImpl::property(QScriptNameIdImpl *nameId,
                                            const QScriptValue::ResolveFlags &mode) const
{
    if (!isObject())
        return QScriptValueImpl();

    QScriptValueImpl base;
    QScript::Member member;

    if (! resolve(nameId, &member, &base, mode))
        return QScriptValueImpl();

    QScriptValueImpl value;
    base.get(nameId, &value);
    if (member.isGetterOrSetter()) {
        QScriptValueImpl getter;
        if (member.isObjectProperty() && !member.isGetter())
            base.m_object_value->findGetter(&member);
        base.get(member, &getter);
        value = getter.call(*this);
    }
    return value;
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

void QScriptValueImpl::setProperty(const QString &name, const QScriptValueImpl &value,
                                   const QScriptValue::PropertyFlags &flags)
{
    if (!isObject())
        return;
    QScriptNameIdImpl *nameId = QScriptEnginePrivate::get(engine())->nameId(name);
    setProperty(nameId, value, flags);
}

QScriptValueImpl QScriptValueImpl::property(const QString &name,
                                            const QScriptValue::ResolveFlags &mode) const
{
    if (!isObject())
        return QScriptValueImpl();
    QScriptNameIdImpl *nameId = QScriptEnginePrivate::get(engine())->nameId(name);
    return property(nameId, mode);
}

QScriptValueImpl QScriptValueImpl::property(quint32 arrayIndex,
                                            const QScriptValue::ResolveFlags &mode) const
{
    if (!isObject())
        return QScriptValueImpl();

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    QScript::Ecma::Array::Instance *instance = eng_p->arrayConstructor->get(*this);
    if (instance)
        return instance->value.at(arrayIndex);

    QScriptValueImpl id;
    eng_p->newNumber(&id, arrayIndex);
    return property(id.toString(), mode);
}

void QScriptValueImpl::setProperty(quint32 arrayIndex, const QScriptValueImpl &value,
                                   const QScriptValue::PropertyFlags &flags)
{
    if (!isObject())
        return;

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    QScript::Ecma::Array::Instance *instance = eng_p->arrayConstructor->get(*this);
    if (instance) {
        instance->value.assign(arrayIndex, value);
        return;
    }

    QScriptValueImpl id;
    eng_p->newNumber(&id, arrayIndex);
    setProperty(id.toString(), value, flags);
}

QScriptValueImpl QScriptValueImpl::call(const QScriptValueImpl &thisObject,
                                        const QScriptValueImplList &args)
{
    if (!isFunction())
        return QScriptValueImpl();

    QScriptEngine *eng = engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    return eng_p->call(*this, thisObject, args, /*asConstructor=*/false);
}

QScriptValueImpl QScriptValueImpl::construct(const QScriptValueImplList &args)
{
    if (!isFunction())
        return QScriptValueImpl();

    QScriptEngine *eng = engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    QScriptValueImpl proto = property(QLatin1String("prototype"), QScriptValue::ResolveLocal);
    QScriptValueImpl object;
    eng_p->newObject(&object, proto);

    QScriptValueImpl result = eng_p->call(*this, object, args, /*asConstructor=*/true);
    if (result.isObject())
        return result;
    return object;
}

void QScriptValueImpl::mark(int generation) const
{
    if (! isValid())
        return;

    else if (isString())
        QScriptEnginePrivate::get(engine())->markString(m_string_value, generation);

    else if (isObject())
        QScriptEnginePrivate::get(engine())->markObject(*this, generation);
}

bool QScriptValueImpl::lessThan(const QScriptValueImpl &other) const
{
    if (!isValid() || !other.isValid())
        return false;

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return eng_p->lessThan(*this, other);
}

bool QScriptValueImpl::equalTo(const QScriptValueImpl &other) const
{
    if (!isValid() || !other.isValid())
        return isValid() == other.isValid();

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return eng_p->equalTo(*this, other);
}

bool QScriptValueImpl::strictEqualTo(const QScriptValueImpl &other) const
{
    if (!isValid() || !other.isValid())
        return isValid() == other.isValid();
    
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return eng_p->strictEqualTo(*this, other);
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
