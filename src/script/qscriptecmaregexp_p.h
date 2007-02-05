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

#ifndef QSCRIPTECMAREGEXP_P_H
#define QSCRIPTECMAREGEXP_P_H

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

#include <QtCore/QRegExp>

#include "qscriptecmacore_p.h"

namespace QScript { namespace Ecma {

class RegExp: public Core
{
public:
    RegExp(QScriptEngine *engine);
    virtual ~RegExp();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValue &object,
                             QScriptClassInfo *klass);

    public: // attributes
#ifndef QT_NO_REGEXP
        QRegExp value;
#else
        QString pattern;
#endif
        QString flags;
    };

    inline Instance *get(const QScriptValue &object) const
        { return Instance::get(object, m_classInfo); }

    void newRegExp(QScriptValue *result, const QString &pattern,
                   const QString &flags);
#ifndef QT_NO_REGEXP
    void newRegExp(QScriptValue *result, const QRegExp &rx);
    QRegExp toRegExp(const QScriptValue &value) const;
#endif

protected:
    static QScriptValue method_exec(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo);
    static QScriptValue method_test(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo);
    static QScriptValue method_toString(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);

private:
#ifndef QT_NO_REGEXP
    void newRegExp_helper(QScriptValue *result, const QRegExp &rx,
                          const QString &flags);
#endif

    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif // QSCRIPTECMAREGEXP_P_H
