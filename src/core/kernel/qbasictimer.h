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

#ifndef QBASICTIMER_H
#define QBASICTIMER_H

#ifndef QT_H
#include "qglobal.h"
#endif

class QObject;

class Q_CORE_EXPORT QBasicTimer
{
    int id;
public:
    inline QBasicTimer():id(0){}
    inline ~QBasicTimer() { if (id) stop(); }

    inline bool isActive() const { return id != 0; }
    inline int timerId() const { return id; }

    void start(int msec, QObject *obj);
    void stop();
};


#endif // QBASICTIMER_H
