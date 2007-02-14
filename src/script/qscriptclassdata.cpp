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

void QScriptClassData::mark(const QScriptValueImpl &, int)
{
}

bool QScriptClassData:: resolve(const QScriptValueImpl &, QScriptNameIdImpl *,
                                QScript::Member *, QScriptValueImpl *)
{
    return false;
}

bool QScriptClassData::get(const QScriptValueImpl &, const QScript::Member &,
                           QScriptValueImpl *)
{
    Q_ASSERT_X(false, "QScriptClassData::get()",
               "implement if resolveMember is implemented");
    return false;
}

bool QScriptClassData::put(QScriptValueImpl *, const QScript::Member &,
                           const QScriptValueImpl &)
{
    Q_ASSERT_X(false, "QScriptClassData::put()",
               "implement if resolveMember is implemented");
    return false;
}

bool QScriptClassData::removeMember(const QScriptValueImpl &,
                                    const QScript::Member &)
{
    return true;
}

int QScriptClassData::extraMemberCount(const QScriptValueImpl &)
{
    return 0;
}

bool QScriptClassData::extraMember(const QScriptValueImpl &,
                                   int, QScript::Member *)
{
    return false;
}
