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

#include "qmutex.h"
#include "qatomic.h"

/*!
    Constructs a mutex locker based on mutex \a m.
*/
QMutexLocker::QMutexLocker(QStaticMutex &m)
{
    if (!m) { // mutex not yet initialized... do it now
        QMutex *x = new QMutex;
        if (!q_atomic_test_and_set_ptr(&m, 0, x))
            delete x; // someone beat us to it
    }
    mtx = reinterpret_cast<QMutex *>(m);
    relock();
}
