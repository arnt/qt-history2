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
    RegExp(QScriptEnginePrivate *engine);
    virtual ~RegExp();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContextPrivate *context);

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValueImpl &object,
                             QScriptClassInfo *klass);

    public: // attributes
#ifndef QT_NO_REGEXP
        QRegExp value;
#else
        QString pattern;
#endif
        QString flags;
    };

    inline Instance *get(const QScriptValueImpl &object) const
        { return Instance::get(object, m_classInfo); }

    void newRegExp(QScriptValueImpl *result, const QString &pattern,
                   const QString &flags);
#ifndef QT_NO_REGEXP
    void newRegExp(QScriptValueImpl *result, const QRegExp &rx);
    QRegExp toRegExp(const QScriptValueImpl &value) const;
#endif

protected:
    static QScriptValueImpl method_exec(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_test(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toString(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);

private:
#ifndef QT_NO_REGEXP
    void newRegExp_helper(QScriptValueImpl *result, const QRegExp &rx,
                          const QString &flags);
#endif

    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif // QSCRIPTECMAREGEXP_P_H
