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

#ifndef QSCRIPTVALUE_P_H
#define QSCRIPTVALUE_P_H

#include <QtCore/qatomic.h>
#include "qscriptvalueimpl_p.h"

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

class QScriptValuePrivate
{
public:
    inline QScriptValuePrivate() { ref.init(); }
    inline QScriptValuePrivate(const QScriptValueImpl &value)
        : value(value) { ref.init(); }

    static inline QScriptValuePrivate *get(const QScriptValue &value)
    { return const_cast<QScriptValuePrivate*>(value.d_func()); }

    static inline QScriptValueImpl valueOf(const QScriptValue &value)
    {
        const QScriptValuePrivate *p = value.d_func();
        if (!p)
            return QScriptValueImpl();
        return p->value;
    }

    static inline void init(QScriptValue &value, QScriptValuePrivate *p)
    {
        Q_ASSERT(value.d_ptr == 0);
        value.d_ptr = p;
        value.d_ptr->ref.ref();
    }

    static inline QScriptValueImplList toImplList(const QScriptValueList &lst)
    {
        QScriptValueImplList result;
        QScriptValueList::const_iterator it;
        for (it = lst.constBegin(); it != lst.constEnd(); ++it)
            result.append((*it).d_ptr->value);
        return result;
    }

    QScriptValueImpl value;
    QBasicAtomic ref;
};

#endif
