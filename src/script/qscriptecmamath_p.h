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

#ifndef QSCRIPTECMAMATH_P_H
#define QSCRIPTECMAMATH_P_H

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

class Math: public QScriptObjectData
{
protected:
    Math(QScriptEngine *engine, QScriptClassInfo *classInfo);

public:
    virtual ~Math();

    static void construct(QScriptValue *object, QScriptEngine *eng);

    inline QScriptEngine *engine() const;

protected:
    static QScriptValue method_abs(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_acos(QScriptContext *context,
                                    QScriptEngine *eng);
    static QScriptValue method_asin(QScriptContext *context,
                                    QScriptEngine *eng);
    static QScriptValue method_atan(QScriptContext *context,
                                    QScriptEngine *eng);
    static QScriptValue method_atan2(QScriptContext *context,
                                     QScriptEngine *eng);
    static QScriptValue method_ceil(QScriptContext *context,
                                    QScriptEngine *eng);
    static QScriptValue method_cos(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_exp(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_floor(QScriptContext *context,
                                     QScriptEngine *eng);
    static QScriptValue method_log(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_max(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_min(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_pow(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_random(QScriptContext *context,
                                      QScriptEngine *eng);
    static QScriptValue method_round(QScriptContext *context,
                                     QScriptEngine *eng);
    static QScriptValue method_sin(QScriptContext *context,
                                   QScriptEngine *eng);
    static QScriptValue method_sqrt(QScriptContext *context,
                                    QScriptEngine *eng);
    static QScriptValue method_tan(QScriptContext *context,
                                   QScriptEngine *eng);

private:
    QScriptEngine *m_engine;
    QScriptClassInfo *m_classInfo;
};

inline QScriptEngine *Math::engine() const
{ return m_engine; }


} } // namespace QScript::Ecma

#endif

