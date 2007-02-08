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
#include "qscriptecmaboolean_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

Boolean::Boolean(QScriptEngine *eng):
    Core(eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    m_classInfo = eng_p->registerClass(QLatin1String("Boolean"));

    publicPrototype.invalidate();
    newBoolean(&publicPrototype, false);

    eng_p->newConstructor(&ctor, this, publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                                eng_p->createFunction(method_valueOf, 0, m_classInfo), flags);
}

Boolean::~Boolean()
{
}

void Boolean::execute(QScriptContext *context)
{
    bool value;
    if (context->argumentCount() > 0)
        value = context->argument(0).toBoolean();
    else
        value = false;

    QScriptValue boolean(engine(), value);
    if (!context->calledAsConstructor()) {
        context->setReturnValue(boolean);
    } else {
        QScriptValue &obj = QScriptContextPrivate::get(context)->thisObject;
        QScriptValueImpl::get(obj)->setClassInfo(classInfo());
        QScriptValueImpl::get(obj)->setInternalValue(boolean);
        obj.setPrototype(publicPrototype);
        context->setReturnValue(obj);
    }
}

void Boolean::newBoolean(QScriptValue *result, bool value)
{
    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    QScriptValueImpl::get(*result)->setInternalValue(QScriptValue(engine(), value));
}

QScriptValue Boolean::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue self = context->thisObject();
    if (QScriptValueImpl::get(self)->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Boolean.prototype.toString"));

    const QScript::IdTable *t = QScriptEnginePrivate::get(context->engine())->idTable();
    bool v = QScriptValueImpl::get(self)->internalValue().toBoolean();
    QScriptValue result;
    QScriptEnginePrivate::get(eng)->newNameId(&result, v ? t->id_true : t->id_false);
    return result;
}

QScriptValue Boolean::method_valueOf(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue self = context->thisObject();
    if (QScriptValueImpl::get(self)->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Boolean.prototype.valueOf"));

    return QScriptValueImpl::get(self)->internalValue();
}

} } // namespace QScript::Ecma
