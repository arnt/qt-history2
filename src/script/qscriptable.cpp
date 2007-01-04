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

#ifndef QT_NO_QOBJECT

#include "qscriptable.h"
#include "qscriptable_p.h"

#include "qscriptengine.h"
#include "qscriptcontext.h"
#include "qscriptvalue.h"

/*!
  \since 4.3
  \class QScriptable

  \brief The QScriptable class provides access to the Qt Script environment from Qt C++ member functions.

  \ingroup script
  \mainclass

  With QScriptEngine::scriptValueFromQObject(), you can expose the
  signals and slots of any QObject (or subclass) to script
  code. QScriptable augments this functionality by giving slots
  access to the Qt Script environment they are invoked in;
  conceptually, it is similar to QObject::sender().

  By subclassing QScriptable, you get the following functions in
  your class: thisObject(), argumentCount(), argument(), context() and
  engine(). With these functions, you have full access to the Qt
  Script environment in the slots of your class, when they are invoked
  from script code. This allows for many exciting opportunities: For
  example, you can throw a Qt Script exception from a slot; you can
  inspect the arguments stored in the QScriptContext to know the
  "real" arguments passed to the function by the script code; or you
  can call script functions from your slot. You can operate on the
  thisObject(), for example by casting it to an instance of your
  class; or, you can cast the thisObject(), or the arguments, to any
  type you want (including custom types you've registered with the
  script engine).

  The following is what subclassing QScriptable typically looks
  like:

  \code
  class MyScriptableObject: public QObject,
        protected QScriptable
  {
    Q_OBJECT

  ...

  public Q_SLOTS:
    void doSomething();
    double doSomethingElse();
  }
  \endcode

  The only difference from regular QObject subclassing is that you
  also inherit from QScriptable.

  In the implementation of your slots, you can then use the functions
  inherited from QScriptable:

  \code
    void MyScriptableObject::doSomething()
    {
      context()->throwError("Threw an error from a slot");
    }

    double MyScriptableObject::doSomethingElse()
    {
      return qscript_cast<double>(thisObject());
    }
  \endcode

  QScriptable is very convenient to use when implementing
  prototype objects; you implement the functions of your prototype as
  slots, then you wrap an instance of your class using
  QScriptEngine::scriptValueFromQObject(), and set the result as the
  \c{prototype} property of your constructor function.
*/

QScriptable::QScriptable()
    : d_ptr(new QScriptablePrivate())
{
    d_ptr->q_ptr = this;
}

QScriptable::~QScriptable()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Returns a pointer to the QScriptEngine associated with the current
  Qt function call, or 0 if the Qt function was not invoked from
  script code.
*/
QScriptEngine *QScriptable::engine() const
{
    Q_D(const QScriptable);
    return d->engine;
}

/*!
  Returns a pointer to the QScriptContext associated with the current
  Qt function call, or 0 if the Qt function was not invoked from
  script code.
*/
QScriptContext *QScriptable::context() const
{
    if (QScriptEngine *e = engine())
        return e->currentContext();
    
    return 0;
}

/*!
  Returns the `this' object associated with the current Qt function
  call, or an invalid QScriptValue if the Qt function was not invoked
  from script code.
*/

QScriptValue QScriptable::thisObject() const
{
    if (QScriptContext *c = context())
        return c->thisObject();
    
    return QScriptValue();
}

/*!
  Returns the number of arguments passed to the function in this
  invocation, or -1 if the Qt function was not invoked from script
  code.
*/
int QScriptable::argumentCount() const
{
    if (QScriptContext *c = context())
        return c->argumentCount();
    
    return -1;
}

/*!
  Returns the function argument at the given \a index, or an invalid
  QScriptValue if the Qt function was not invoked from script code.
*/
QScriptValue QScriptable::argument(int index) const
{
    if (QScriptContext *c = context())
        return c->argument(index);
    
    return QScriptValue();
}

#endif // QT_NO_QOBJECT
