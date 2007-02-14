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

#ifndef QSCRIPTOBJECT_P_H
#define QSCRIPTOBJECT_P_H

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

#include "qscriptbuffer_p.h"
#include "qscriptmember_p.h"
#include "qscriptvalueimpl_p.h"

namespace QScript
{
    class Member;
} // namespace QScript


class QScriptObjectData: public QSharedData
{
protected:
    QScriptObjectData() {}

public:
    virtual ~QScriptObjectData() {}

private:
    Q_DISABLE_COPY(QScriptObjectData)
};

class QScriptObject
{
public:
    inline void reset();
    inline void finalize();

    inline bool findMember(QScriptNameIdImpl *nameId,
                           QScript::Member *m) const
    {
        const QScript::Member *members = m_members.constData();
        const int size = m_members.size();

        const QScript::Member *first = &members[-1];
        const QScript::Member *last = &members[size - 1];

        for (const QScript::Member *it = last; it != first; --it) {
            if (it->nameId() == nameId && it->isValid()) {
                *m = *it;
                return true;
            }
        }

        return false;
    }

    // assumes that m already points to the setter
    inline bool findGetter(QScript::Member *m) const
    {
        const QScript::Member *members = m_members.constData();
        const QScript::Member *first = &members[-1];
        const QScript::Member *last = &members[m->id() - 1];

        for (const QScript::Member *it = last; it != first; --it) {
            if (it->nameId() == m->nameId() && it->isValid() && it->isGetter()) {
                *m = *it;
                return true;
            }
        }

        return false;
    }

    // assumes that m already points to the getter
    inline bool findSetter(QScript::Member *m) const
    {
        const QScript::Member *members = m_members.constData();
        const QScript::Member *first = &members[-1];
        const QScript::Member *last = &members[m->id() - 1];

        for (const QScript::Member *it = last; it != first; --it) {
            if (it->nameId() == m->nameId() && it->isValid() && it->isSetter()) {
                *m = *it;
                return true;
            }
        }

        return false;
    }

    inline int memberCount() const
    {
        return m_members.size();
    }

    inline void createMember(QScriptNameIdImpl *nameId,
                             QScript::Member *member, uint flags)
    {
        member->object(nameId, m_objects.size(), flags);
        m_members.append(*member);
        m_objects.append(QScriptValueImpl());
    }

    inline void member(int index, QScript::Member *member) {
        *member = m_members[index];
    }

    inline void put(const QScript::Member &m, const QScriptValueImpl &v) {
        m_objects[m.id()] = v;
    }

    inline QScriptValueImpl &reference(const QScript::Member &m) {
        return m_objects[m.id()];
    }

    inline void get(const QScript::Member &m, QScriptValueImpl *v) {
        Q_ASSERT(m.isObjectProperty());
        *v = m_objects[m.id()];
    }

    inline void removeMember(const QScript::Member &member) {
        m_members[member.id()].invalidate();
        m_objects[member.id()].invalidate();
    }

    QScriptValueImpl m_prototype;
    QScriptValueImpl m_scope;
    QScriptValueImpl m_internalValue; // [[value]]
    QExplicitlySharedDataPointer<QScriptObjectData> m_data;
    QScript::Buffer<QScript::Member> m_members;
    QScript::Buffer<QScriptValueImpl> m_objects;
};

inline void QScriptObject::finalize()
{
    m_data = 0;
}

inline void QScriptObject::reset()
{
    m_prototype.invalidate();
    m_scope.invalidate();
    m_internalValue.invalidate();
    m_members.resize(0);
    m_objects.resize(0);
    m_data = 0;
}

#endif

