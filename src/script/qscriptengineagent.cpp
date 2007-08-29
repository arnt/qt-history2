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

#include "qscriptengineagent.h"

#ifndef QT_NO_SCRIPT

#include "qscriptvalue.h"
#include "qscriptengineagent_p.h"

/*!
  \since 4.4
  \class QScriptEngineAgent

  \brief The QScriptEngineAgent class provides an interface to report events pertaining to QScriptEngine execution.

  \ingroup script
  \mainclass

  The QScriptEngineAgent class is the basis of tools that monitor and/or control the execution of a
  QScriptEngine, such as debuggers and profilers.

  To process script loading and unloading events, reimplement the
  scriptLoad() and scriptUnload() functions. scriptLoad() is called
  after the input to QScriptEngine::evaluate() has been parsed, right
  before the given script is executed. The engine assigns each
  script an ID, which is available as one of the arguments to
  scriptLoad(); subsequently, other event handlers can use the ID to
  identify a particular script. One common usage of scriptLoad() is
  to retain the script text, filename and base line number (the
  original input to QScriptEngine::evaluate()), so that other event
  handlers can e.g. map a line number to the corresponding line of
  text.

  scriptUnload() is called when the QScriptEngine has no further use
  for a script; the QScriptEngineAgent may at this point safely
  discard any resources associated with the script (such as the
  script text). Note that after scriptUnload() has been called, the
  QScriptEngine may reuse the relevant script ID for new scripts
  (i.e. as argument to a subsequent call to scriptLoad()).

  Evaluating the following script will result in scriptUnload()
  being called immediately after evaluation has completed:

  \code
      var a = Math.random() + 2;
  \endcode

  Evaluating the following script will \b{not} result in a call to
  scriptUnload() when evaluation has completed:

  \code
      function cube(a) {
          return a * a * a;
      }

      var a = cube(3);
  \endcode

  The script isn't unloaded because it defines a function (\c{cube})
  that remains in the script environment after evaluation has
  completed. If a subsequent script removed the \c{cube} function
  (e.g. by setting it to \c{null}), scriptUnload() would be called
  when the function is garbage collected. In general terms, a script
  isn't unloaded until the engine has determined that none of its
  contents is referenced.

  To process script function calls and returns, reimplement the
  functionEntry() and functionExit() functions. functionEntry() is
  called when a script function is about to be executed;
  functionExit() is called when a script function is about to return,
  either normally or due to an exception.

  To process individual script statements, reimplement
  positionChange(). positionChange() is called each time the engine is
  about to execute a new statement of a script, and thus offers the
  finest level of script monitoring.

  To process exceptions, reimplement exceptionThrow() and
  exceptionCatch(). exceptionThrow() is called when a script exception
  is thrown, before it has been handled. exceptionCatch() is called
  when an exception handler is present, and execution is about to be
  resumed at the handler code.

  \sa QScriptEngine::setAgent(), QScriptContextInfo
*/

/*!
  \enum QScriptEngineAgent::Extension

  This enum specifies the possible extensions to a QScriptEngineAgent.

  \sa extension()
*/

QScriptEngineAgentPrivate::QScriptEngineAgentPrivate()
{
}

QScriptEngineAgentPrivate::~QScriptEngineAgentPrivate()
{
}

/*!
    Constructs a QScriptEngineAgent object for the given \a engine.

    The engine takes ownership of the agent.

    Call QScriptEngine::setAgent() to make this agent the active
    agent.
*/
QScriptEngineAgent::QScriptEngineAgent(QScriptEngine *engine)
    : d_ptr(new QScriptEngineAgentPrivate)
{
    d_ptr->q_ptr = this;
    d_ptr->engine = engine;
}

/*!
  \internal
*/
QScriptEngineAgent::QScriptEngineAgent(QScriptEngineAgentPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
  Destroys this QScriptEngineAgent.
*/
QScriptEngineAgent::~QScriptEngineAgent()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!

  This function is called when the engine has parsed a script and has
  associated it with the given \a id. The id can be used to identify
  this particular script in subsequent event notifications.

  \a program, \a fileName and \a baseLineNumber are the original
  arguments to the QScriptEngine::evaluate() call that triggered this
  event.

  This function is called just before the script is about to be
  evaluated.

  You can reimplement this function to record information about the
  script; for example, by retaining the script text, you can obtain
  the line of text corresponding to a line number in a subsequent
  call to positionChange().

  The default implementation does nothing.

  \sa scriptUnload()
*/
void QScriptEngineAgent::scriptLoad(qint64 id, const QString &program,
                                    const QString &fileName, int baseLineNumber)
{
    Q_UNUSED(id);
    Q_UNUSED(program);
    Q_UNUSED(fileName);
    Q_UNUSED(baseLineNumber);
}

/*!
  This function is called when the engine has discarded the script
  identified by the given \a id.

  You can reimplement this function to clean up any resources you have
  associated with the script.

  The default implementation does nothing.

  \sa scriptLoad()
*/
void QScriptEngineAgent::scriptUnload(qint64 id)
{
    Q_UNUSED(id);
}

/*!
  This function is called when a script function is called in the
  engine. If the script function is not a native Qt Script function,
  it resides in the script identified by \a scriptId; otherwise, \a
  scriptId is -1.

  This function is called just before execution of the script function
  begins.  You can obtain the QScriptContext associated with the
  function call with QScriptEngine::currentContext(). The arguments
  passed to the function are available.

  Reimplement this function to handle this event. For example, a
  debugger implementation could reimplement this function (and
  functionExit()) to keep track of the call stack and provide
  step-over functionality.

  The default implementation does nothing.

  \sa functionExit(), positionChange(), QScriptEngine::currentContext()
*/
void QScriptEngineAgent::functionEntry(qint64 scriptId)
{
    Q_UNUSED(scriptId);
}

/*!
  This function is called when the currently executing script function
  is about to return. If the script function is not a native Qt Script
  function, it resides in the script identified by \a scriptId;
  otherwise, \a scriptId is -1. The \a returnValue is the value that
  the script function will return.

  This function is called just before the script function returns.
  You can still access the QScriptContext associated with the
  script function call with QScriptEngine::currentContext().

  If the engine's
  \l{QScriptEngine::hasUncaughtException()}{hasUncaughtException}()
  function returns true, the script function is exiting due to an
  exception; otherwise, the script function is returning normally.

  Reimplement this function to handle this event; typically you will
  then also want to reimplement functionEntry().

  The default implementation does nothing.

  \sa functionEntry(), QScriptEngine::hasUncaughtException()
*/
void QScriptEngineAgent::functionExit(qint64 scriptId,
                                      const QScriptValue &returnValue)
{
    Q_UNUSED(scriptId);
    Q_UNUSED(returnValue);
}

/*!
  This function is called when the engine is about to execute a new
  statement in the script identified by \a scriptId.  The statement
  begins on the line and column specified by \a lineNumber and \a
  columnNumber.  This event is not generated for native Qt Script
  functions.

  Reimplement this function to handle this event. For example, a
  debugger implementation could reimplement this function to provide
  line-by-line stepping, and a profiler implementation could use it to
  count the number of times each statement is executed.

  The default implementation does nothing.

  \sa scriptLoad(), functionEntry()
*/
void QScriptEngineAgent::positionChange(qint64 scriptId,
                                        int lineNumber, int columnNumber)
{
    Q_UNUSED(scriptId);
    Q_UNUSED(lineNumber);
    Q_UNUSED(columnNumber);
}

/*!
  This function is called when the given \a exception has occurred in
  the engine, in the script identified by \a scriptId. If the
  exception was thrown by a native Qt Script function, \a scriptId is
  -1.

  If \a hasHandler is true, there is a \c{catch} or \c{finally} block
  that will handle the exception. If \a hasHandler is false, there is
  no handler for the exception.

  Reimplement this function if you want to handle this event. For
  example, a debugger can notify the user when an uncaught exception
  occurs (i.e. \a hasHandler is false).

  The default implementation does nothing.

  \sa exceptionCatch()
*/
void QScriptEngineAgent::exceptionThrow(qint64 scriptId,
                                        const QScriptValue &exception,
                                        bool hasHandler)
{
    Q_UNUSED(scriptId);
    Q_UNUSED(exception);
    Q_UNUSED(hasHandler);
}

/*!
  This function is called when the given \a exception is about to be
  caught, in the script identified by \a scriptId.

  Reimplement this function if you want to handle this event.

  The default implementation does nothing.

  \sa exceptionThrow()
*/
void QScriptEngineAgent::exceptionCatch(qint64 scriptId,
                                        const QScriptValue &exception)
{
    Q_UNUSED(scriptId);
    Q_UNUSED(exception);
}

#if 0
/*!
  This function is called when a property of the given \a object has
  been added, changed or removed.

  Reimplement this function if you want to handle this event.

  The default implementation does nothing.
*/
void QScriptEngineAgent::propertyChange(qint64 scriptId,
                                        const QScriptValue &object,
                                        const QString &propertyName,
                                        PropertyChange change)
{
    Q_UNUSED(scriptId);
    Q_UNUSED(object);
    Q_UNUSED(propertyName);
    Q_UNUSED(change);
}
#endif

/*!
  Returns true if the QScriptEngineAgent supports the given \a
  extension; otherwise, false is returned. By default, no extensions
  are supported.

  \sa extension()
*/
bool QScriptEngineAgent::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
  This virtual function can be reimplemented in a
  QScriptEngineAgent subclass to provide support for
  extensions. The optional \a argument can be provided as input to the
  \a extension; the result must be returned in the form of a
  QVariant. You can call supportsExtension() to check if an extension
  is supported by the QScriptClass.  By default, no extensions are
  supported, and this function returns an invalid QVariant.

  \sa supportsExtension()
*/
QVariant QScriptEngineAgent::extension(Extension extension,
                                       const QVariant &argument)
{
    Q_UNUSED(extension);
    Q_UNUSED(argument);
    return QVariant();
}

/*!
  Returns the QScriptEngine that this agent is associated with.
*/
QScriptEngine *QScriptEngineAgent::engine() const
{
    Q_D(const QScriptEngineAgent);
    return d->engine;
}

#endif // QT_NO_SCRIPT
