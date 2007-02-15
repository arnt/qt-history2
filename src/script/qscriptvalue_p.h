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

#include "qscriptvaluefwd_p.h"

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

inline QScriptValuePrivate::QScriptValuePrivate()
{
    ref.init();
}

inline QScriptValuePrivate::QScriptValuePrivate(const QScriptValueImpl &value)
    : value(value)
{
    ref.init();
}

inline QScriptValuePrivate *QScriptValuePrivate::get(const QScriptValue &value)
{
    return const_cast<QScriptValuePrivate*>(value.d_func());
}

inline QScriptValueImpl QScriptValuePrivate::valueOf(const QScriptValue &value)
{
    const QScriptValuePrivate *p = value.d_func();
    if (!p)
        return QScriptValueImpl();
    return p->value;
}

inline void QScriptValuePrivate::init(QScriptValue &value, QScriptValuePrivate *p)
{
    Q_ASSERT(value.d_ptr == 0);
    value.d_ptr = p;
    value.d_ptr->ref.ref();
}

inline QScriptValueImplList QScriptValuePrivate::toImplList(const QScriptValueList &lst)
{
    QScriptValueImplList result;
    QScriptValueList::const_iterator it;
    for (it = lst.constBegin(); it != lst.constEnd(); ++it)
        result.append((*it).d_ptr->value);
    return result;
}

#endif
