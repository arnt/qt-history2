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

#ifndef QSCRIPTVALUEFWD_P_H
#define QSCRIPTVALUEFWD_P_H

#include <QtCore/qatomic.h>

#ifndef QT_NO_SCRIPT

#include "qscriptvalueimplfwd_p.h"

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
    inline QScriptValuePrivate();

    static inline QScriptValuePrivate *create();

    static inline QScriptValuePrivate *get(const QScriptValue &value);

    static inline QScriptValueImpl valueOf(const QScriptValue &value);

    static inline void init(QScriptValue &value, QScriptValuePrivate *p);

    static inline QScriptValueImplList toImplList(const QScriptValueList &lst);

    QScriptValueImpl value;
    QBasicAtomic ref;
};

#endif // QT_NO_SCRIPT
#endif
