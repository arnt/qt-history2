/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWAITCONDITION_H
#define QWAITCONDITION_H

#include "QtCore/qglobal.h"

#include <limits.h>

QT_MODULE(Core)

class QWaitConditionPrivate;
class QMutex;

class Q_CORE_EXPORT QWaitCondition
{
public:
    QWaitCondition();
    ~QWaitCondition();

    bool wait(QMutex *mutex, unsigned long time = ULONG_MAX);

    void wakeOne();
    void wakeAll();

private:
    Q_DISABLE_COPY(QWaitCondition)

    QWaitConditionPrivate * d;
};

#endif // QWAITCONDITION_H
