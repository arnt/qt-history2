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

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

#include <limits.h>

class QWaitConditionPrivate;
class QMutex;

class Q_CORE_EXPORT QWaitCondition
{
public:
    QWaitCondition();
    virtual ~QWaitCondition();

    // default argument causes thread to block indefinately
    bool wait(unsigned long time = ULONG_MAX);
    bool wait(QMutex *mutex, unsigned long time = ULONG_MAX);

    void wakeOne();
    void wakeAll();

private:
    QWaitConditionPrivate * d;

#if defined(Q_DISABLE_COPY)
    QWaitCondition(const QWaitCondition &);
    QWaitCondition &operator=(const QWaitCondition &);
#endif
};

#endif // QWAITCONDITION_H
