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

#ifndef QSCRIPTECMADATE_P_H
#define QSCRIPTECMADATE_P_H

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
class QDate;
class QDateTime;

namespace QScript { namespace Ecma {

class Date: public Core
{
public:
    Date(QScriptEngine *engine);
    virtual ~Date();

    inline QScriptClassInfo *classInfo() const { return m_classInfo; }

    virtual void execute(QScriptContext *context);

    void newDate(QScriptValue *result, double t);
    void newDate(QScriptValue *result, const QDateTime &dt);
    void newDate(QScriptValue *result, const QDate &d);

    QDateTime toDateTime(const QScriptValue &date);

protected:
    static QScriptValue method_MakeTime(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_MakeDate(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_TimeClip(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_parse(QScriptEngine *eng,
                                     QScriptClassInfo *classInfo);
    static QScriptValue method_UTC(QScriptEngine *eng,
                                   QScriptClassInfo *classInfo);
    static QScriptValue method_toString(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_toDateString(QScriptEngine *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValue method_toTimeString(QScriptEngine *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValue method_toLocaleString(QScriptEngine *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValue method_toLocaleDateString(QScriptEngine *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValue method_toLocaleTimeString(QScriptEngine *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValue method_valueOf(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_getTime(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_getYear(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_getFullYear(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCFullYear(QScriptEngine *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValue method_getMonth(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCMonth(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_getDate(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCDate(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_getDay(QScriptEngine *eng,
                                      QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCDay(QScriptEngine *eng,
                                         QScriptClassInfo *classInfo);
    static QScriptValue method_getHours(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCHours(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_getMinutes(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCMinutes(QScriptEngine *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValue method_getSeconds(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCSeconds(QScriptEngine *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValue method_getMilliseconds(QScriptEngine *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValue method_getUTCMilliseconds(QScriptEngine *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValue method_getTimezoneOffset(QScriptEngine *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValue method_setTime(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_setMilliseconds(QScriptEngine *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValue method_setUTCMilliseconds(QScriptEngine *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValue method_setSeconds(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_setUTCSeconds(QScriptEngine *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValue method_setMinutes(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_setUTCMinutes(QScriptEngine *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValue method_setHours(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_setUTCHours(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_setDate(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_setUTCDate(QScriptEngine *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValue method_setMonth(QScriptEngine *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValue method_setUTCMonth(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_setYear(QScriptEngine *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValue method_setFullYear(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValue method_setUTCFullYear(QScriptEngine *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValue method_toUTCString(QScriptEngine *eng,
                                           QScriptClassInfo *classInfo);

private:
    QScriptClassInfo *m_classInfo;
};

} } // namespace QScript::Ecma

#endif

