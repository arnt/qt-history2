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

#ifndef QSCRIPTMEMBER_P_H
#define QSCRIPTMEMBER_P_H

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

#include <QtCore/QHash>
#include <QtCore/QVector>

#include "qscriptvalue.h"

class QScriptNameIdImpl;

namespace QScript {

    class Member
    {
    public:
        inline void resetFlags(uint flags) { m_flags = flags; }
        inline void setFlags(uint flags) { m_flags |= flags; }
        inline void unsetFlags(uint flags) { m_flags &= ~flags; }
        inline uint flags() const { return m_flags; }
        inline bool testFlags(uint mask) const { return m_flags & mask; }

        inline bool isValid() const { return m_flags & 0x00000300; }

        inline bool isWritable() const
            { return !(m_flags & QScriptValue::ReadOnly); }
        inline bool isDeletable() const
            { return !(m_flags & QScriptValue::Undeletable); }

        inline bool dontEnum() const
            { return m_flags & QScriptValue::SkipInEnumeration; }

        inline bool isObjectProperty() const
            { return m_flags & QScriptValue::ObjectProperty; }
        inline bool isNativeProperty() const
            { return m_flags & QScriptValue::NativeProperty; }

        inline bool isUninitializedConst() const
            { return m_flags & QScriptValue::UninitializedConst; }

        inline bool isGetter() const
            { return m_flags & QScriptValue::PropertyGetter; }
        inline bool isSetter() const
            { return m_flags & QScriptValue::PropertySetter; }
        inline bool isGetterOrSetter() const
            { return m_flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter); }

        inline int id() const { return m_id; }
        inline QScriptNameIdImpl *nameId() const { return m_nameId; }

        inline bool operator==(const Member &other) const;
        inline bool operator!=(const Member &other) const;

        inline static Member invalid();
        inline void invalidate();

        inline void native(QScriptNameIdImpl *nameId, int id, uint flags);
        inline void object(QScriptNameIdImpl *nameId, int id, uint flags);

    private:
        QScriptNameIdImpl *m_nameId;
        int m_id;
        uint m_flags;
    };

    inline uint qHash(const QScript::Member &m)
    {
        return ::qHash(m.nameId());
    }

} // namespace QScript


inline QScript::Member QScript::Member::invalid()
{
    Member m;
    m.m_flags = 0;
    return m;
}

inline void QScript::Member::invalidate()
{
    m_flags = 0;
}

inline void QScript::Member::native(QScriptNameIdImpl *nameId, int id, uint flags)
{
    Q_ASSERT(! (flags & QScriptValue::ObjectProperty));

    m_nameId = nameId;
    m_id = id;
    m_flags = flags | QScriptValue::NativeProperty;
}

inline void QScript::Member::object(QScriptNameIdImpl *nameId, int id, uint flags)
{
    Q_ASSERT(! (flags & QScriptValue::NativeProperty));

    m_nameId = nameId;
    m_id = id;
    m_flags = flags | QScriptValue::ObjectProperty;
}

inline bool QScript::Member::operator==(const QScript::Member &other) const
{
    return m_nameId == other.m_nameId;
}

inline bool QScript::Member::operator!=(const QScript::Member &other) const
{
    return m_nameId != other.m_nameId;
}

#endif

