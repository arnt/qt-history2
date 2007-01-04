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

#ifndef QSCRIPTECMANUMBER_P_H
#define QSCRIPTECMANUMBER_P_H

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

class Number: public Core
{
public:
    Number(QScriptEngine *engine);
    virtual ~Number();

    inline QScriptClassInfo *classInfo() const { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    void newNumber(QScriptValue *result, double value = 0);

protected:
    static QScriptValue method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo);
    static QScriptValue method_toLocaleString(QScriptEngine *eng, QScriptClassInfo *classInfo);
    static QScriptValue method_valueOf(QScriptEngine *eng, QScriptClassInfo *classInfo);
    static QScriptValue method_toFixed(QScriptEngine *eng, QScriptClassInfo *classInfo);
    static QScriptValue method_toExponential(QScriptEngine *eng, QScriptClassInfo *classInfo);
    static QScriptValue method_toPrecision(QScriptEngine *eng, QScriptClassInfo *classInfo);

    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif

