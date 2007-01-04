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

#include "qscriptengine.h"
#include "qscriptvalue.h"
#include "qscriptmember_p.h"
#include "qscriptvalue_p.h"
#include "qscriptextvariant_p.h"
#include "qscriptextqobject_p.h"
#include "qscriptcontext_p.h"
#include "qscriptecmaerror_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmaregexp_p.h"
#include "qscriptecmadate_p.h"

/*!
  \since 4.3
  \class QScriptValue

  \brief The QScriptValue class acts as a container for the Qt Script data types.

  \ingroup script
  \mainclass

  QScriptValue supports the types defined in the \l{ECMA-262}
  standard: The primitive types, which are Undefined, Null, Boolean,
  Number, and String; and the Object type. Additionally, Qt Script
  defines two types: \c{Variant} (a QVariant), and \c{QObject} (a pointer to a
  QObject (or subclass)). Custom types are supported by means of the
  Qt meta type system; see qScriptRegisterMetaType().

  To obtain a QScriptValue, you use one of the scriptValue() or
  scriptValueFromT() methods in QScriptEngine
  (e.g. \c{QScriptEngine::scriptValue(123)}).

  The methods named isT() (e.g. isBoolean(), isUndefined()) can be
  used to test if a value is of a certain type. The methods named
  toT() (e.g. toBoolean(), toString()) can be used to convert a
  QScriptValue to another type. You can also use the generic
  qscript_cast() function.
  
  Object values have zero or more properties which are themselves
  QScriptValues. Use setProperty() to set a property of an object,
  and call property() to retrieve the value of a property.

  Object values have an internal \c{prototype} property, which can be
  accessed with prototype() and setPrototype(). Properties added to a
  prototype are shared by all objects having that prototype. For more
  information, see the \l{QtScript} documentation.

  Function objects (objects for which isFunction() returns true) can
  be invoked by calling call().

  \sa QScriptEngine
*/

/*!
    \enum QScriptValue::TypeHint

    This enum is used to provide toPrimitive() with the desired type
    of the return value.

    \value NoTypeHint No hint.

    \value NumberTypeHint A number value is desired.

    \value StringTypeHint A string value is desired.
*/

/*!
    \enum QScriptValue::PropertyFlag

    This enum describes the attributes of a property.

    \value ReadOnly The property is read-only. Attempts by Qt Script code to write to the property will be ignored.

    \value Undeletable Attempts by Qt Script code to \c{delete} the property will be ignored.

    \value SkipInEnumeration The property is not to be enumerated by a \c{for-in} enumeration.

    \value UserRange Flags in this range are not used by Qt Script, and can be used for custom purposes.
*/

/*!
    \enum QScriptValue::ResolveFlag

    This enum specifies how to look up a property of an object.

    \value ResolveLocal Only check the object's own properties. This is the default.

    \value ResolvePrototype Check the object's own properties first, then search the prototype chain.

    \value ResolveScope Check the object's own properties first, then search the scope chain.

    \value ResolveFull Check the object's own properties first, then search the prototype chain, and finally search the scope chain.
*/

/*!
  \fn QScriptValue::QScriptValue()
  
  Constructs an invalid QScriptValue.
*/

/*!
  Returns true if this QScriptValue is an object of the Error class;
  otherwise returns false.

  \sa QScriptContext::throwError()
*/
bool QScriptValue::isError() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->errorConstructor->classInfo();
}

/*!
  Returns true if this QScriptValue is an object of the Array class;
  otherwise returns false.

  \sa QScriptEngine::newArray()
*/
bool QScriptValue::isArray() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->arrayConstructor->classInfo();
}

/*!
  Returns true if this QScriptValue is an object of the Date class;
  otherwise returns false.

  \sa QScriptEngine::newDate()
*/
bool QScriptValue::isDate() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->dateConstructor->classInfo();
}

/*!
  Returns true if this QScriptValue is an object of the RegExp class;
  otherwise returns false.

  \sa QScriptEngine::newRegExp()
*/
bool QScriptValue::isRegExp() const
{
    if (!isObject())
        return false;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    return m_class == eng_p->regexpConstructor->classInfo();
}

/*!
  \fn QScriptValueImpl *QScriptValue::impl() const
  \internal
*/

/*!
  \internal
*/
QDebug &operator<<(QDebug &d, const QScriptValue &object)
{
    d.nospace() << "QScriptValue(";

    if (!object.isValid()) {
        d.nospace() << "Invalid)";
        return d;
    }

    switch (object.impl()->type()) {
    case QScript::BooleanType:
        d.nospace() << "bool=" << object.toBoolean();
        break;

    case QScript::IntegerType:
        d.nospace() << "int=" << object.toInt32();
        break;

    case QScript::NumberType:
        d.nospace() << "qnumber=" << object.toNumber();
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
            d.nospace() << object.impl()->classInfo()->name() << ",{";
            QScriptObject *od = object.impl()->objectValue();
            for (int i=0; i<od->memberCount(); ++i) {
                if (i != 0)
                    d << ",";

                QScript::Member m;
                od->member(i, &m);

                if (m.isValid() && m.isObjectProperty()) {
                    d << QScriptEnginePrivate::get(object.engine())->toString(m.nameId());
                    QScriptValue o;
                    od->get(m, &o);
                    d.nospace() << QLatin1String(":")
                                << (o.impl()->classInfo()
                                    ? o.impl()->classInfo()->name()
                                    : QLatin1String("?"));
                }
            }

            d.nospace() << "} scope={";
            QScriptValue scope = object.impl()->scope();
            while (scope.isValid()) {
                Q_ASSERT(scope.isObject());
                d.nospace() << " " << scope.impl()->objectValue();
                scope = scope.impl()->scope();
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

/*!
  If this QScriptValue is an object, returns the internal prototype
  (\c{__proto__} property) of this object; otherwise returns an
  invalid QScriptValue.

  \sa setPrototype(), isObject()
*/
QScriptValue QScriptValue::prototype() const
{
    if (!isObject())
        return QScriptValue();
    return m_object_value->m_prototype;
}

/*!
  If this QScriptValue is an object, sets the internal prototype
  (\c{__proto__} property) of this object to be \a prototype;
  otherwise does nothing.

  \sa prototype(), isObject()
*/
void QScriptValue::setPrototype(const QScriptValue &prototype)
{
    if (isObject())
        m_object_value->m_prototype = prototype;
}

/*!
  \internal
*/
void QScriptValueImpl::removeMember(const QScript::Member &member)
{
    if (member.isObjectProperty())
        m_object_value->removeMember(member);

    else if (QScriptClassData *data = m_class->data())
        data->removeMember(*this, member);
}

/*!
  Returns true if this QScriptValue is an instance of
  \a ctorValue; otherwise returns false.

  A QScriptValue A is considered to be an instance of
  QScriptValue B if B is in the prototype chain of A.
*/
bool QScriptValue::instanceOf(const QScriptValue &ctorValue) const
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

        const QScriptValue &proto = instance->m_prototype;

        if (! proto.isValid() || ! proto.isObject())
            break;

        instance = proto.m_object_value;
    }

    return false;
}

/*!
  Returns the string value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.8, "ToString".

  \sa isString()
*/
QString QScriptValue::toString() const
{
    if (!isValid())
        return QString();
    return QScriptEnginePrivate::get(engine())->convertToNativeString(*this);
}

/*!
  Returns the number value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.3, "ToString".

  \sa isNumber(), toInteger(), toInt32(), toUInt32(), toUInt16()
*/
qnumber QScriptValue::toNumber() const
{
    if (!isValid())
        return 0;
    return QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
}

/*!
  Returns the boolean value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.2, "ToBoolean".

  \sa isBoolean()
*/
bool QScriptValue::toBoolean() const
{
    if (!isValid())
        return false;
    return QScriptEnginePrivate::get(engine())->convertToNativeBoolean(*this);
}

/*!
  Returns the signed 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.5, "ToInt32".

  \sa toNumber(), toUInt32()
*/
qint32 QScriptValue::toInt32() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toInt32(d);
}

/*!
  Returns the unsigned 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.6, "ToUint32".

  \sa toNumber(), toInt32()
*/
quint32 QScriptValue::toUInt32() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toUint32(d);
}

/*!
  Returns the unsigned 16-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.7, "ToUint16".

  \sa toNumber()
*/
quint16 QScriptValue::toUInt16() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toUint16(d);
}

/*!
  Returns the integer value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.4, "ToInteger".

  \sa toNumber()
*/
qnumber QScriptValue::toInteger() const
{
    if (!isValid())
        return 0;
    double d = QScriptEnginePrivate::get(engine())->convertToNativeDouble(*this);
    return QScriptEnginePrivate::toInteger(d);
}

/*!
  \internal
*/
QScriptFunction *QScriptValueImpl::toFunction() const
{
    Q_ASSERT(isFunction());
    return QScriptEnginePrivate::get(engine())->convertToNativeFunction(*this);
}

/*!
  Returns the variant value of this QScriptValue.

  \sa isVariant()
*/
QVariant QScriptValue::toVariant() const
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
        return impl()->variantValue();

#ifndef QT_NO_QOBJECT
    case QScript::QObjectType:
        return QVariant(toQObject());
#endif

    default: {
        QScriptValue v = toPrimitive();

        if (!v.isObject())
            return v.toVariant();
    }

    } // switch
    return QVariant();
}

/*!
  Returns the object value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.9, "ToObject".

  \sa isObject()
*/
QScriptValue QScriptValue::toObject() const
{
    if (!isValid())
        return QScriptValue();
    return QScriptEnginePrivate::get(engine())->toObject(*this);
}

/*!
  Returns the primitive value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.1, "ToPrimitive".

  If this QScriptValue is an object, the given \a hint can be used
  to indicate the desired primitive type.
  
*/
QScriptValue QScriptValue::toPrimitive(TypeHint hint) const
{
    if (!isValid())
        return QScriptValue();
    return QScriptEnginePrivate::get(engine())->toPrimitive(*this, hint);
}

/*!
  \internal
*/
QVariant QScriptValueImpl::variantValue() const
{
    Q_ASSERT(isVariant());

    QScript::Ext::Variant *ctor = QScriptEnginePrivate::get(engine())->variantConstructor;
    Q_ASSERT(ctor != 0);

    QScript::Ext::Variant::Instance *data = ctor->get(*this);
    Q_ASSERT(data != 0);

    return data->value;
}

/*!
  Returns the QObject value of this QScriptValue.

  If this QScriptValue is a QObject, returns the QObject pointer
  that the QScriptValue represents; otherwise, returns 0.

  \sa isQObject()
*/
QObject *QScriptValue::toQObject() const
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

/*!
  Returns the value of this QScriptValue's property identified by \a nameId,
  using the given \a mode to resolve the property.

  Accessing a property by ID is faster than accessing it by name, so
  consider using this function if you are repeatedly accessing the same
  property.

  \sa setProperty(), QScriptEngine::nameId()
*/
QScriptValue QScriptValue::property(const QScriptNameId &nameId,
                                    const ResolveFlags &mode) const
{
    if (!isObject())
        return QScriptValue();

    QScriptValue base;
    QScript::Member member;

    if (! impl()->resolve(nameId, &member, &base, mode))
        return QScriptValue();

    base.impl()->get(nameId, &base);
    return base;
}

/*!
  Sets the value of this QScriptValue's property identified by \a nameId to
  the given \a value.

  If this QScriptValue is not an object, this function does nothing.

  \sa property(), QScriptEngine::nameId()
*/
void QScriptValue::setProperty(const QScriptNameId &nameId,
                               const QScriptValue &value,
                               const PropertyFlags &flags)
{
    if (!isValid())
        return;

    QScriptValue base;
    QScript::Member member;

    QScriptValueImpl *i = impl();

    if (! i->resolve(nameId, &member, &base, ResolveLocal))
        i->createMember(nameId, &member, flags);

    i->put(member, value);
}

/*!
  Sets the value of this QScriptValue's property with the given \a name to
  the given \a value.

  If this QScriptValue is not an object, this function does nothing.

  If this QScriptValue does not already have a property with name \a name,
  a new property is created; the given \a flags then specify how this
  property may be accessed by script code.

  \sa property()
*/
void QScriptValue::setProperty(const QString &name, const QScriptValue &value,
                               const PropertyFlags &flags)
{
    QScriptNameId nameId = engine()->nameId(name);
    setProperty(nameId, value, flags);
}

/*!
  Returns the value of this QScriptValue's property with the given \a name,
  using the given \a mode to resolve the property.

  If no such property exists, an invalid QScriptValue is returned.

  \sa setProperty()
*/
QScriptValue QScriptValue::property(const QString &name,
                                    const ResolveFlags &mode) const
{
    QScriptNameId nameId = engine()->nameId(name);
    return property(nameId, mode);
}

/*!
  \internal
*/
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

/*!
  Sets the variant value of this QScriptValue to be the given \a
  value. If this QScriptValue is not a variant, this function does
  nothing.

  \sa isVariant(), toVariant()
*/
void QScriptValue::setVariantValue(const QVariant &value)
{
    if (!isVariant())
        return;

    QScript::Ext::Variant *ctor = QScriptEnginePrivate::get(engine())->variantConstructor;
    Q_ASSERT(ctor != 0);

    QScript::Ext::Variant::Instance *data = ctor->get(*this);
    Q_ASSERT(data != 0);

    data->value = value;
}

/*!
  Calls this QScriptValue as a function, using \a thisObject as
  the `this' object in the function call, and passing \a args
  as arguments to the function. Returns the value returned from
  the function.

  If this QScriptValue is not a function, call() does nothing
  and returns an invalid QScriptValue.

  Note that if \a thisObject is not an object, the global object
  (see \l{QScriptEngine::globalObject()}) will be used as the
  `this' object.

  \sa isFunction()
*/
QScriptValue QScriptValue::call(const QScriptValue &thisObject,
                                const QScriptValueList &args)
{
    if (!isFunction())
        return QScriptValue();

    QScriptEngine *eng = engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    QScriptFunction *function = impl()->toFunction();

    QScriptContext *nested_frame = eng_p->pushContext();
    QScriptContextPrivate *nested = QScriptContextPrivate::get(nested_frame);
    Q_ASSERT(nested->stackPtr != 0);

    eng_p->newActivation(&nested->activation);
    nested->activation.m_object_value->m_scope = m_object_value->m_scope;

    QScriptObject *activation_data = nested->activation.m_object_value;

    QScriptValue undefined;
    eng_p->newUndefined(&undefined);

    int formalCount = function->formals.count();
    int argc = args.count();
    int mx = qMax(formalCount, argc);
    activation_data->m_members.resize(mx);
    activation_data->m_objects.resize(mx);
    for (int i = 0; i < mx; ++i) {
        QScriptNameIdImpl *nameId = 0;
        if (i < formalCount)
            nameId = function->formals.at(i);

        activation_data->m_members[i].object(nameId, i, SkipInEnumeration);
        activation_data->m_objects[i] = (i < argc) ? args.at(i) : undefined;
    }

    nested->argc = argc;
    QVector<QScriptValue> argsv = args.toVector();
    nested->args = const_cast<QScriptValue*> (argsv.constData());

    if (thisObject.isValid() && thisObject.isObject())
        nested->thisObject = thisObject;
    else
        nested->thisObject = eng_p->globalObject;
    nested->callee = *this;
    nested->functionNameId = 0; // ### fixme

    eng_p->newUndefined(&nested->result);
    function->execute(nested_frame);
    QScriptValue result = nested->result;
    nested->args = 0;
    eng_p->popContext();

    return result;
}

/*!
  \internal
*/
void QScriptValue::mark(int generation) const
{
    if (! isValid())
        return;

    else if (isString())
        QScriptEnginePrivate::get(engine())->markString(m_string_value, generation);

    else if (isObject())
        QScriptEnginePrivate::get(engine())->markObject(*this, generation);
}

/*!
  \internal
*/
QList<QScriptNameIdImpl*> QScriptValueImpl::propertyIds() const
{
    QList<QScriptNameIdImpl*> names;

    if (! (isValid() && isObject()))
        return names;

    int count = memberCount();
    for (int i = 0; i < count; ++i) {
        QScript::Member m;
        member(i, &m);
        if (! m.isValid() || ! m.nameId() || m.dontEnum())
            continue;

        names.append(m.nameId());
    }

    return names;
}

/*!
  \internal
  Returns the \c{length} property of this QScriptValue.

  This function is provided for convenience and performance when
  working with array objects.
*/
quint32 QScriptValueImpl::length() const
{
    Q_ASSERT(isObject());

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    QScript::Ecma::Array::Instance *instance = eng_p->arrayConstructor->get(*this);
    if (instance)
        return instance->value.count();

    QScriptValue len = property(eng_p->idTable()->id_length);
    if (! len.isValid())
        return 0;

    return eng_p->toUint32(len.toNumber());
}

/*!
  \internal
  Returns the property at the given \a index.

  This function is provided for convenience and performance when
  working with array objects.
*/
QScriptValue QScriptValueImpl::propertyByIndex(quint32 index) const
{
    Q_ASSERT(isObject());

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    QScript::Ecma::Array::Instance *instance = eng_p->arrayConstructor->get(*this);
    if (instance)
        return instance->value.at(index);

    QScriptValue id;
    eng_p->newNumber(&id, index);
    return property(id.toString());
}

/*!
  \internal
  Sets the property at the given \a index to the given \a value.

  This function is provided for convenience and performance when
  working with array objects.
*/
void QScriptValueImpl::setPropertyByIndex(quint32 index, const QScriptValue &value)
{
    Q_ASSERT(isObject());

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    QScript::Ecma::Array::Instance *instance = eng_p->arrayConstructor->get(*this);
    if (instance) {
        instance->value.assign(index, value);
        return;
    }

    QScriptValue id;
    eng_p->newNumber(&id, index);
    setProperty(id.toString(), value);
}

/*!
  Returns the QScriptEngine that created this QScriptValue,
  or 0 if this QScriptValue is invalid.
*/
QScriptEngine *QScriptValue::engine() const
{
    if (! m_class)
        return 0;

    return m_class->engine();
}

/*!
  Returns true if this QScriptValue is of the primitive type Boolean;
  otherwise returns false.

  \sa toBoolean()
*/
bool QScriptValue::isBoolean() const
{
    return m_class && m_class->type() == QScript::BooleanType;
}

/*!
  Returns true if this QScriptValue is of the primitive type Number;
  otherwise returns false.

  \sa toNumber()
*/
bool QScriptValue::isNumber() const
{
    return m_class && m_class->type() == QScript::NumberType;
}

/*!
  Returns true if this QScriptValue is of the primitive type String;
  otherwise returns false.

  \sa toString()
*/
bool QScriptValue::isString() const
{
    return m_class && m_class->type() == QScript::StringType;
}

/*!
  Returns true if this QScriptValue is a function; otherwise returns
  false.

  \sa call()
*/
bool QScriptValue::isFunction() const
{
    return m_class && (m_class->type() & QScript::FunctionBased);
}

/*!
  Returns true if this QScriptValue is of the primitive type Null;
  otherwise returns false.

  \sa QScriptEngine::nullScriptValue()
*/
bool QScriptValue::isNull() const
{
    return m_class && m_class->type() == QScript::NullType;
}

/*!
  Returns true if this QScriptValue is of the primitive type Undefined;
  otherwise returns false.

  \sa QScriptEngine::undefinedScriptValue()
*/
bool QScriptValue::isUndefined() const
{
    return m_class && m_class->type() == QScript::UndefinedType;
}

/*!
  Returns true if this QScriptValue is of the Object type; otherwise
  returns false.

  Note that function values, variant values and QObject values are
  objects, so this function will return true for such values.

  \sa toObject(), toPrimitive(), QScriptEngine::newObject()
*/
bool QScriptValue::isObject() const
{
    return m_class && (m_class->type() & QScript::ObjectBased);
}

/*!
  Returns true if this QScriptValue is a variant value;
  otherwise returns false.

  \sa toVariant(), QScriptEngine::scriptValueFromVariant()
*/
bool QScriptValue::isVariant() const
{
    return m_class && m_class->type() == QScript::VariantType;
}

/*!
  Returns true if this QScriptValue is a QObject; otherwise returns
  false.

  \sa toQObject(), QScriptEngine::scriptValueFromQObject()
*/
bool QScriptValue::isQObject() const
{
    return m_class && m_class->type() == QScript::QObjectType;
}

/*!
  \fn QScriptValue::impl()
  \internal
*/

/*!
  \internal
  Invalidates this QScriptValue.

  \sa isValid()
*/
void QScriptValue::invalidate()
{
    m_class = 0;
}

/*!
  Returns true if this QScriptValue is valid; otherwise returns
  false.
*/
bool QScriptValue::isValid() const
{
    return m_class && m_class->engine();
}
