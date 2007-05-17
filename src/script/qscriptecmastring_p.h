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

#ifndef QT_NO_SCRIPT

namespace QScript { namespace Ecma {

class String: public Core
{
public:
    String(QScriptEnginePrivate *engine);
    virtual ~String();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual void execute(QScriptContextPrivate *context);

    class StringClassData: public QScriptClassData
    {
        QScriptClassInfo *m_classInfo;

    public:
        StringClassData(QScriptClassInfo *classInfo);
        virtual ~StringClassData();

        inline QScriptClassInfo *classInfo() const
            { return m_classInfo; }

        virtual bool resolve(const QScriptValueImpl &object,
                             QScriptNameIdImpl *nameId,
                             QScript::Member *member, QScriptValueImpl *base);
        virtual bool get(const QScriptValueImpl &obj, const Member &m,
                         QScriptValueImpl *out_value);
        virtual int extraMemberCount(const QScriptValueImpl &object);
        virtual bool extraMember(const QScriptValueImpl &object,
                                 int index, Member *member);
    };

    void newString(QScriptValueImpl *result, const QString &value = QString());

protected:
    static QScriptValueImpl method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValueImpl method_charAt(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_charCodeAt(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValueImpl method_concat(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_indexOf(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValueImpl method_lastIndexOf(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_localeCompare(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValueImpl method_match(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValueImpl method_replace(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValueImpl method_search(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_slice(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValueImpl method_split(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValueImpl method_substring(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                         QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toLowerCase(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toLocaleLowerCase(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toUpperCase(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toLocaleUpperCase(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValueImpl method_fromCharCode(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);

private:
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif // QT_NO_SCRIPT
#endif

