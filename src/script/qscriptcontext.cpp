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

#include "qscriptcontext.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

/*!
  \since 4.3
  \class QScriptContext

  \brief The QScriptContext class represents a Qt Script function invocation.

  \ingroup script
  \mainclass

  A QScriptContext provides access to the `this' object and arguments
  passed to a script function. You typically want to access this
  information when you're writing a native (C++) function (see
  QScriptEngine::newFunction()) that will be called from script
  code. For example, when the script code

  \code
  foo(20.5, "hello", new Object())
  \endcode

  is evaluated, a QScriptContext will be created, and the context will
  carry the arguments as QScriptValues; in this particular case, the
  arguments will be one QScriptValue containing the number 20.5, a second
  QScriptValue containing the string \c{"hello"}, and a third QScriptValue
  containing a Qt Script object.

  Use argumentCount() to get the number of arguments passed to the
  function, and argument() to get an argument at a certain index.

  Use thisObject() to get the `this' object associated with the function call,
  and setThisObject() to set the `this' object.

  Use isCalledAsConstructor() to determine if the function was called as a
  constructor (e.g. \c{"new foo()"} (as constructor) or just \c{"foo()"}).

  Use throwValue() or throwError() to throw an exception.

  Use callee() to obtain the QScriptValue that represents the function being
  called.

  Use parentContext() to get a pointer to the context that precedes
  this context in the activation stack.

  Use engine() to obtain a pointer to the QScriptEngine that this context
  resides in.

  Use backtrace() to get a human-readable backtrace associated with this
  context. This can be useful for debugging purposes when implementing
  native functions.

  \sa QScriptEngine::newFunction(), QScriptable
*/

/*!
    \enum QScriptContext::ExecutionState

    This enum specifies the execution state of the context.

    \value NormalState The context is in a normal state.

    \value ExceptionState The context is in an exceptional state.
*/

/*!
    \enum QScriptContext::Error

    This enum specifies types of error.

    \value ReferenceError A reference error.

    \value SyntaxError A syntax error.

    \value TypeError A type error.

    \value RangeError A range error.

    \value URIError A URI error.

    \value UnknownError An unknown error.
*/

/*!
  Throws an exception with the given \a value.
  Returns the value thrown (the same as the argument).

  \sa throwError(), state()
*/
QScriptValue QScriptContext::throwValue(const QScriptValue &value)
{
    Q_D(QScriptContext);
    d->m_result = QScriptValuePrivate::valueOf(value);
    d->m_state = QScriptContext::ExceptionState;
    return value;
}

/*!
  Throws an \a error with the given \a text.
  Returns the created error object.

  The \a text will be stored in the \c{message} property of the error
  object.

  The error object will be initialized to contain information about
  the location where the error occurred; specifically, it will have
  properties \c{lineNumber}, \c{fileName} and \c{stack}. These
  properties are described in \l {QtScript Extensions to ECMAScript}.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(Error error, const QString &text)
{
    Q_D(QScriptContext);
    return d->throwError(error, text);
}

/*!
  \overload

  Throws an error with the given \a text.
  Returns the created error object.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(const QString &text)
{
    Q_D(QScriptContext);
    return d->throwError(text);
}

/*!
  \internal
*/
QScriptContext::QScriptContext():
    d_ptr(new QScriptContextPrivate())
{
    d_ptr->q_ptr = this;
}

/*!
  Destroys this QScriptContext.
*/
QScriptContext::~QScriptContext()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Returns the QScriptEngine that this QScriptContext belongs to.
*/
QScriptEngine *QScriptContext::engine() const
{
    Q_D(const QScriptContext);
    return d->engine();
}

/*!
  Returns the function argument at the given \a index.

  If \a index >= argumentCount(), a QScriptValue of
  the primitive type Undefined is returned.

  \sa argumentCount()
*/
QScriptValue QScriptContext::argument(int index) const
{
    Q_D(const QScriptContext);
    return d->argument(index);
}

/*!
  Returns the callee. The callee is the function object that this
  QScriptContext represents an invocation of.
*/
QScriptValue QScriptContext::callee() const
{
    Q_D(const QScriptContext);
    return d->m_callee;
}

/*!
  Returns the arguments object of this QScriptContext.

  The arguments object has properties \c callee (equal to callee())
  and \c length (equal to argumentCount()), and properties \c 0, \c 1,
  ..., argumentCount() - 1 that provide access to the argument
  values. Initially, property \c P (0 <= \c P < argumentCount()) has
  the same value as argument(\c P). In the case when \c P is less
  than the number of formal parameters of the function, \c P shares
  its value with the corresponding property of the activation object
  (activationObject()). This means that changing this property changes
  the corresponding property of the activation object and vice versa.

  \sa argument(), activationObject()
*/
QScriptValue QScriptContext::argumentsObject() const
{
    Q_D(const QScriptContext);
    if (!d->m_arguments.isValid() && d->m_activation.isValid()) {
        QScriptContextPrivate *dd = const_cast<QScriptContextPrivate*>(d);
        QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
        eng_p->newArguments(&dd->m_arguments, d->m_activation,
                            d->argc, d->m_callee);
    }
    return d->m_arguments;
}

/*!
  Returns true if the function was called as a constructor
  (e.g. \c{"new foo()"}); otherwise returns false.

  When a function is called as constructor, the thisObject()
  contains the newly constructed object to be initialized.
*/
bool QScriptContext::isCalledAsConstructor() const
{
    Q_D(const QScriptContext);
    return d->m_calledAsConstructor;
}

/*!
  Returns the parent context of this QScriptContext.
*/
QScriptContext *QScriptContext::parentContext() const
{
    Q_D(const QScriptContext);
    return d->previous;
}

/*!
  Returns the number of arguments passed to the function
  in this invocation.

  Note that the argument count can be different from the
  formal number of arguments (the \c{length} property of
  callee()).

  \sa argument()
*/
int QScriptContext::argumentCount() const
{
    Q_D(const QScriptContext);
    return d->argc;
}

/*!
  \internal
*/
QScriptValue QScriptContext::returnValue() const
{
    Q_D(const QScriptContext);
    return d->m_result;
}

/*!
  \internal
*/
void QScriptContext::setReturnValue(const QScriptValue &result)
{
    Q_D(QScriptContext);
    d->m_result = QScriptValuePrivate::valueOf(result);
}

/*!
  Returns the activation object of this QScriptContext. The activation
  object provides access to the local variables associated with this
  context.

  \sa argument(), argumentsObject()
*/
QScriptValue QScriptContext::activationObject() const
{
    Q_D(const QScriptContext);
    if (d->previous && !d->m_activation.property(QLatin1String("arguments")).isValid()) {
        QScriptContextPrivate *dd = const_cast<QScriptContextPrivate*>(d);
        dd->m_activation.setProperty(QLatin1String("arguments"), QScriptValuePrivate::valueOf(argumentsObject()));
    }
    return d->m_activation;
}

/*!
  Sets the activation object of this QScriptContext to be the given \a
  activation.
*/
void QScriptContext::setActivationObject(const QScriptValue &activation)
{
    Q_D(QScriptContext);
    d->m_activation = QScriptValuePrivate::valueOf(activation);
}

/*!
  Returns the `this' object associated with this QScriptContext.
*/
QScriptValue QScriptContext::thisObject() const
{
    Q_D(const QScriptContext);
    return d->m_thisObject;
}

/*!
  Sets the `this' object associated with this QScriptContext to be
  \a thisObject.
*/
void QScriptContext::setThisObject(const QScriptValue &thisObject)
{
    Q_D(QScriptContext);
    d->m_thisObject = QScriptValuePrivate::valueOf(thisObject);
}

/*!
  Returns the execution state of this QScriptContext.
*/
QScriptContext::ExecutionState QScriptContext::state() const
{
    Q_D(const QScriptContext);
    return d->m_state;
}

/*!
  Returns a human-readable backtrace of this QScriptContext.

  Each line is of the form \c{<function-name>(<arguments>)@<file-name>:<line-number>}.

  \sa QScriptEngine::uncaughtExceptionBacktrace()
*/
QStringList QScriptContext::backtrace() const
{
    Q_D(const QScriptContext);
    return d->backtrace();
}

#endif // QT_NO_SCRIPT
