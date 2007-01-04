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

#ifndef QSCRIPTCLASSDATA_P_H
#define QSCRIPTCLASSDATA_P_H

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

class QScriptValue;
class QScriptNameIdImpl;
#include "qscriptmember_p.h"
#include <QtCore/qshareddata.h>

class QScriptClassData: public QSharedData
{
protected:
    QScriptClassData() {}

public:
    virtual ~QScriptClassData() {}

    virtual void mark(const QScriptValue &object, int generation);
    virtual bool resolve(const QScriptValue &object, QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValue *base);
    virtual bool get(const QScriptValue &obj, const QScript::Member &m,
                     QScriptValue *result);
    virtual bool put(QScriptValue *object, const QScript::Member &member,
                     const QScriptValue &value);
    virtual bool removeMember(const QScriptValue &object,
                              const QScript::Member &member);
    virtual int extraMemberCount(const QScriptValue &object);
    virtual bool extraMember(const QScriptValue &object, int index,
                             QScript::Member *member);

private:
    Q_DISABLE_COPY(QScriptClassData)
};

#endif // QSCRIPTCLASSDATA_P_H
