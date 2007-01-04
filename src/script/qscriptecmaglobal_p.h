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

#ifndef QSCRIPTECMAGLOBAL_P_H
#define QSCRIPTECMAGLOBAL_P_H

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

class Global: public QScriptObjectData
{
protected:
    Global(QScriptEngine *engine, QScriptClassInfo *classInfo);

public:
    virtual ~Global();

    inline QScriptEngine *engine() const;

    static void construct(QScriptValue *object, QScriptEngine *eng);
    static void initialize(QScriptValue *object, QScriptEngine *eng);

protected:
    static QScriptValue method_parseInt(QScriptContext *context,
                                        QScriptEngine *eng);
    static QScriptValue method_parseFloat(QScriptContext *context,
                                          QScriptEngine *eng);
    static QScriptValue method_isNaN(QScriptContext *context,
                                     QScriptEngine *eng);
    static QScriptValue method_isFinite(QScriptContext *context,
                                        QScriptEngine *eng);
    static QScriptValue method_decodeURI(QScriptContext *context,
                                         QScriptEngine *eng);
    static QScriptValue method_decodeURIComponent(QScriptContext *context,
                                                  QScriptEngine *eng);
    static QScriptValue method_encodeURI(QScriptContext *context,
                                         QScriptEngine *eng);
    static QScriptValue method_encodeURIComponent(QScriptContext *context,
                                                  QScriptEngine *eng);
    static QScriptValue method_escape(QScriptContext *context,
                                      QScriptEngine *eng);
    static QScriptValue method_unescape(QScriptContext *context,
                                        QScriptEngine *eng);
    static QScriptValue method_version(QScriptContext *context,
                                       QScriptEngine *eng);
    static QScriptValue method_gc(QScriptContext *context,
                                  QScriptEngine *eng);

private:
    QScriptEngine *m_engine;
    QScriptClassInfo *m_classInfo;
};

inline QScriptEngine *Global::engine() const
{ return m_engine; }


} } // namespace QScript::Ecma

#endif

