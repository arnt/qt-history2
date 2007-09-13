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

#ifndef QSCRIPTENGINEAGENT_P_H
#define QSCRIPTENGINEAGENT_P_H

#include <QtCore/qobjectdefs.h>

#ifndef QT_NO_SCRIPT

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

QT_BEGIN_NAMESPACE

class QScriptEngine;

class QScriptEngineAgent;
class QScriptEngineAgentPrivate
{
    Q_DECLARE_PUBLIC(QScriptEngineAgent)
public:
    QScriptEngineAgentPrivate();
    ~QScriptEngineAgentPrivate();

    QScriptEngine *engine;

    QScriptEngineAgent *q_ptr;
};

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif
