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

#ifndef QSCRIPTECMASTRING_P_H
#define QSCRIPTECMASTRING_P_H

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

class String: public Core
{
public:
    String(QScriptEngine *engine);
    virtual ~String();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    class StringClassData: public QScriptClassData
    {
        QScriptClassInfo *m_classInfo;

    public:
        StringClassData(QScriptClassInfo *classInfo);
        virtual ~StringClassData();

        inline QScriptClassInfo *classInfo() const
            { return m_classInfo; }

        virtual bool resolve(const QScriptValue &object,
                             QScriptNameIdImpl *nameId,
                             QScript::Member *member, QScriptValue *base);
        virtual bool get(const QScriptValue &obj, const Member &m,
                         QScriptValue *out_value);
        virtual int extraMemberCount(const QScriptValue &object);
        virtual bool extraMember(const QScriptValue &object,
                                 int index, Member *member);
    };

    void newString(QScriptValue *result, const QString &value = QString());

protected:
    static QScriptValue method_toString(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_valueOf(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_charAt(QScriptEngine *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValue method_charCodeAt(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_concat(QScriptEngine *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValue method_indexOf(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_lastIndexOf(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_localeCompare(QScriptEngine *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValue method_match(QScriptEngine *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValue method_replace(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_search(QScriptEngine *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValue method_slice(QScriptEngine *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValue method_split(QScriptEngine *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValue method_substring(QScriptEngine *eng,
                                         QScriptClassInfo *classInfo);
    static QScriptValue method_toLowerCase(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_toLocaleLowerCase(QScriptEngine *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValue method_toUpperCase(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_toLocaleUpperCase(QScriptEngine *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValue method_fromCharCode(QScriptEngine *eng,
                                            QScriptClassInfo *classInfo);

private:
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif

