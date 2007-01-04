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

#ifndef QSCRIPTECMAFUNCTION_P_H
#define QSCRIPTECMAFUNCTION_P_H

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

class Function: public Core
{
public:
    Function(QScriptEngine *engine, QScriptClassInfo *classInfo);
    virtual ~Function();

    void initialize();

    inline QScriptClassInfo *classInfo() const { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    void newFunction(QScriptValue *result, QScriptFunction *foo);

protected:
    QString buildFunction(QScriptContext *context);

    static QScriptValue method_toString(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_apply(QScriptEngine *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValue method_call(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo);
    static QScriptValue method_void(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo); // internal
    static QScriptValue method_disconnect(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_connect(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);

private:
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif

