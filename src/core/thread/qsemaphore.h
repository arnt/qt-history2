/****************************************************************************
**
** Definition of QSemaphore class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSEMAPHORE_H
#define QSEMAPHORE_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class QSemaphorePrivate;

class Q_CORE_EXPORT QSemaphore
{
public:
    QSemaphore( int );
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
    QSemaphorePrivate *d;

#if defined(Q_DISABLE_COPY)
    QSemaphore(const QSemaphore &);
    QSemaphore &operator=(const QSemaphore &);
#endif
};

#endif // QSEMAPHORE_H
