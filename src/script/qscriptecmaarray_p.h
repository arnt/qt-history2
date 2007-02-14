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
    Array(QScriptEnginePrivate *engine);
    virtual ~Array();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContextPrivate *context);

    class ArrayClassData: public QScriptClassData
    {
        QScriptClassInfo *m_classInfo;

    public:
        ArrayClassData(QScriptClassInfo *classInfo);
        virtual ~ArrayClassData();

        inline QScriptClassInfo *classInfo() const
            { return m_classInfo; }

        virtual void mark(const QScriptValueImpl &object, int generation);
        virtual bool resolve(const QScriptValueImpl &object,
                             QScriptNameIdImpl *nameId,
                             QScript::Member *member,
                             QScriptValueImpl *base);
        virtual bool get(const QScriptValueImpl &obj, const Member &m,
                         QScriptValueImpl *out_value);
        virtual bool put(QScriptValueImpl *object, const Member &member,
                         const QScriptValueImpl &value);
        virtual int extraMemberCount(const QScriptValueImpl &object);
        virtual bool extraMember(const QScriptValueImpl &object,
                                 int index, Member *member);
    };

    class Instance: public QScriptObjectData {
    public:
        Instance() {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValueImpl &object,
                             QScriptClassInfo *klass);

    public: // attributes
        QScript::Array value;
    };

    inline Instance *get(const QScriptValueImpl &object) const
    { return Instance::get(object, m_classInfo); }

    void newArray(QScriptValueImpl *result,
                  const QScript::Array &value = QScript::Array());

protected:
    static QScriptValueImpl method_toString(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toLocaleString(QScriptContextPrivate *context,
                                                  QScriptEnginePrivate *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValueImpl method_concat(QScriptContextPrivate *context,
                                          QScriptEnginePrivate *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValueImpl method_join(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_pop(QScriptContextPrivate *context,
                                       QScriptEnginePrivate *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValueImpl method_push(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_reverse(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_shift(QScriptContextPrivate *context,
                                         QScriptEnginePrivate *eng,
                                         QScriptClassInfo *classInfo);
    static QScriptValueImpl method_slice(QScriptContextPrivate *context,
                                         QScriptEnginePrivate *eng,
                                         QScriptClassInfo *classInfo);
    static QScriptValueImpl method_sort(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_splice(QScriptContextPrivate *context,
                                          QScriptEnginePrivate *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValueImpl method_unshift(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);

    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif

