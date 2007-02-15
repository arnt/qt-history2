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

#include <QtCore/qshareddata.h>

class QScriptValueImpl;
class QScriptNameIdImpl;

namespace QScript {
    class Member;
}

class QScriptClassData: public QSharedData
{
protected:
    QScriptClassData() {}

public:
    virtual ~QScriptClassData() {}

    virtual void mark(const QScriptValueImpl &object, int generation);
    virtual bool resolve(const QScriptValueImpl &object, QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValueImpl *base);
    virtual bool get(const QScriptValueImpl &obj, const QScript::Member &m,
                     QScriptValueImpl *result);
    virtual bool put(QScriptValueImpl *object, const QScript::Member &member,
                     const QScriptValueImpl &value);
    virtual bool removeMember(const QScriptValueImpl &object,
                              const QScript::Member &member);
    virtual int extraMemberCount(const QScriptValueImpl &object);
    virtual bool extraMember(const QScriptValueImpl &object, int index,
                             QScript::Member *member);

private:
    Q_DISABLE_COPY(QScriptClassData)
};

#endif // QSCRIPTCLASSDATA_P_H
