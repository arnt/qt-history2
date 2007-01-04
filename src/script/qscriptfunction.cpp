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

#include "qscriptfunction_p.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptcontext_p.h"

QScriptFunction::~QScriptFunction()
{
}

QString QScriptFunction::toString(QScriptContext *) const
{
    QString result;
    result += QLatin1String("function () { [native] }");
    return result;
}

void QScript::CFunction::execute(QScriptContext *context)
{
    QScriptContextPrivate *context_p = QScriptContextPrivate::get(context);
    QScriptEngine *eng = context_p->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    eng_p->newUndefined(&context_p->result);

    bool blocked = eng_p->blockGC(true);
    context_p->result = (*m_funPtr)(eng->currentContext(), eng);
    eng_p->blockGC(blocked);
}

void QScript::C2Function::execute(QScriptContext *context)
{
    QScriptContextPrivate *context_p = QScriptContextPrivate::get(context);
    QScriptEngine *eng = context_p->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    eng_p->newUndefined(&context_p->result);

    bool blocked = eng_p->blockGC(true);
    context_p->result = (*m_funPtr)(eng, m_classInfo);
    eng_p->blockGC(blocked);
}
