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

#ifndef QSCRIPTCONTEXTINFO_P_H
#define QSCRIPTCONTEXTINFO_P_H

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

#include "qscriptcontextinfo.h"

#ifndef QT_NO_SCRIPT

#include <QtCore/qatomic.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QScriptContext;

class QScriptContextInfo;
class QScriptContextInfoPrivate
{
    Q_DECLARE_PUBLIC(QScriptContextInfo)
public:
    QScriptContextInfoPrivate(QScriptContext *context);
    ~QScriptContextInfoPrivate();

    int lineNumber;
    int columnNumber;
    QString fileName;

    QString functionName;
    QScriptContextInfo::FunctionType functionType;

    int functionStartLineNumber;
    int functionEndLineNumber;
    int functionMetaIndex;

    QStringList parameterNames;

    QBasicAtomicInt ref;

    QScriptContextInfo *q_ptr;
};

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif
