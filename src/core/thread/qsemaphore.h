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

#ifndef QSEMAPHORE_H
#define QSEMAPHORE_H

#include "qglobal.h"

class QSemaphorePrivate;

class Q_CORE_EXPORT QSemaphore
{
public:
    QSemaphore(int);
    virtual ~QSemaphore();

    int available() const;
    int total() const;

    // postfix operators
    int operator++(int);
    int operator--(int);

    int operator+=(int);
    int operator-=(int);

    bool tryAccess(int);

private:
    Q_DISABLE_COPY(QSemaphore)

    QSemaphorePrivate *d;
};

#endif // QSEMAPHORE_H
