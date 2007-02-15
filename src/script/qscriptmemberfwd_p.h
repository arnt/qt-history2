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

#ifndef QSCRIPTMEMBERFWD_P_H
#define QSCRIPTMEMBERFWD_P_H

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

class QScriptNameIdImpl;

namespace QScript {

    class Member
    {
    public:
        inline void resetFlags(uint flags);
        inline void setFlags(uint flags);
        inline void unsetFlags(uint flags);
        inline uint flags() const;
        inline bool testFlags(uint mask) const;

        inline bool isValid() const;

        inline bool isWritable() const;
        inline bool isDeletable() const;

        inline bool dontEnum() const;

        inline bool isObjectProperty() const;
        inline bool isNativeProperty() const;

        inline bool isUninitializedConst() const;

        inline bool isGetter() const;
        inline bool isSetter() const;
        inline bool isGetterOrSetter() const;

        inline int id() const;
        inline QScriptNameIdImpl *nameId() const;

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

} // namespace QScript

#endif
