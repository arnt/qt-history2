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
#include "qscriptcontext_p.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaerror_p.h"

/*!
  \since 4.3
  \class QScriptContext

  \brief The QScriptContext class represents a Qt Script function invocation.

  \ingroup script
  \mainclass

  A QScriptContext contains the `this' object and arguments passed to
  a script function. You typically want to access this information
  when you're writing a native (C++) function (see
  QScriptEngine::createFunction()) that will be called from script
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

  Use calledAsConstructor() to determine if the function was called as a
  constructor (e.g. \c{"new foo()"} (as constructor) or just \c{"foo()"}).

  Use throwValue() or throwError() to throw an exception.

  Use callee() to obtain the QScriptValue that represents the function being
  called.

  Use parentContext() to get a pointer to the context that precedes
  this context in the activation stack.

  Use engine() to obtain a pointer to the QScriptEngine that this context
  resides in.

  \sa QScriptEngine
*/

/*!
    \enum QScriptContext::State

    \value Normal The context is in a normal state.

    \value Exception The context is in an exceptional state.
*/

/*!
    \enum QScriptContext::Error

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

  \sa throwError(), recoverFromException(), state()
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

  \sa throwValue(), recoverFromException(), state()
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

  \sa throwValue(), recoverFromException(), state()
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
  Returns true if the function was called as a constructor
  (e.g. \c{"new foo()"}); otherwise returns false.

  When a function is called as constructor, the thisObject()
  contains the newly constructed object to be initialized.
*/
bool QScriptContext::calledAsConstructor() const
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
  \internal
*/
QScriptValue QScriptContext::activationObject() const
{
    Q_D(const QScriptContext);
    return d->m_activation;
}

/*!
  \internal
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
  \internal
*/
const QScriptInstruction *QScriptContext::instructionPointer() const
{
    Q_D(const QScriptContext);
    return d->iPtr;
}

/*!
  \internal
*/
void QScriptContext::setInstructionPointer(const QScriptInstruction *instructionPointer)
{
    Q_D(QScriptContext);
    d->iPtr = instructionPointer;
}

/*!
  \internal
*/
const QScriptInstruction *QScriptContext::firstInstruction() const
{
    Q_D(const QScriptContext);
    return d->firstInstruction;
}

/*!
  \internal
*/
const QScriptInstruction *QScriptContext::lastInstruction() const
{
    Q_D(const QScriptContext);
    return d->lastInstruction;
}

/*!
  Returns the state of this QScriptContext.
*/
QScriptContext::ExecutionState QScriptContext::state() const
{
    Q_D(const QScriptContext);
    return d->m_state;
}

/*!
  \internal
*/
int QScriptContext::errorLineNumber() const
{
    Q_D(const QScriptContext);
    return d->errorLineNumber;
}
