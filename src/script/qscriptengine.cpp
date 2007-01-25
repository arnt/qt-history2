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
#include "qscriptengine_p.h"
#include "qscriptcontext_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmadate_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmaregexp_p.h"
#include "qscriptextvariant_p.h"
#include "qscriptextqobject_p.h"
#include "qscriptsyntaxchecker_p.h"

/*!
  \since 4.3
  \class QScriptEngine

  \brief The QScriptEngine class provides an environment for evaluating Qt Script code.

  \ingroup script
  \mainclass

  See the \l{QtScript} documentation for information about the Qt Script language,
  and how to get started with scripting your C++ application.

  Use evaluate() to evaluate script code.
  \code
    QScriptValue three = myEngine.evaluate("1 + 2");
  \endcode

  QScriptEngine acts as a QScriptValue factory. Use one of the
  scriptValue() or scriptValueFromT() functions to create a value. You
  can then pass the QScriptValue as argument to functions such as
  QScriptValue::setProperty() and QScriptValue::call().

  For QObjects (and subclasses), you can create a QScriptValue which
  represents a given object by calling
  newQObject(). Properties, children and signals and slots
  of the object will then become available to script code as
  properties of the created QScriptValue.  No binding code is needed
  because it is done dynamically using the Qt meta object system. See
  the \l{QtScript} documentation for more information.

  Typically, you set properties in the engine's Global Object to make
  your own extensions available to scripts; properties of the Global
  Object are accessible from any script code.

  Here is an example of how to expose a number value through the
  Global Object:

  \code
    QScriptValue myNumber = myEngine.scriptValue(123);
    myEngine.globalObject().setProperty("myNumber", myNumber);

    ...

    myNumber = myEngine.evaluate("myNumber + 1");
  \endcode

  In addition to exposing plain data, you can also write C++ functions
  that can be invoked from script code. Such functions must have the
  signature QScriptFunctionSignature. You may then pass the function
  as argument to scriptValue(). Here is an example:

  \code
    QScriptValue myAdd(QScriptContext *context, QScriptEngine *engine)
    {
       QScriptValue a = context->argument(0);
       QScriptValue b = context->argument(1);
       return engine->scriptValue(a.toNumber() + b.toNumber());
    }
  \endcode

  First, we get the current context; the context represents the
  current Qt Script function invocation. We use the context to
  retrieve the arguments passed to the function. We convert the
  arguments to C++ primitive number types, add them together, and
  return a QScriptValue that holds the sum.

  To expose this function to script code, you can set it as a property
  of the Global Object:

  \code
    QScriptValue fun = myEngine.scriptValue(myAdd);
    myEngine.globalObject().setProperty("myAdd", fun);
  \endcode

  Once this is done, script code can call your function in the exact
  same manner as a "normal" script function:

  \code
    QSscriptValue result = myEngine.evaluate("myAdd(myNumber, 1)");
  \endcode

  A different approach to writing and exposing (either generic or
  non-generic) native functions is by using a combination of
  newQObject() and QScriptable; see the documentation for
  QScriptable for details.

  You can extend the C++ types recognized by QScriptEngine by calling
  qScriptRegisterMetaType(). You provide the engine with functions
  that convert between a QScriptValue and the C++ type. Once the type
  has been registered, you can use scriptValueFromValue() and
  qscript_cast() to create and cast values in a convenient
  manner. Additionally, the engine will call the proper conversion
  functions when calling slots in QObjects from script code, and when
  getting or setting a property of that type in the QObject.

  \sa QScriptValue, QScriptContext

*/

#ifdef QT_NO_QOBJECT
QScriptEngine::QScriptEngine()
    : d_ptr(new QScriptEnginePrivate)
{
    d_ptr->q_ptr = this;
    d_ptr->init();
}

/*! \internal
*/
QScriptEngine::QScriptEngine(QScriptEnginePrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    d_ptr->init();
}
#else

/*!
    Constructs a QScriptEngine object.

    The Global Object is initialized to have properties as described in
    \l{ECMA-262}, Section 15.1.
*/
QScriptEngine::QScriptEngine()
    : QObject(*new QScriptEnginePrivate, 0)
{
    Q_D(QScriptEngine);
    d->init();
}

/*!
    Constructs a QScriptEngine object with the given \a parent.

    The Global Object is initialized to have properties as described in
    \l{ECMA-262}, Section 15.1.
*/

QScriptEngine::QScriptEngine(QObject *parent)
    : QObject(*new QScriptEnginePrivate, parent)
{
    Q_D(QScriptEngine);
    d->init();
}

/*! \internal
*/
QScriptEngine::QScriptEngine(QScriptEnginePrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    Q_D(QScriptEngine);
    d->init();
}
#endif

/*!
  Destroys this QScriptEngine.
*/
QScriptEngine::~QScriptEngine()
{
    Q_D(QScriptEngine);
    d->popContext();
#ifdef QT_NO_QOBJECT
    delete d_ptr;
    d_ptr = 0;
#endif
}

/*!
  \internal

  Returns a unique name identifier for the given \a name.
  The identifier can be used to access properties of a
  QScriptValue.

  \sa QScriptValue::property()
*/
QScriptNameId QScriptEngine::nameId(const QString &name)
{
    Q_D(QScriptEngine);
    return d->publicNameId(name);
}

/*!
  Returns this engine's Global Object.

  The Global Object contains the built-in objects that are part of
  \l{ECMA-262}, such as Math, Date and String. Additionally, you can set
  properties of the Global Object to make your own extensions
  available to all script code.
*/
QScriptValue QScriptEngine::globalObject() const
{
    Q_D(const QScriptEngine);
    return d->globalObject;
}

/*!
  Returns a QScriptValue of the primitive type Null.
*/
QScriptValue QScriptEngine::nullValue()
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newNull(&v);
    return v;
}

/*!
  Returns a QScriptValue of the primitive type Undefined.
*/
QScriptValue QScriptEngine::undefinedValue()
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newUndefined(&v);
    return v;
}

/*!
  \internal
*/
QScriptValue QScriptEngine::nullScriptValue()
{
    return nullValue();
}

/*!
  \internal
*/
QScriptValue QScriptEngine::undefinedScriptValue()
{
    return undefinedValue();
}

/*!
  Returns a QScriptValue of the primitive type Boolean
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(bool value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newBoolean(&v, value);
    return v;
}

/*!
  Returns a QScriptValue of the primitive type Number
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(qsreal value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newNumber(&v, value);
    return v;
}

/*!
  Returns a QScriptValue of the primitive type Number
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(int value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newNumber(&v, value);
    return v;
}

/*!
  Returns a QScriptValue of the primitive type Number
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(uint value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newNumber(&v, value);
    return v;
}

/*!
  Returns a QScriptValue of the primitive type Number
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(qlonglong value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newNumber(&v, value);
    return v;
}

/*!
  Returns a QScriptValue of the primitive type Number
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(qulonglong value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
#if defined(Q_OS_WIN) && _MSC_FULL_VER <= 12008804
#pragma message("** NOTE: You need the Visual Studio Processor Pack to compile support for 64bit unsigned integers.")
    d->newNumber(&v, (qlonglong)value);
#else
    d->newNumber(&v, value);
#endif
    return v;
}

/*!
  Returns a QScriptValue of the primitive type String
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(const QString &value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newNameId(&v, value);
    return v;
}

#ifndef QT_NO_CAST_FROM_ASCII
/*!
  Returns a QScriptValue of the primitive type String
  with the given \a value.
*/
QScriptValue QScriptEngine::scriptValue(const char *value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newNameId(&v, QLatin1String(value));
    return v;
}
#endif

/*!
  Creates a QScriptValue that wraps a native (C++) function. \a fun
  must be a C++ function with signature QScriptFunctionSignature.  \a
  length is the number of arguments that \a fun expects; this becomes
  the \c{length} property of the created QScriptValue.

  Note that \a length only gives an indication of the number of
  arguments that the function expects; an actual invocation of a
  function can include any number of arguments. You can check the
  \l{QScriptContext::argumentCount()}{argumentCount()} of the
  QScriptContext associated with the invocation to determine the
  actual number of arguments passed.

  \sa QScriptValue::call()
*/
QScriptValue QScriptEngine::newFunction(QScriptFunctionSignature fun,
                                        int length)
{
    Q_D(QScriptEngine);
    QScriptValue v = d->createFunction(new QScript::CFunction(fun, length));
    return v;
}

/*!
  \internal
*/
QScriptValue QScriptEngine::scriptValue(QScriptFunctionSignature fun,
                                        int length)
{
    return newFunction(fun, length);
}

/*!
  Creates a constructor function from \a fun, with the given \a length.
  The \c{prototype} property of the resulting function is set to be the
  given \a prototype. The \c{constructor} property of \a prototype is
  set to be the resulting function.
*/
QScriptValue QScriptEngine::newFunction(QScriptFunctionSignature fun,
                                        const QScriptValue &prototype,
                                        int length)
{
    Q_D(QScriptEngine);
    QScriptValue v = d->createFunction(new QScript::CFunction(fun, length));
    v.setProperty(d->idTable()->id_prototype, prototype);
    QScriptValueImpl::get(prototype)->setProperty(d->idTable()->id_constructor, v);
    return v;
}

/*!
  \internal
*/
QScriptValue QScriptEngine::scriptValue(QScriptFunctionSignature fun,
                                        const QScriptValue &prototype,
                                        int length)
{
    return newFunction(fun, prototype, length);
}

#ifndef QT_NO_REGEXP
/*!
  Creates a QScriptValue object of class RegExp with the given
  \a regexp.
*/
QScriptValue QScriptEngine::newRegExp(const QRegExp &regexp)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->regexpConstructor->newRegExp(&v, regexp);
    return v;
}

/*!
  \internal
*/
QScriptValue QScriptEngine::scriptValue(const QRegExp &regexp)
{
    return newRegExp(regexp);
}
#endif // QT_NO_REGEXP

/*!
  Returns a QScriptValue holding the given variant \a value.

  If a default prototype has been registered with the meta type id of
  \a value, then the prototype of the created object will be that
  prototype; otherwise, the prototype will be the Object prototype
  object.

  \sa setDefaultPrototype()
*/
QScriptValue QScriptEngine::newVariant(const QVariant &value)
{
    Q_D(QScriptEngine);
    Q_ASSERT(d->variantConstructor != 0);

    QScriptValue v;
    d->variantConstructor->newVariant(&v, value);
    QScriptValue proto = defaultPrototype(value.userType());
    if (proto.isValid())
        v.setPrototype(proto);
    return v;
}

/*!
  \internal
*/
QScriptValue QScriptEngine::scriptValueFromVariant(const QVariant &value)
{
    return newVariant(value);
}

#ifndef QT_NO_QOBJECT
/*!
  Returns a QScriptValue that wraps the given QObject \a object.

  Signals and slots, properties and children of \a object are
  available as properties of the created QScriptValue. For more
  information, see the \l{QtScript} documentation.

  The engine does not take ownership of \a object.
*/
QScriptValue QScriptEngine::newQObject(QObject *object)
{
    Q_D(QScriptEngine);
    Q_ASSERT(d->qobjectConstructor != 0);

    QScriptValue v;
    d->qobjectConstructor->newQObject(&v, object);

    if (object) {
        // see if we have a default prototype
        QByteArray typeString = object->metaObject()->className();
        typeString.append('*');
        int typeId = QMetaType::type(typeString);
        if (typeId != 0) {
            QScriptValue proto = defaultPrototype(typeId);
            if (proto.isValid())
                v.setPrototype(proto);
        }
    }

    return v;
}

/*!
  \internal
*/
QScriptValue QScriptEngine::scriptValueFromQObject(QObject *object)
{
    return newQObject(object);
}
#endif // QT_NO_QOBJECT

/*!
  Creates a QScriptValue object of class Object.

  The prototype of the created object will be the Object
  prototype object.
*/
QScriptValue QScriptEngine::newObject()
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newObject(&v, d->objectConstructor->publicPrototype);
    return v;
}

/*!
  Creates a QScriptValue object of class Array with the given \a length.
*/
QScriptValue QScriptEngine::newArray(uint length)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    QScript::Array a;
    a.resize(length);
    d->newArray(&v, a);
    return v;
}

/*!
  Creates a QScriptValue object of class RegExp with the given
  \a pattern and \a flags.
*/
QScriptValue QScriptEngine::newRegExp(const QString &pattern, const QString &flags)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->regexpConstructor->newRegExp(&v, pattern, flags);
    return v;
}

/*!
  Creates a QScriptValue object of class Date with the given
  \a value (the number of milliseconds since 01 January 1970,
  UTC).
*/
QScriptValue QScriptEngine::newDate(qsreal value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->dateConstructor->newDate(&v, value);
    return v;
}

/*!
  Creates a QScriptValue object of class Date from the given
  \a value.
*/
QScriptValue QScriptEngine::newDate(const QDateTime &value)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->dateConstructor->newDate(&v, value);
    return v;
}

#ifndef QT_NO_QOBJECT
/*!
  Creates a QScriptValue that represents a QObject class, using the
  the given \a metaObject and constructor \a ctor.

  Enums of \a metaObject are available as properties of the created
  QScriptValue. When the class is called as a function, \a ctor will
  be called to create a new instance of the class.
*/
QScriptValue QScriptEngine::newQMetaObject(
    const QMetaObject *metaObject, const QScriptValue &ctor)
{
    Q_D(QScriptEngine);
    QScriptValue v;
    d->newFunction(&v, new QScript::ExtQClass(metaObject, ctor));
    v.setPrototype(ctor); // ###
    QScriptValueImpl::get(v)->setClassInfo(d->m_class_qclass);
    return v;
}

QScriptValue QScriptEngine::scriptValue(
    const QMetaObject *metaObject, const QScriptValue &ctor)
{
    return newQMetaObject(metaObject, ctor);
}

/*!
  \fn QScriptValue QScriptEngine::scriptValueFromQClass()

  Creates a QScriptValue that represents the Qt class \c{T}.

  This function is used in combination with one of the
  Q_SCRIPT_DECLARE_QCLASS() macro. Example:

  \code
  Q_SCRIPT_DECLARE_QCLASS(QLineEdit, QWidget*)

  ...

  QScriptValue lineEditClass = engine.scriptValueFromQClass<QLineEdit>();
  engine.globalObject().setProperty("QLineEdit", lineEditClass);
  \endcode

  \warning This function is not available with MSVC 6. Use
  qScriptValueFromQClass() instead if you need to support that version
  of the compiler.

*/

/*!
  \fn QScriptValue qScriptValueFromQClass(QScriptEngine *engine)
  \relates QScriptEngine

  Uses \a engine to create a QScriptValue that represents the Qt class
  \c{T}.

  This function is equivalent to
  QScriptEngine::scriptValueFromQClass(). It is provided as a
  work-around for MSVC 6, which doesn't support member template
  functions.
*/
#endif // QT_NO_QOBJECT

/*!
  Returns true if \a program can be evaluated (is syntactically
  complete); otherwise returns false.

  \sa evaluate()
*/
bool QScriptEngine::canEvaluate(const QString &program) const
{
    QScript::SyntaxChecker checker;
    return checker.parse(program);
}

/*!
  Evaluates \a program and returns the result of the evaluation.

  The script code will be evaluated in the current context.

  \sa canEvaluate(), hasUncaughtException()
*/
QScriptValue QScriptEngine::evaluate(const QString &program, int lineNumber)
{
    Q_D(QScriptEngine);
    QScriptContext *ctx = d->context();
    d->evaluate(ctx, program, lineNumber);
    return QScriptContextPrivate::get(ctx)->result;
}

/*!
  Evaluates \a program and returns the result of the evaluation.

  The script code will be evaluated in the current context.

  \sa canEvaluate(), hasUncaughtException()
*/
QScriptValue QScriptEngine::evaluate(const QString &program)
{
    Q_D(QScriptEngine);
    QScriptContextPrivate *ctx = QScriptContextPrivate::get(d->context());
    return evaluate(program, ctx->currentLine);
}

/*!
  Returns the current context.

  The current context is typically accessed to retrieve the arguments
  to native functions.
*/
QScriptContext *QScriptEngine::currentContext() const
{
    Q_D(const QScriptEngine);
    return d->context();
}

/*!
  Returns true if the last invocation of evaluate() resulted in an
  uncaught exception; otherwise returns false.

  \sa uncaughtExceptionLineNumber()
*/
bool QScriptEngine::hasUncaughtException() const
{
    return (currentContext()->state() == QScriptContext::Exception);
}

/*!
  Returns the line number where the last uncaught exception occurred.

  \sa hasUncaughtException()
*/
int QScriptEngine::uncaughtExceptionLineNumber() const
{
    return QScriptContextPrivate::get(currentContext())->errorLineNumber;
}

/*!
  Returns the default prototype associated with the given \a metaTypeId,
  or an invalid QScriptValue if no default prototype has been set.

  \sa setDefaultPrototype(), scriptValueFromVariant()
*/
QScriptValue QScriptEngine::defaultPrototype(int metaTypeId) const
{
    Q_D(const QScriptEngine);
    QScriptCustomTypeInfo info = d->m_customTypes.value(metaTypeId);
    return info.prototype;
}

/*!
  Sets the default prototype of the given \a metaTypeId to \a prototype.

  \sa scriptValueFromVariant(), qScriptRegisterMetaType()
*/
void QScriptEngine::setDefaultPrototype(int metaTypeId, const QScriptValue &prototype)
{
    Q_D(QScriptEngine);
    QScriptCustomTypeInfo info = d->m_customTypes.value(metaTypeId);
    info.prototype.deref();
    info.prototype = prototype;
    prototype.ref();
    d->m_customTypes.insert(metaTypeId, info);
}

/*!
    \typedef QScriptFunctionSignature
    \relates QScriptEngine

    The function signature \c{QScriptValue f(QScriptContext *, QScriptEngine *)}.

    A function with such a signature can be passed to QScriptEngine::scriptValue()
    to wrap the function.
*/

/*!
    \typedef QScriptEngine::MarshalFunction
    \internal
*/

/*!
    \typedef QScriptEngine::DemarshalFunction
    \internal
*/

/*!
    \internal
*/
QScriptValue QScriptEngine::create(int type, const void *ptr)
{
    Q_D(QScriptEngine);
    return d->create(type, ptr);
}

/*!
    \internal
*/
bool QScriptEngine::convert(const QScriptValue &value, int type, void *ptr)
{
    Q_D(QScriptEngine);
    return d->convert(value, type, ptr);
}

/*!
    \internal
*/
void QScriptEngine::registerCustomType(int type, MarshalFunction mf,
                                       DemarshalFunction df,
                                       const QScriptValue &prototype)
{
    Q_D(QScriptEngine);
    QScriptCustomTypeInfo info = d->m_customTypes.value(type);
    info.prototype.deref();
    info.marshal = mf;
    info.demarshal = df;
    info.prototype = prototype;
    prototype.ref();
    d->m_customTypes.insert(type, info);
}

/*! \fn QScriptValue QScriptEngine::scriptValueFromValue(const T &value)

    Creates a QScriptValue with the given \a value.

    \warning This function is not available with MSVC 6. Use
    qScriptValueFromValue() instead if you need to support that
    version of the compiler.

    \sa qScriptRegisterMetaType(), qscript_cast()
*/

/*!
    \fn QScriptValue qScriptValueFromValue(QScriptEngine *engine, const T &value)
    \relates QScriptEngine

    Creates a QScriptValue using the given \a engine with the given \a
    value of template type \c{T}.

    This function is equivalent to
    QScriptEngine::scriptValueFromValue(\a value). It is provided as a
    work-around for MSVC 6, which doesn't support member template
    functions.

    \sa qscript_cast()
*/

/*! \fn T qscript_cast(const QScriptValue &value)
    \relates QScriptValue

    Returns the given \a value converted to the template type \c{T}.

    \sa qScriptRegisterMetaType(), QScriptEngine::scriptValueFromValue()
*/

/*! \fn int qScriptRegisterMetaType(
            QScriptEngine *engine,
            QScriptValue (*toValue)(QScriptEngine *, const T &t),
            void (*fromValue)(const QScriptValue &, T &t),
            const QScriptValue &prototype = QScriptValue::invalid())
    \relates QScriptEngine

    Registers the type \c{T} in the given \a engine. \a toValue must
    be a function that will convert from a value of type \c{T} to a
    QScriptValue, and \a fromValue a function that does the
    opposite. \a prototype, if valid, is the prototype that's set on
    QScriptValues returned by \a toValue.

    Returns the internal ID used by QMetaType.

    You need to declare the custom type first with
    Q_DECLARE_METATYPE().

    After a type has been registered, you can cast from a QScriptValue
    to that type using qscript_cast(), and create a QScriptValue from
    a value of that type using
    \l{QScriptEngine::scriptValueFromValue()}{scriptValueFromValue}(). The
    engine will take care of calling the proper conversion function
    when calling C++ slots, and when getting or setting a C++
    property; i.e. the custom type may be used seamlessly on both the
    C++ side and the script side.

    The following is an example of how to use this function. We will
    make our engine able to handle our custom type
    \c{MyStruct}. Here's the C++ type:

    \code
      struct MyStruct {
        int x;
        int y;
      };

    \endcode

    We must declare it so that the type will be known to QMetaType:

    \code
      Q_DECLARE_METATYPE(MyStruct)
    \endcode

    Here are the \c{MyStruct} conversion functions:

    \code
    QScriptValue toScriptValue(QScriptEngine *engine, const MyStruct &s)
    {
      QScriptValue obj = engine->newObject();
      obj.setProperty("x", engine->scriptValue(s.x));
      obj.setProperty("y", engine->scriptValue(s.y));
      return obj;
    }

    void fromScriptValue(const QScriptValue &obj, MyStruct &s)
    {
      s.x = obj.property("x").toInt32();
      s.y = obj.property("y").toInt32();
    }
    \endcode

    Now we can register \c{MyStruct} with the engine:
    \code
    qScriptRegisterMetaType(engine, toScriptValue, fromScriptValue);
    \endcode

    Working with \c{MyStruct} values is now easy:
    \code
    MyStruct s = qscript_cast<MyStruct>(context->argument(0));

    ...

    MyStruct s2;
    s2.x = s.x + 10;
    s2.y = s.y + 20;
    return engine->scriptValueFromValue(s2);
    \endcode

    If you want to construct values of your custom type from script code,
    you have to register a constructor function for the type. For example:

    \code
    QScriptValue createMyStruct(QScriptContext *, QScriptEngine *eng)
    {
        MyStruct s;
        s.x = 123;
        s.y = 456;
        return qScriptValueFromValue(eng, s);
    }

    ...

    QScriptValue ctor = engine.scriptValue(createMyStruct);
    engine.globalObject().setProperty("MyStruct", ctor);
    \endcode

    \sa QScriptEngine::scriptValueFromVariant(), qRegisterMetaType()
*/

/*!
    \macro Q_SCRIPT_DECLARE_QCLASS(QClass, ArgType)
    \relates QScriptEngine

    Declares the given \a QClass. Used in combination with
    QScriptEngine::scriptValueFromQClass() to make enums and
    instantiation of \a QClass available to script code. The
    constructor generated by this macro takes a single argument of
    type \a ArgType; typically the argument is the parent type of the
    new instance, in which case \a ArgType is \c{QWidget*} or
    \c{QObject*}.
*/
