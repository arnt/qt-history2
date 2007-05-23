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

#ifndef QT_NO_SCRIPT

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
  has built-in support for QVariant, QObject and QMetaObject.

  For the object-based types (including Date and RegExp), use the
  newT() functions in QScriptEngine (e.g. QScriptEngine::newObject())
  to create a QScriptValue of the desired type. For the primitive types,
  use one of the QScriptValue constructor overloads.

  The methods named isT() (e.g. isBoolean(), isUndefined()) can be
  used to test if a value is of a certain type. The methods named
  toT() (e.g. toBoolean(), toString()) can be used to convert a
  QScriptValue to another type. You can also use the generic
  qscriptvalue_cast() function.

  Object values have zero or more properties which are themselves
  QScriptValues. Use setProperty() to set a property of an object, and
  call property() to retrieve the value of a property.

  Note that a QScriptValue for which isObject() is true only carries a
  reference to an actual object; copying the QScriptValue will only
  copy the object reference, not the object itself. If you want to
  clone an object (i.e. copy an object's properties to another
  object), you can do so with the help of a \c{for-in} statement in
  script code, or QScriptValueIterator in C++.

  Object values have an internal \c{prototype} property, which can be
  accessed with prototype() and setPrototype(). Properties added to a
  prototype are shared by all objects having that prototype; this is
  referred to as prototype-based inheritance. For more information,
  see the \l{QtScript} documentation.

  Function objects (objects for which isFunction() returns true) can
  be invoked by calling call(). Constructor functions can be used to
  construct new objects by calling construct().

  Use equals(), strictlyEquals() and lessThan() to compare a QScriptValue
  to another.

  \sa QScriptEngine, QScriptValueIterator
*/

/*!
    \enum QScriptValue::SpecialValue

    This enum is used to specify a single-valued type.

    \value UndefinedValue An undefined value.

    \value NullValue A null value.
*/

/*!
    \enum QScriptValue::PropertyFlag

    This enum describes the attributes of a property.

    \value ReadOnly The property is read-only. Attempts by Qt Script code to write to the property will be ignored.

    \value Undeletable Attempts by Qt Script code to \c{delete} the property will be ignored.

    \value SkipInEnumeration The property is not to be enumerated by a \c{for-in} enumeration.

    \value PropertyGetter The property is defined by a function which will be called to get the property value.

    \value PropertySetter The property is defined by a function which will be called to set the property value.

    \value QObjectMember This flag is used to indicate that an existing property is a QObject member (a property or method).

    \value KeepExistingFlags This value is used to indicate to setProperty() that the property's flags should be left unchanged. If the property doesn't exist, the default flags (0) will be used.

    \value UserRange Flags in this range are not used by Qt Script, and can be used for custom purposes.
*/

/*!
    \enum QScriptValue::ResolveFlag

    This enum specifies how to look up a property of an object.

    \value ResolveLocal Only check the object's own properties.

    \value ResolvePrototype Check the object's own properties first, then search the prototype chain. This is the default.

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

  Note that if \a other is an object (i.e., isObject() would return
  true), then only a reference to the underlying object is copied into
  the new script value (i.e., the object itself is not copied).
*/
QScriptValue::QScriptValue(const QScriptValue &other)
    : d_ptr(other.d_ptr)
{
    if (d_ptr)
        d_ptr->ref.ref();
}

/*!
  Constructs a new QScriptValue with the special \a value and
  registers it with the script \a engine.
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
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, bool value)

  Constructs a new QScriptValue with the boolean \a value and
  registers it with the script \a engine.
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
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, int value)

  Constructs a new QScriptValue with the integer \a value and
  registers it with the script \a engine.
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
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, uint value)

  Constructs a new QScriptValue with the unsigned integer \a value and
  registers it with the script \a engine.
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
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, qsreal value)

  Constructs a new QScriptValue with the qsreal \a value and
  registers it with the script \a engine.
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
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, const QString &value)

  Constructs a new QScriptValue with the string \a value and
  registers it with the script \a engine.
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

  Constructs a new QScriptValue with the string \a value and
  registers it with the script \a engine.
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
  Assigns the \a other value to this QScriptValue.

  Note that if \a other is an object (isObject() returns true),
  only a reference to the underlying object will be assigned;
  the object itself will not be copied.
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

  The internal prototype should not be confused with the public
  property with name "prototype"; the public prototype is usually
  only set on functions that act as constructors.

  \sa prototype(), isObject()
*/
void QScriptValue::setPrototype(const QScriptValue &prototype)
{
    if (!isObject())
        return;
    if (prototype.isValid() && (prototype.engine() != engine())) {
        qWarning("QScriptValue::setPrototype() failed: "
                 "cannot set a prototype created in "
                 "a different engine");
        return;
    }
    QScriptValueImpl self = QScriptValuePrivate::valueOf(*this);
    QScriptValueImpl was = self.prototype();
    self.setPrototype(QScriptValuePrivate::valueOf(prototype));
    if (self.detectedCycle()) {
        qWarning("QScriptValue::setPrototype() failed: "
                 "cyclic prototype value");
        self.setPrototype(was);
    }
}

/*!
  \internal
*/
QScriptValue QScriptValue::scope() const
{
    if (!isObject())
        return QScriptValue();
    return QScriptValuePrivate::valueOf(*this).scope();
}

/*!
  \internal
*/
void QScriptValue::setScope(const QScriptValue &scope)
{
    if (!isObject())
        return;
    if (scope.isValid() && (scope.engine() != engine())) {
        qWarning("QScriptValue::setScope() failed: "
                 "cannot set a scope object created in "
                 "a different engine");
        return;
    }
    QScriptValuePrivate::valueOf(*this).setScope(QScriptValuePrivate::valueOf(scope));
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

  Note that if this QScriptValue or the \a other value are objects,
  calling this function has side effects on the script engine, since
  the engine will call the object's valueOf() function (and possibly
  toString()) in an attempt to convert the object to a primitive value
  (possibly resulting in an uncaught script exception).

  \sa equals()
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

  Note that if this QScriptValue or the \a other value are objects,
  calling this function has side effects on the script engine, since
  the engine will call the object's valueOf() function (and possibly
  toString()) in an attempt to convert the object to a primitive value
  (possibly resulting in an uncaught script exception).

  \sa strictlyEquals(), lessThan()
*/
bool QScriptValue::equals(const QScriptValue &other) const
{
    if (isValid() && other.isValid() && (other.engine() != engine())) {
        qWarning("QScriptValue::equals: "
                 "cannot compare to a value created in "
                 "a different engine");
        return false;
    }
    return QScriptValuePrivate::valueOf(*this).equals(QScriptValuePrivate::valueOf(other));
}

/*!
  Returns true if this QScriptValue is equal to \a other using strict
  comparison (no conversion), otherwise returns false. The comparison
  follows the behavior described in \l{ECMA-262} section 11.9.6, "The
  Strict Equality Comparison Algorithm".

  \sa equals()
*/
bool QScriptValue::strictlyEquals(const QScriptValue &other) const
{
    if (isValid() && other.isValid() && (other.engine() != engine())) {
        qWarning("QScriptValue::strictlyEquals: "
                 "cannot compare to a value created in "
                 "a different engine");
        return false;
    }
    return QScriptValuePrivate::valueOf(*this).strictlyEquals(QScriptValuePrivate::valueOf(other));
}

/*!
  Returns the string value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.8, "ToString".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's toString() function (and possibly valueOf()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isString()
*/
QString QScriptValue::toString() const
{
    return QScriptValuePrivate::valueOf(*this).toString();
}

/*!
  Returns the number value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.3, "ToNumber".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isNumber(), toInteger(), toInt32(), toUInt32(), toUInt16()
*/
qsreal QScriptValue::toNumber() const
{
    return QScriptValuePrivate::valueOf(*this).toNumber();
}

/*!
  Returns the boolean value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.2, "ToBoolean".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isBoolean()
*/
bool QScriptValue::toBoolean() const
{
    return QScriptValuePrivate::valueOf(*this).toBoolean();
}

/*!
  Returns the signed 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.5, "ToInt32".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber(), toUInt32()
*/
qint32 QScriptValue::toInt32() const
{
    return QScriptValuePrivate::valueOf(*this).toInt32();
}

/*!
  Returns the unsigned 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.6, "ToUint32".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber(), toInt32()
*/
quint32 QScriptValue::toUInt32() const
{
    return QScriptValuePrivate::valueOf(*this).toUInt32();
}

/*!
  Returns the unsigned 16-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.7, "ToUint16".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber()
*/
quint16 QScriptValue::toUInt16() const
{
    return QScriptValuePrivate::valueOf(*this).toUInt16();
}

/*!
  Returns the integer value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.4, "ToInteger".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber()
*/
qsreal QScriptValue::toInteger() const
{
    return QScriptValuePrivate::valueOf(*this).toInteger();
}

/*!
  Returns the QVariant value of this QScriptValue, if it can be
  converted to a QVariant; otherwise returns an invalid QVariant.
  The conversion is performed according to the following table:

    \table
    \header \o Input Type \o Result
    \row    \o Undefined  \o An invalid QVariant.
    \row    \o Null       \o An invalid QVariant.
    \row    \o Boolean    \o A QVariant containing the value of the boolean.
    \row    \o Number     \o A QVariant containing the value of the number.
    \row    \o String     \o A QVariant containing the value of the string.
    \row    \o QVariant Object \o The result is the QVariant value of the object (no conversion).
    \row    \o QObject Object \o A QVariant containing a pointer to the QObject.
    \row    \o Date Object \o A QVariant containing the date value (toDateTime()).
    \row    \o RegExp Object \o A QVariant containing the regular expression value (toRegExp()).
    \row    \o Object     \o If the value is primitive, then the result is converted to a QVariant according to the above rules; otherwise, an invalid QVariant is returned.
    \endtable

  \sa isVariant()
*/
QVariant QScriptValue::toVariant() const
{
    return QScriptValuePrivate::valueOf(*this).toVariant();
}

/*!
  Returns the object value of this QScriptValue, if it can be
  converted to an object; otherwise returns an invalid
  QScriptValue. The conversion is performed according to the following
  table:

    \table
    \header \o Input Type \o Result
    \row    \o Undefined  \o An invalid QScriptValue.
    \row    \o Null       \o An invalid QScriptValue.
    \row    \o Boolean    \o A new Boolean object whose internal value is set to the value of the boolean.
    \row    \o Number     \o A new Number object whose internal value is set to the value of the number.
    \row    \o String     \o A new String object whose internal value is set to the value of the string.
    \row    \o Object     \o The result is the object itself (no conversion).
    \endtable

    \sa isObject(), QScriptEngine::newObject()
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
  If this QScriptValue is a QObject, returns the QObject pointer
  that the QScriptValue represents; otherwise, returns 0.

  If the QObject that this QScriptValue wraps has been deleted,
  this function returns 0 (i.e. it is possible for toQObject()
  to return 0 even when isQObject() returns true).

  \sa isQObject()
*/
QObject *QScriptValue::toQObject() const
{
    return QScriptValuePrivate::valueOf(*this).toQObject();
}

/*!
  If this QScriptValue is a QMetaObject, returns the QMetaObject pointer
  that the QScriptValue represents; otherwise, returns 0.

  \sa isQMetaObject()
*/
const QMetaObject *QScriptValue::toQMetaObject() const
{
    return QScriptValuePrivate::valueOf(*this).toQMetaObject();
}

/*!
  Sets the value of this QScriptValue's property with the given \a name to
  the given \a value.

  If this QScriptValue is not an object, this function does nothing.

  If this QScriptValue does not already have a property with name \a name,
  a new property is created; the given \a flags then specify how this
  property may be accessed by script code.

  If \a value is invalid, the property is removed.

  If the property is implemented using a setter function (i.e. has the
  PropertySetter flag set), calling setProperty() has side-effects on
  the script engine, since the setter function will be called with the
  given \a value as argument (possibly resulting in an uncaught script
  exception).

  Note that you cannot specify custom getter or setter functions for
  built-in properties, such as the \c{length} property of Array objects
  or meta properties of QObject objects.

  \sa property()
*/
void QScriptValue::setProperty(const QString &name, const QScriptValue &value,
                               const PropertyFlags &flags)
{
    if (isValid() && value.isValid() && (value.engine() != engine())) {
        qWarning("QScriptValue::setProperty(%s) failed: "
                 "cannot set value created in a different engine",
                 qPrintable(name));
        return;
    }
    QScriptValuePrivate::valueOf(*this).setProperty(name, QScriptValuePrivate::valueOf(value), flags);
}

/*!
  Returns the value of this QScriptValue's property with the given \a name,
  using the given \a mode to resolve the property.

  If no such property exists, an invalid QScriptValue is returned.

  If the property is implemented using a getter function (i.e. has the
  PropertyGetter flag set), calling property() has side-effects on the
  script engine, since the getter function will be called (possibly
  resulting in an uncaught script exception). If an exception
  occurred, property() returns the value that was thrown (typically
  an \c{Error} object).

  \sa setProperty(), propertyFlags()
*/
QScriptValue QScriptValue::property(const QString &name,
                                    const ResolveFlags &mode) const
{
    return QScriptValuePrivate::valueOf(*this).property(name, mode);
}

/*!
  \overload

  Returns the property at the given \a arrayIndex, using the given \a
  mode to resolve the property.

  This function is provided for convenience and performance when
  working with array objects.

  If this QScriptValue is not an Array object, this function behaves
  as if property() was called with the string representation of \a
  arrayIndex.
*/
QScriptValue QScriptValue::property(quint32 arrayIndex,
                                    const ResolveFlags &mode) const
{
    return QScriptValuePrivate::valueOf(*this).property(arrayIndex, mode);
}

/*!
  \overload

  Sets the property at the given \a arrayIndex to the given \a value.

  This function is provided for convenience and performance when
  working with array objects.

  If this QScriptValue is not an Array object, this function behaves
  as if setProperty() was called with the string representation of \a
  arrayIndex.
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
  Returns the flags of the property with the given \a name, using the
  given \a mode to resolve the property.

  \sa property()
*/
QScriptValue::PropertyFlags QScriptValue::propertyFlags(const QString &name,
                                                        const ResolveFlags &mode) const
{
    return QScriptValuePrivate::valueOf(*this).propertyFlags(name, mode);
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

  Calling call() can cause an exception to occur in the script engine;
  in that case, call() returns the value that was thrown (typically an
  \c{Error} object). You can call
  QScriptEngine::hasUncaughtException() to determine if an exception
  occurred.

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
  Calls this QScriptValue as a function, using \a thisObject as
  the `this' object in the function call, and passing \a arguments
  as arguments to the function. Returns the value returned from
  the function.

  If this QScriptValue is not a function, call() does nothing
  and returns an invalid QScriptValue.

  \a arguments can be an arguments object, an array, null or
  undefined; any other type will cause a TypeError to be thrown.

  Note that if \a thisObject is not an object, the global object
  (see \l{QScriptEngine::globalObject()}) will be used as the
  `this' object.

  \sa construct(), QScriptContext::argumentsObject()
*/
QScriptValue QScriptValue::call(const QScriptValue &thisObject,
                                const QScriptValue &arguments)
{
    if (isFunction() && thisObject.isValid() && (thisObject.engine() != engine())) {
        qWarning("QScriptValue::call() failed: "
                 "cannot call function with thisObject created in "
                 "a different engine");
        return QScriptValue();
    }
    return QScriptValuePrivate::valueOf(*this).call(QScriptValuePrivate::valueOf(thisObject),
                                                    QScriptValuePrivate::valueOf(arguments));
}

/*!
  Creates a new \c{Object} and calls this QScriptValue as a constructor,
  using the created object as the `this' object and passing \a args
  as arguments. If the return value from the constructor call is an
  object, then that object is returned; otherwise the created object
  is returned.

  If this QScriptValue is not a function, construct() does nothing
  and returns an invalid QScriptValue.

  \a args can be an arguments object, an array, null or
  undefined; any other type will cause a TypeError to be thrown.

  Calling construct() can cause an exception to occur in the script
  engine; in that case, construct() returns the value that was thrown
  (typically an \c{Error} object). You can call
  QScriptEngine::hasUncaughtException() to determine if an exception
  occurred.

  \sa call(), QScriptEngine::newObject()
*/
QScriptValue QScriptValue::construct(const QScriptValueList &args)
{
    return QScriptValuePrivate::valueOf(*this).construct(QScriptValuePrivate::toImplList(args));
}

/*!
  Creates a new \c{Object} and calls this QScriptValue as a constructor,
  using the created object as the `this' object and passing \a arguments
  as arguments. If the return value from the constructor call is an
  object, then that object is returned; otherwise the created object
  is returned.

  If this QScriptValue is not a function, construct() does nothing
  and returns an invalid QScriptValue.

  \a arguments can be an arguments object, an array, null or
  undefined. Any other type will cause a TypeError to be thrown.

  \sa call(), QScriptEngine::newObject(), QScriptContext::argumentsObject()
*/
QScriptValue QScriptValue::construct(const QScriptValue &arguments)
{
    return QScriptValuePrivate::valueOf(*this).construct(QScriptValuePrivate::valueOf(arguments));
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

  \sa QScriptEngine::nullValue()
*/
bool QScriptValue::isNull() const
{
    return QScriptValuePrivate::valueOf(*this).isNull();
}

/*!
  Returns true if this QScriptValue is of the primitive type Undefined;
  otherwise returns false.

  \sa QScriptEngine::undefinedValue()
*/
bool QScriptValue::isUndefined() const
{
    return QScriptValuePrivate::valueOf(*this).isUndefined();
}

/*!
  Returns true if this QScriptValue is of the Object type; otherwise
  returns false.

  Note that function values, variant values, and QObject values are
  objects, so this function returns true for such values.

  \sa toObject(), QScriptEngine::newObject()
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

  Note: This function returns true even if the QObject that this
  QScriptValue wraps has been deleted.

  \sa toQObject(), QScriptEngine::newQObject()
*/
bool QScriptValue::isQObject() const
{
    return QScriptValuePrivate::valueOf(*this).isQObject();
}

/*!
  Returns true if this QScriptValue is a QMetaObject; otherwise returns
  false.

  \sa toQMetaObject(), QScriptEngine::newQMetaObject()
*/
bool QScriptValue::isQMetaObject() const
{
    return QScriptValuePrivate::valueOf(*this).isQMetaObject();
}

/*!
  Returns true if this QScriptValue is valid; otherwise returns
  false.
*/
bool QScriptValue::isValid() const
{
    return QScriptValuePrivate::valueOf(*this).isValid();
}

#endif // QT_NO_SCRIPT
