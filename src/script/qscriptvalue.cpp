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

#include "qscriptvalue.h"
#include "qscriptvalue_p.h"
#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtCore/QDateTime>
#include <QtCore/QRegExp>

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
  qscriptvalue_cast() function.

  Object values have zero or more properties which are themselves
  QScriptValues. Use setProperty() to set a property of an object,
  and call property() to retrieve the value of a property.

  Object values have an internal \c{prototype} property, which can be
  accessed with prototype() and setPrototype(). Properties added to a
  prototype are shared by all objects having that prototype. For more
  information, see the \l{QtScript} documentation.

  Function objects (objects for which isFunction() returns true) can
  be invoked by calling call(). Constructor functions can be used to
  construct new objects by calling construct().

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

    \value PropertyGetter The property is defined by a function which will be called to get the property value.

    \value PropertySetter The property is defined by a function which will be called to set the property value.

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
  Constructs an invalid QScriptValue.
*/
QScriptValue::QScriptValue()
    : d_ptr(0)
{
}

/*!
  Destroys this QScriptValue.
*/
QScriptValue::~QScriptValue()
{
    if (d_ptr && !d_ptr->ref.deref()) {
        if (isValid()) {
            QScriptEnginePrivate::get(engine())->unregisterValue(d_ptr);
        } else {
            // the engine has already been deleted
            delete d_ptr;
        }
        d_ptr = 0;
    }
}

/*!
  Constructs a new QScriptValue that is a copy of \a other.
*/
QScriptValue::QScriptValue(const QScriptValue &other)
    : d_ptr(other.d_ptr)
{
    if (d_ptr)
        d_ptr->ref.ref();
}

/*!
*/
QScriptValue::QScriptValue(QScriptEngine *engine, QScriptValue::SpecialValue value)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    if (value == NullValue)
        eng_p->newNull(&v);
    else if (value == UndefinedValue)
        eng_p->newUndefined(&v);
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with a boolean value, \a val.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, bool val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    eng_p->newBoolean(&v, val);
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with an integer value, \a val.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, int val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    eng_p->newNumber(&v, val);
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with an unsigned integer value, \a val.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, uint val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    eng_p->newNumber(&v, val);
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with a qlonglong value, \a val.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, qlonglong val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    eng_p->newNumber(&v, val);
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with a qulonglong value, \a val.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, qulonglong val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
#if defined(Q_OS_WIN) && _MSC_FULL_VER <= 12008804
#pragma message("** NOTE: You need the Visual Studio Processor Pack to compile support for 64bit unsigned integers.")
    eng_p->newNumber(&v, (qlonglong)val);
#else
    eng_p->newNumber(&v, val);
#endif
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with a qsreal value, \a val.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, qsreal val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    eng_p->newNumber(&v, val);
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with a string value, \a val.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, const QString &val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    eng_p->newString(&v, val);
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}

/*!
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, const char *value)

  Constructs a new QScriptValue with a string value, \a val.
*/

#ifndef QT_NO_CAST_FROM_ASCII
QScriptValue::QScriptValue(QScriptEngine *engine, const char *val)
{
    QScriptValueImpl v;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine);
    eng_p->newString(&v, QString::fromAscii(val));
    d_ptr = eng_p->registerValue(v);
    d_ptr->ref.ref();
}
#endif

/*!
*/
QScriptValue &QScriptValue::operator=(const QScriptValue &other)
{
    if (d_ptr == other.d_ptr)
        return *this;
    if (d_ptr && !d_ptr->ref.deref()) {
        if (isValid()) {
            QScriptEnginePrivate::get(engine())->unregisterValue(d_ptr);
        } else {
            // the engine has already been deleted
            delete d_ptr;
        }
    }
    d_ptr = other.d_ptr;
    if (d_ptr)
        d_ptr->ref.ref();
    return *this;
}

/*!
  Returns true if this QScriptValue is an object of the Error class;
  otherwise returns false.

  \sa QScriptContext::throwError()
*/
bool QScriptValue::isError() const
{
    return QScriptValuePrivate::valueOf(*this).isError();
}

/*!
  Returns true if this QScriptValue is an object of the Array class;
  otherwise returns false.

  \sa QScriptEngine::newArray()
*/
bool QScriptValue::isArray() const
{
    return QScriptValuePrivate::valueOf(*this).isArray();
}

/*!
  Returns true if this QScriptValue is an object of the Date class;
  otherwise returns false.

  \sa QScriptEngine::newDate()
*/
bool QScriptValue::isDate() const
{
    return QScriptValuePrivate::valueOf(*this).isDate();
}

/*!
  Returns true if this QScriptValue is an object of the RegExp class;
  otherwise returns false.

  \sa QScriptEngine::newRegExp()
*/
bool QScriptValue::isRegExp() const
{
    return QScriptValuePrivate::valueOf(*this).isRegExp();
}

/*!
  If this QScriptValue is an object, returns the internal prototype
  (\c{__proto__} property) of this object; otherwise returns an
  invalid QScriptValue.

  \sa setPrototype(), isObject()
*/
QScriptValue QScriptValue::prototype() const
{
    return QScriptValuePrivate::valueOf(*this).prototype();
}

/*!
  If this QScriptValue is an object, sets the internal prototype
  (\c{__proto__} property) of this object to be \a prototype;
  otherwise does nothing.

  \sa prototype(), isObject()
*/
void QScriptValue::setPrototype(const QScriptValue &prototype)
{
    if (isObject() && prototype.isValid() && (prototype.engine() != engine())) {
        qWarning("QScriptValue::setPrototype() failed: "
                 "cannot set a prototype created in "
                 "a different engine");
        return;
    }
    QScriptValuePrivate::valueOf(*this).setPrototype(QScriptValuePrivate::valueOf(prototype));
}

/*!
  Returns true if this QScriptValue is an instance of
  \a ctorValue; otherwise returns false.

  A QScriptValue A is considered to be an instance of
  QScriptValue B if B is in the prototype chain of A.
*/
bool QScriptValue::instanceOf(const QScriptValue &ctorValue) const
{
    if (!ctorValue.isValid())
        return false;
    return QScriptValuePrivate::valueOf(*this).instanceOf(QScriptValuePrivate::valueOf(ctorValue));
}

/*!
  Returns true if this QScriptValue is less than \a other, otherwise
  returns false.  The comparison follows the behavior described in
  \l{ECMA-262} section 11.8.5, "The Abstract Relational Comparison
  Algorithm".
*/
bool QScriptValue::lessThan(const QScriptValue &other) const
{
    if (isValid() && other.isValid() && (other.engine() != engine())) {
        qWarning("QScriptValue::lessThan: "
                 "cannot compare to a value created in "
                 "a different engine");
        return false;
    }
    return QScriptValuePrivate::valueOf(*this).lessThan(QScriptValuePrivate::valueOf(other));
}

/*!
  Returns true if this QScriptValue is equal to \a other, otherwise
  returns false. The comparison follows the behavior described in
  \l{ECMA-262} section 11.9.3, "The Abstract Equality Comparison
  Algorithm".
*/
bool QScriptValue::equalTo(const QScriptValue &other) const
{
    if (isValid() && other.isValid() && (other.engine() != engine())) {
        qWarning("QScriptValue::equalTo: "
                 "cannot compare to a value created in "
                 "a different engine");
        return false;
    }
    return QScriptValuePrivate::valueOf(*this).equalTo(QScriptValuePrivate::valueOf(other));
}

/*!
  Returns true if this QScriptValue is equal to \a other using strict
  comparison (no conversion), otherwise returns false. The comparison
  follows the behavior described in \l{ECMA-262} section 11.9.6, "The
  Strict Equality Comparison Algorithm".
*/
bool QScriptValue::strictEqualTo(const QScriptValue &other) const
{
    if (isValid() && other.isValid() && (other.engine() != engine())) {
        qWarning("QScriptValue::strictEqualTo: "
                 "cannot compare to a value created in "
                 "a different engine");
        return false;
    }
    return QScriptValuePrivate::valueOf(*this).strictEqualTo(QScriptValuePrivate::valueOf(other));
}

/*!
  Returns the string value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.8, "ToString".

  \sa isString()
*/
QString QScriptValue::toString() const
{
    return QScriptValuePrivate::valueOf(*this).toString();
}

/*!
  Returns the number value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.3, "ToString".

  \sa isNumber(), toInteger(), toInt32(), toUInt32(), toUInt16()
*/
qsreal QScriptValue::toNumber() const
{
    return QScriptValuePrivate::valueOf(*this).toNumber();
}

/*!
  Returns the boolean value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.2, "ToBoolean".

  \sa isBoolean()
*/
bool QScriptValue::toBoolean() const
{
    return QScriptValuePrivate::valueOf(*this).toBoolean();
}

/*!
  Returns the signed 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.5, "ToInt32".

  \sa toNumber(), toUInt32()
*/
qint32 QScriptValue::toInt32() const
{
    return QScriptValuePrivate::valueOf(*this).toInt32();
}

/*!
  Returns the unsigned 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.6, "ToUint32".

  \sa toNumber(), toInt32()
*/
quint32 QScriptValue::toUInt32() const
{
    return QScriptValuePrivate::valueOf(*this).toUInt32();
}

/*!
  Returns the unsigned 16-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.7, "ToUint16".

  \sa toNumber()
*/
quint16 QScriptValue::toUInt16() const
{
    return QScriptValuePrivate::valueOf(*this).toUInt16();
}

/*!
  Returns the integer value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.4, "ToInteger".

  \sa toNumber()
*/
qsreal QScriptValue::toInteger() const
{
    return QScriptValuePrivate::valueOf(*this).toInteger();
}

/*!
  Returns the variant value of this QScriptValue.

  \sa isVariant()
*/
QVariant QScriptValue::toVariant() const
{
    return QScriptValuePrivate::valueOf(*this).toVariant();
}

/*!
  Returns the object value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.9, "ToObject".

  \sa isObject()
*/
QScriptValue QScriptValue::toObject() const
{
    return QScriptValuePrivate::valueOf(*this).toObject();
}

/*!
  Returns the QDateTime representation of this value.
  If this QScriptValue is not a date, or the value of the
  date is NaN (Not-a-Number), an invalid QDateTime is
  returned.

  \sa isDate()
*/
QDateTime QScriptValue::toDateTime() const
{
    return QScriptValuePrivate::valueOf(*this).toDateTime();
}

#ifndef QT_NO_REGEXP
/*!
  Returns the QRegExp representation of this value.
  If this QScriptValue is not a regular expression, an empty
  QRegExp is returned.

  \sa isRegExp()
*/
QRegExp QScriptValue::toRegExp() const
{
    return QScriptValuePrivate::valueOf(*this).toRegExp();
}
#endif // QT_NO_REGEXP

/*!
  Returns the primitive value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.1, "ToPrimitive".

  If this QScriptValue is an object, the given \a hint can be used
  to indicate the desired primitive type.

*/
QScriptValue QScriptValue::toPrimitive(TypeHint hint) const
{
    return QScriptValuePrivate::valueOf(*this).toPrimitive(hint);
}

/*!
  Returns the QObject value of this QScriptValue.

  If this QScriptValue is a QObject, returns the QObject pointer
  that the QScriptValue represents; otherwise, returns 0.

  \sa isQObject()
*/
QObject *QScriptValue::toQObject() const
{
    return QScriptValuePrivate::valueOf(*this).toQObject();
}

/*!
  \internal

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
    return QScriptValuePrivate::valueOf(*this).property(nameId, mode);
}

/*!
  \internal

  Sets the value of this QScriptValue's property identified by \a nameId to
  the given \a value.

  If this QScriptValue is not an object, this function does nothing.

  \sa property(), QScriptEngine::nameId()
*/
void QScriptValue::setProperty(const QScriptNameId &nameId,
                               const QScriptValue &value,
                               const PropertyFlags &flags)
{

    if (isValid() && value.isValid() && (value.engine() != engine())) {
        qWarning("QScriptValue::setProperty() failed: "
                 "cannot set value created in a different engine");
        return;
    }
    QScriptValuePrivate::valueOf(*this).setProperty(nameId, QScriptValuePrivate::valueOf(value), flags);
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
    if (isValid() && value.isValid() && (value.engine() != engine())) {
        qWarning("QScriptValue::setProperty() failed: "
                 "cannot set value created in a different engine");
        return;
    }
    QScriptValuePrivate::valueOf(*this).setProperty(name, QScriptValuePrivate::valueOf(value), flags);
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
    return QScriptValuePrivate::valueOf(*this).property(name, mode);
}

/*!
  Returns the property at the given \a arrayIndex.

  This function is provided for convenience and performance when
  working with array objects.
*/
QScriptValue QScriptValue::property(quint32 arrayIndex,
                                    const ResolveFlags &mode) const
{
    return QScriptValuePrivate::valueOf(*this).property(arrayIndex, mode);
}

/*!
  Sets the property at the given \a arrayIndex to the given \a value.

  This function is provided for convenience and performance when
  working with array objects.
*/
void QScriptValue::setProperty(quint32 arrayIndex, const QScriptValue &value,
                               const PropertyFlags &flags)
{
    if (isValid() && value.isValid() && (value.engine() != engine())) {
        qWarning("QScriptValue::setProperty() failed: "
                 "cannot set value created in a different engine");
        return;
    }
    QScriptValuePrivate::valueOf(*this).setProperty(arrayIndex, QScriptValuePrivate::valueOf(value), flags);
}

/*!
  Sets the variant value of this QScriptValue to be the given \a
  value. If this QScriptValue is not a variant, this function does
  nothing.

  \sa isVariant(), toVariant()
*/
void QScriptValue::setVariantValue(const QVariant &value)
{
    QScriptValuePrivate::valueOf(*this).setVariantValue(value);
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

  \sa construct()
*/
QScriptValue QScriptValue::call(const QScriptValue &thisObject,
                                const QScriptValueList &args)
{
    if (isFunction() && thisObject.isValid() && (thisObject.engine() != engine())) {
        qWarning("QScriptValue::call() failed: "
                 "cannot call function with thisObject created in "
                 "a different engine");
        return QScriptValue();
    }
    return QScriptValuePrivate::valueOf(*this).call(QScriptValuePrivate::valueOf(thisObject),
                                                    QScriptValuePrivate::toImplList(args));
}

/*!
  Creates a new \c{Object} and calls this QScriptValue as a constructor,
  using the created object as the `this' object and passing \a args
  as arguments. If the return value from the constructor call is an
  object, then that object is returned; otherwise the created object
  is returned.

  If this QScriptValue is not a function, construct() does nothing
  and returns an invalid QScriptValue.

  \sa call(), newObject()
*/
QScriptValue QScriptValue::construct(const QScriptValueList &args)
{
    return QScriptValuePrivate::valueOf(*this).construct(QScriptValuePrivate::toImplList(args));
}


/*!
  Returns the QScriptEngine that created this QScriptValue,
  or 0 if this QScriptValue is invalid.
*/
QScriptEngine *QScriptValue::engine() const
{
    return QScriptValuePrivate::valueOf(*this).engine();
}

/*!
  Returns true if this QScriptValue is of the primitive type Boolean;
  otherwise returns false.

  \sa toBoolean()
*/
bool QScriptValue::isBoolean() const
{
    return QScriptValuePrivate::valueOf(*this).isBoolean();
}

/*!
  Returns true if this QScriptValue is of the primitive type Number;
  otherwise returns false.

  \sa toNumber()
*/
bool QScriptValue::isNumber() const
{
    return QScriptValuePrivate::valueOf(*this).isNumber();
}

/*!
  Returns true if this QScriptValue is of the primitive type String;
  otherwise returns false.

  \sa toString()
*/
bool QScriptValue::isString() const
{
    return QScriptValuePrivate::valueOf(*this).isString();
}

/*!
  Returns true if this QScriptValue is a function; otherwise returns
  false.

  \sa call()
*/
bool QScriptValue::isFunction() const
{
    return QScriptValuePrivate::valueOf(*this).isFunction();
}

/*!
  Returns true if this QScriptValue is of the primitive type Null;
  otherwise returns false.

  \sa QScriptEngine::nullScriptValue()
*/
bool QScriptValue::isNull() const
{
    return QScriptValuePrivate::valueOf(*this).isNull();
}

/*!
  Returns true if this QScriptValue is of the primitive type Undefined;
  otherwise returns false.

  \sa QScriptEngine::undefinedScriptValue()
*/
bool QScriptValue::isUndefined() const
{
    return QScriptValuePrivate::valueOf(*this).isUndefined();
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
    return QScriptValuePrivate::valueOf(*this).isObject();
}

/*!
  Returns true if this QScriptValue is a variant value;
  otherwise returns false.

  \sa toVariant(), QScriptEngine::newVariant()
*/
bool QScriptValue::isVariant() const
{
    return QScriptValuePrivate::valueOf(*this).isVariant();
}

/*!
  Returns true if this QScriptValue is a QObject; otherwise returns
  false.

  \sa toQObject(), QScriptEngine::newQObject()
*/
bool QScriptValue::isQObject() const
{
    return QScriptValuePrivate::valueOf(*this).isQObject();
}

/*!
  Returns true if this QScriptValue is valid; otherwise returns
  false.
*/
bool QScriptValue::isValid() const
{
    return QScriptValuePrivate::valueOf(*this).isValid();
}
