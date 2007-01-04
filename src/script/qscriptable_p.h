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

#ifndef QSCRIPTABLE_P_H
#define QSCRIPTABLE_P_H

#include <QtCore/qobjectdefs.h>

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

#ifndef QT_NO_QOBJECT

class QScriptEngine;

class QScriptablePrivate
{
    Q_DECLARE_PUBLIC(QScriptable)
public:
    inline QScriptablePrivate()
        : engine(0)
        { }

    static inline QScriptablePrivate *get(QScriptable *q)
        { return q->d_func(); }

    QScriptEngine *engine;

    QScriptable *q_ptr;
};

#endif // QT_NO_QOBJECT

#endif
