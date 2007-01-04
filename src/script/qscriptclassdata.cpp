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

#include "qscriptclassdata_p.h"

void QScriptClassData::mark(const QScriptValue &, int)
{
}

bool QScriptClassData:: resolve(const QScriptValue &, QScriptNameIdImpl *,
                                QScript::Member *, QScriptValue *)
{
    return false;
}

bool QScriptClassData::get(const QScriptValue &, const QScript::Member &,
                           QScriptValue *)
{
    Q_ASSERT_X(false, "QScriptClassData::get()",
               "implement if resolveMember is implemented");
    return false;
}

bool QScriptClassData::put(QScriptValue *, const QScript::Member &,
                           const QScriptValue &)
{
    Q_ASSERT_X(false, "QScriptClassData::put()",
               "implement if resolveMember is implemented");
    return false;
}

bool QScriptClassData::removeMember(const QScriptValue &,
                                    const QScript::Member &)
{
    return true;
}

int QScriptClassData::extraMemberCount(const QScriptValue &)
{
    return 0;
}

bool QScriptClassData::extraMember(const QScriptValue &,
                                   int, QScript::Member *)
{
    return false;
}
