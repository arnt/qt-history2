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

#ifndef QSCRIPTEXTENUMERATION_P_H
#define QSCRIPTEXTENUMERATION_P_H

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
#include <QtCore/QSet>

namespace QScript { namespace Ext {

class EnumerationClassData: public QScriptClassData
{
    QScriptClassInfo *m_classInfo;

public:
    EnumerationClassData(QScriptClassInfo *classInfo);
    virtual ~EnumerationClassData();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void mark(const QScriptValue &object, int generation);
};

class Enumeration: public QScript::Ecma::Core
{
public:
    Enumeration(QScriptEngine *engine);
    virtual ~Enumeration();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValue &object,
                             QScriptClassInfo *klass);

        void toFirst();
        void hasNext(QScriptContext *context, QScriptValue *result);
        void next(QScriptContext *context, QScriptValue *result);

    public: // attributes
        QScriptValue object;
        QScriptValue value;
        int index;
    };

    void newEnumeration(QScriptValue *result, const QScriptValue &value);

    inline Instance *get(const QScriptValue &object) const
    {
        return Instance::get(object, m_classInfo);
    }

protected:
    static QScriptValue method_toFirst(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_hasNext(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_next(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo);

private:
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ext

#endif
