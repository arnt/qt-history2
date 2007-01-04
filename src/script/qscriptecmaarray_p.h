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

#ifndef QSCRIPTECMAARRAY_P_H
#define QSCRIPTECMAARRAY_P_H

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

#include "qscriptarray_p.h"
#include "qscriptecmacore_p.h"
#include "qscriptclassdata_p.h"

namespace QScript { namespace Ecma {

class Array: public Core
{
public:
    Array(QScriptEngine *engine);
    virtual ~Array();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    class ArrayClassData: public QScriptClassData
    {
        QScriptClassInfo *m_classInfo;

    public:
        ArrayClassData(QScriptClassInfo *classInfo);
        virtual ~ArrayClassData();

        inline QScriptClassInfo *classInfo() const
            { return m_classInfo; }

        virtual void mark(const QScriptValue &object, int generation);
        virtual bool resolve(const QScriptValue &object,
                             QScriptNameIdImpl *nameId,
                             QScript::Member *member,
                             QScriptValue *base);
        virtual bool get(const QScriptValue &obj, const Member &m,
                         QScriptValue *out_value);
        virtual bool put(QScriptValue *object, const Member &member,
                         const QScriptValue &value);
        virtual int extraMemberCount(const QScriptValue &object);
        virtual bool extraMember(const QScriptValue &object,
                                 int index, Member *member);
    };

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValue &object,
                             QScriptClassInfo *klass);

    public: // attributes
        QScript::Array value;
    };

    inline Instance *get(const QScriptValue &object) const
    { return Instance::get(object, m_classInfo); }

    void newArray(QScriptValue *result,
                  const QScript::Array &value = QScript::Array());

protected:
    static QScriptValue method_toString(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_toLocaleString(QScriptEngine *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValue method_concat(QScriptEngine *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValue method_join(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo);
    static QScriptValue method_pop(QScriptEngine *eng,
                                   QScriptClassInfo *classInfo);
    static QScriptValue method_push(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo);
    static QScriptValue method_reverse(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_shift(QScriptEngine *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValue method_slice(QScriptEngine *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValue method_sort(QScriptEngine *eng,
                                    QScriptClassInfo *classInfo);
    static QScriptValue method_splice(QScriptEngine *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValue method_unshift(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);

    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif

