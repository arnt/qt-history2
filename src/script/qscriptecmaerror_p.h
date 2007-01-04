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

#ifndef QSCRIPTECMAERROR_P_H
#define QSCRIPTECMAERROR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qscriptecmacore_p.h"

namespace QScript { namespace Ecma {

class Error: public Core
{
public:
    Error(QScriptEngine *engine);
    virtual ~Error();

    inline QScriptClassInfo *classInfo() const { return m_objectClass; }

    virtual void execute(QScriptContext *context);

    void newError(QScriptValue *result, const QString &message = QString());
    void newEvalError(QScriptValue *result, const QString &message = QString());
    void newRangeError(QScriptValue *result, const QString &message = QString());
    void newReferenceError(QScriptValue *result, const QString &message = QString());
    void newSyntaxError(QScriptValue *result, const QString &message = QString());
    void newTypeError(QScriptValue *result, const QString &message = QString());
    void newURIError(QScriptValue *result, const QString &message = QString());

    QScriptValue evalErrorCtor;
    QScriptValue rangeErrorCtor;
    QScriptValue referenceErrorCtor;
    QScriptValue syntaxErrorCtor;
    QScriptValue typeErrorCtor;
    QScriptValue uriErrorCtor;

    QScriptValue evalErrorPrototype;
    QScriptValue rangeErrorPrototype;
    QScriptValue referenceErrorPrototype;
    QScriptValue syntaxErrorPrototype;
    QScriptValue typeErrorPrototype;
    QScriptValue uriErrorPrototype;

protected:
    static QScriptValue method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo);

    QScriptClassInfo *m_objectClass;

private:
    void newError(QScriptValue *result, const QScriptValue &proto,
                  const QString &message = QString());
    void newErrorPrototype(QScriptValue *result, const QScriptValue &proto,
                           QScriptValue &ztor, const QString &name);
};

} } // namespace QScript::Ecma

#endif // QSCRIPTECMAERROR_P_H
