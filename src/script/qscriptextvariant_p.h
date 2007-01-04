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

#ifndef QSCRIPTEXTVARIANT_P_H
#define QSCRIPTEXTVARIANT_P_H

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

#include <QtCore/QVariant>

#include "qscriptecmacore_p.h"

namespace QScript { namespace Ext {

class Instance;

class Variant: public Ecma::Core
{
public:
    Variant(QScriptEngine *engine, QScriptClassInfo *classInfo);
    virtual ~Variant();

    inline QScriptClassInfo *classInfo() const { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValue &object,
                             QScriptClassInfo *klass);

    public:
        QVariant value;
    };

    inline Instance *get(const QScriptValue &object) const
        { return Instance::get(object, classInfo()); }

    void newVariant(QScriptValue *result, const QVariant &value);

protected:
    static QScriptValue method_toString(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_valueOf(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);

protected:
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ext

#endif // QSCRIPTEXTVARIANT_P_H
