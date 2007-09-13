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

#ifndef QSCRIPTECMACORE_P_H
#define QSCRIPTECMACORE_P_H

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

#include "qscriptfunction_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptvalueimplfwd_p.h"

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

class Core: public QScriptFunction
{
public:
    Core(QScriptEnginePrivate *engine, const QString &className);
    Core(QScriptEnginePrivate *engine, QScriptClassInfo *classInfo);
    virtual ~Core();

    inline QScriptEnginePrivate *engine() const
    { return m_engine; }

    inline QScriptClassInfo *classInfo() const
    { return m_classInfo; }

    void addPrototypeFunction(
        const QString &name, QScriptInternalFunctionSignature fun, int length,
        const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration);
    void addConstructorFunction(
        const QString &name, QScriptInternalFunctionSignature fun, int length,
        const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration);

    QString functionName() const;

public: // attributes
    QScriptValueImpl ctor;
    QScriptValueImpl publicPrototype;

private:
    void addFunction(QScriptValueImpl &object, const QString &name,
                     QScriptInternalFunctionSignature fun, int length,
                     const QScriptValue::PropertyFlags flags);

    QScriptEnginePrivate *m_engine;
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif // QT_NO_SCRIPT

QT_END_NAMESPACE

#endif
