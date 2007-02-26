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
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"
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

  Use newObject() to create a standard Qt Script object. Use newDate()
  to create a \c{Date} object, and newRegExp() to create a \c{RegExp}
  object.  Use newQObject() to wrap a QObject (or subclass) pointer,
  and newQMetaObject() to wrap a QMetaObject. Use newVariant() to wrap
  a QVariant. Use newFunction() to wrap a native (C++) function.

  When wrapping a QObject pointer with newQObject(), properties,
  children and signals and slots of the object will then become
  available to script code as properties of the created QScriptValue.
  No binding code is needed because it is done dynamically using the
  Qt meta object system. See the \l{QtScript} documentation for more
  information.

  Typically, you set properties in the engine's Global Object to make
  your own extensions available to scripts; properties of the Global
  Object are accessible from any script code.

  Here is an example of how to expose a number value through the
  Global Object:

  \code
    QScriptValue myNumber = QScriptValue(&myEngine, 123);
    myEngine.globalObject().setProperty("myNumber", myNumber);

    ...

    QScriptValue myNumberPlusOne = myEngine.evaluate("myNumber + 1");
  \endcode

  In addition to exposing plain data, you can also write C++ functions
  that can be invoked from script code. Such functions must have the
  signature QScriptEngine::FunctionSignature. You may then pass the function
  as argument to newFunction(). Here is an example of a function that
  returns the sum of its first two arguments:

  \code
    QScriptValue myAdd(QScriptContext *context, QScriptEngine *engine)
    {
       QScriptValue a = context->argument(0);
       QScriptValue b = context->argument(1);
       return QScriptValue(engine, a.toNumber() + b.toNumber());
    }
  \endcode

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
  qScriptRegisterMetaType() or qScriptRegisterSequenceMetaType().

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
    d->objectAllocator.destruct();
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
    return d->m_globalObject;
}

/*!
  Returns a QScriptValue of the primitive type Null.
*/
QScriptValue QScriptEngine::nullValue()
{
    Q_D(QScriptEngine);
    return d->nullValue();
}

/*!
  Returns a QScriptValue of the primitive type Undefined.
*/
QScriptValue QScriptEngine::undefinedValue()
{
    Q_D(QScriptEngine);
    return d->undefinedValue();
}

/*!
  Creates a constructor function from \a fun, with the given \a length.
  The \c{prototype} property of the resulting function is set to be the
  given \a prototype. The \c{constructor} property of \a prototype is
  set to be the resulting function.
*/
QScriptValue QScriptEngine::newFunction(QScriptEngine::FunctionSignature fun,
                                        const QScriptValue &prototype,
                                        int length)
{
    Q_D(QScriptEngine);
    QScriptValueImpl v = d->createFunction(new QScript::CFunction(fun, length));
    QScriptValueImpl proto = QScriptValuePrivate::valueOf(prototype);
    v.setProperty(d->idTable()->id_prototype, proto);
    proto.setProperty(d->idTable()->id_constructor, v);
    return v;
}

#ifndef QT_NO_REGEXP
/*!
  Creates a QScriptValue object of class RegExp with the given
  \a regexp.
*/
QScriptValue QScriptEngine::newRegExp(const QRegExp &regexp)
{
    Q_D(QScriptEngine);
    QScriptValueImpl v;
    d->regexpConstructor->newRegExp(&v, regexp);
    return v;
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
    return d->newVariant(value);
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
    return d->newQObject(object);
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
    QScriptValueImpl v;
    d->newObject(&v, d->objectConstructor->publicPrototype);
    return v;
}

/*!
  Creates a QScriptValue that wraps a native (C++) function. \a fun
  must be a C++ function with signature QScriptEngine::FunctionSignature.  \a
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
QScriptValue QScriptEngine::newFunction(QScriptEngine::FunctionSignature fun, int length)
{
    Q_D(QScriptEngine);
    QScriptValueImpl v = d->createFunction(new QScript::CFunction(fun, length));
    QScriptValueImpl prototype = d->newObject();
    v.setProperty(d->idTable()->id_prototype, prototype);
    prototype.setProperty(d->idTable()->id_constructor, v);
    return v;
}


/*!
  Creates a QScriptValue object of class Array with the given \a length.
*/
QScriptValue QScriptEngine::newArray(uint length)
{
    Q_D(QScriptEngine);
    QScriptValueImpl v;
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
    QScriptValueImpl v;
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
    QScriptValueImpl v;
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
    QScriptValueImpl v;
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
    QScriptValueImpl v;
    d->qmetaObjectConstructor->newQMetaObject(&v, metaObject, QScriptValuePrivate::valueOf(ctor));
    return v;
}

/*!
  \fn QScriptValue QScriptEngine::scriptValueFromQMetaObject()

  Creates a QScriptValue that represents the Qt class \c{T}.

  This function is used in combination with one of the
  Q_SCRIPT_DECLARE_QMETAOBJECT() macro. Example:

  \code
  Q_SCRIPT_DECLARE_QMETAOBJECT(QLineEdit, QWidget*)

  ...

  QScriptValue lineEditClass = engine.scriptValueFromQMetaObject<QLineEdit>();
  engine.globalObject().setProperty("QLineEdit", lineEditClass);
  \endcode

  \warning This function is not available with MSVC 6. Use
  qScriptValueFromQMetaObject() instead if you need to support that version
  of the compiler.

*/

/*!
  \fn QScriptValue qScriptValueFromQMetaObject(QScriptEngine *engine)
  \relates QScriptEngine

  Uses \a engine to create a QScriptValue that represents the Qt class
  \c{T}.

  This function is equivalent to
  QScriptEngine::scriptValueFromQMetaObject(). It is provided as a
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
    QScriptContextPrivate *ctx_p = QScriptContextPrivate::get(d->currentContext());
    d->evaluate(ctx_p, program, lineNumber);
    return ctx_p->m_result;
}

/*!
  Evaluates \a program and returns the result of the evaluation.

  The script code will be evaluated in the current context.

  \sa canEvaluate(), hasUncaughtException()
*/
QScriptValue QScriptEngine::evaluate(const QString &program)
{
    Q_D(QScriptEngine);
    QScriptContextPrivate *ctx_p = QScriptContextPrivate::get(d->currentContext());
    return evaluate(program, ctx_p->currentLine);
}

/*!
  Returns the current context.

  The current context is typically accessed to retrieve the arguments
  to native functions.
*/
QScriptContext *QScriptEngine::currentContext() const
{
    Q_D(const QScriptEngine);
    return d->currentContext();
}

/*!
  Returns true if the last invocation of evaluate() resulted in an
  uncaught exception; otherwise returns false.

  \sa uncaughtExceptionLineNumber()
*/
bool QScriptEngine::hasUncaughtException() const
{
    return (currentContext()->state() == QScriptContext::ExceptionState);
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

  \sa setDefaultPrototype(), newVariant()
*/
QScriptValue QScriptEngine::defaultPrototype(int metaTypeId) const
{
    Q_D(const QScriptEngine);
    return d->defaultPrototype(metaTypeId);
}

/*!
  Sets the default prototype of the given \a metaTypeId to \a prototype.

  \sa newVariant(), qScriptRegisterMetaType()
*/
void QScriptEngine::setDefaultPrototype(int metaTypeId, const QScriptValue &prototype)
{
    Q_D(QScriptEngine);
    d->setDefaultPrototype(metaTypeId, QScriptValuePrivate::valueOf(prototype));
}

/*!
    \typedef QScriptEngine::FunctionSignature
    \relates QScriptEngine

    The function signature \c{QScriptValue f(QScriptContext *, QScriptEngine *)}.

    A function with such a signature can be passed to QScriptEngine::newFunction()
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
    return d->convert(QScriptValuePrivate::valueOf(value), type, ptr);
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
    info.marshal = mf;
    info.demarshal = df;
    info.prototype = QScriptValuePrivate::valueOf(prototype);
    d->m_customTypes.insert(type, info);
}

/*! \fn QScriptValue QScriptEngine::toScriptValue(const T &value)

    Creates a QScriptValue with the given \a value.

    \warning This function is not available with MSVC 6. Use
    qScriptValueFromValue() instead if you need to support that
    version of the compiler.

    \sa fromScriptValue(), qScriptRegisterMetaType()
*/

/*! \fn T QScriptEngine::fromScriptValue(const QScriptValue &value)

    Returns the given \a value converted to the template type \c{T}.

    \warning This function is not available with MSVC 6. Use
    qScriptValueToValue() or qscriptvalue_cast() instead if you need
    to support that version of the compiler.

    \sa toScriptValue(), qScriptRegisterMetaType()
*/

/*!
    \fn QScriptValue qScriptValueFromValue(QScriptEngine *engine, const T &value)
    \relates QScriptEngine

    Creates a QScriptValue using the given \a engine with the given \a
    value of template type \c{T}.

    This function is equivalent to QScriptEngine::toScriptValue(\a
    value). It is provided as a work-around for MSVC 6, which doesn't
    support member template functions.

    \sa qScriptValueToValue()
*/

/*!
    \fn T qScriptValueToValue(const QScriptValue &value)
    \relates QScriptEngine

    Returns the given \a value converted to the template type \c{T}.

    This function is equivalent to QScriptEngine::fromScriptValue(\a
    value). It is provided as a work-around for MSVC 6, which doesn't
    support member template functions.

    \sa qScriptValueFromValue()
*/

/*! \fn T qscriptvalue_cast(const QScriptValue &value)
    \relates QScriptValue

    Returns the given \a value converted to the template type \c{T}.

    \sa qScriptRegisterMetaType(), QScriptEngine::toScriptValue()
*/

/*! \fn int qScriptRegisterMetaType(
            QScriptEngine *engine,
            QScriptValue (*toScriptValue)(QScriptEngine *, const T &t),
            void (*fromScriptValue)(const QScriptValue &, T &t),
            const QScriptValue &prototype = QScriptValue::invalid())
    \relates QScriptEngine

    Registers the type \c{T} in the given \a engine. \a toScriptValue must
    be a function that will convert from a value of type \c{T} to a
    QScriptValue, and \a fromScriptValue a function that does the
    opposite. \a prototype, if valid, is the prototype that's set on
    QScriptValues returned by \a toValue.

    Returns the internal ID used by QMetaType.

    You need to declare the custom type first with
    Q_DECLARE_METATYPE().

    After a type has been registered, you can convert from a
    QScriptValue to that type using
    \l{QScriptEngine::fromScriptValue()}{fromScriptValue}(), and
    create a QScriptValue from a value of that type using
    \l{QScriptEngine::toScriptValue()}{toScriptValue}(). The engine
    will take care of calling the proper conversion function when
    calling C++ slots, and when getting or setting a C++ property;
    i.e. the custom type may be used seamlessly on both the C++ side
    and the script side.

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
      obj.setProperty("x", QScriptValue(engine, s.x));
      obj.setProperty("y", QScriptValue(engine, s.y));
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
    MyStruct s = qscriptvalue_cast<MyStruct>(context->argument(0));

    ...

    MyStruct s2;
    s2.x = s.x + 10;
    s2.y = s.y + 20;
    QScriptValue v = engine->toScriptValue(s2);
    \endcode

    If you want to construct values of your custom type from script code,
    you have to register a constructor function for the type. For example:

    \code
    QScriptValue createMyStruct(QScriptContext *, QScriptEngine *engine)
    {
        MyStruct s;
        s.x = 123;
        s.y = 456;
        return engine->toScriptValue(s);
    }

    ...

    QScriptValue ctor = engine.newFunction(createMyStruct);
    engine.globalObject().setProperty("MyStruct", ctor);
    \endcode

    \sa qScriptRegisterSequenceMetaType(), qRegisterMetaType()
*/

/*!
    \macro Q_SCRIPT_DECLARE_QMETAOBJECT(QMetaObject, ArgType)
    \relates QScriptEngine

    Declares the given \a QMetaObject. Used in combination with
    QScriptEngine::scriptValueFromQMetaObject() to make enums and
    instantiation of \a QMetaObject available to script code. The
    constructor generated by this macro takes a single argument of
    type \a ArgType; typically the argument is the parent type of the
    new instance, in which case \a ArgType is \c{QWidget*} or
    \c{QObject*}.
*/

/*! \fn int qScriptRegisterSequenceMetaType(
            QScriptEngine *engine,
            const QScriptValue &prototype = QScriptValue::invalid())
    \relates QScriptEngine

    Registers the sequence type \c{T} in the given \a engine. This
    function provides conversion functions that convert between \c{T}
    and Qt Script \c{Array} objects. \c{T} must provide a
    const_iterator class and begin(), end() and push_back()
    functions. If \a prototype is valid, it will be set as the
    prototype of \c{Array} objects due to conversion from \c{T};
    otherwise, the standard \c{Array} prototype will be used.

    Returns the internal ID used by QMetaType.

    You need to declare the container type first with
    Q_DECLARE_METATYPE(). Example:

    \code
    Q_DECLARE_METATYPE(QVector<int>)

    ...

    qScriptRegisterSequenceMetaType<QVector<int> >(engine);
    \endcode

    \sa qScriptRegisterMetaType()
*/
