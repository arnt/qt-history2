/****************************************************************************
** $Id: $
**
** Definition of QThread class
**
** Created : 931107
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTHREAD_H
#define QTHREAD_H

#ifndef QT_H
#  include "qwindowdefs.h"
#endif // QT_H

#include <limits.h>

struct QThreadInstance;
class QObject;
class QEvent;

class Q_EXPORT QThread
{
public:
#ifndef QT_NO_COMPAT
    static void postEvent( QObject *,QEvent * );
#endif

    static Qt::HANDLE currentThread();

    static void initialize();
    static void cleanup();

    static void exit();

    QThread( unsigned int stackSize = 0 );
    virtual ~QThread();

    // default argument causes thread to block indefinately
    bool wait( unsigned long time = ULONG_MAX );

    enum Priority {
	IdlePriority,

	LowestPriority,
	LowPriority,
	NormalPriority,
	HighPriority,
	HighestPriority,

	TimeCriticalPriority,

	InheritPriority
    };

    void start( Priority = InheritPriority );
    void terminate();

    bool finished() const;
    bool running() const;

protected:
    virtual void run() = 0;

    static void sleep( unsigned long );
    static void msleep( unsigned long );
    static void usleep( unsigned long );

private:
    QThreadInstance * d;
    friend struct QThreadInstance;

#if defined(Q_DISABLE_COPY)
    QThread( const QThread & );
    QThread &operator=( const QThread & );
#endif // Q_DISABLE_COPY
};

#endif // QTHREAD_H
