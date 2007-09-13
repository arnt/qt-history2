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

#ifndef QWSLOCK_P_H
#define QWSLOCK_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//

#include <qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_MULTIPROCESS

class QWSLock
{
public:
    enum LockType { BackingStore, Communication, RegionEvent };

    QWSLock();
    QWSLock(int lockId);
    ~QWSLock();

    bool lock(LockType type, int timeout = -1);
    void unlock(LockType type);
    bool wait(LockType type, int timeout = -1);
    bool hasLock(LockType type);
    int id() const { return semId; }

private:
    int semId;
    int lockCount[2];
};


QT_END_NAMESPACE
#endif // QT_NO_QWS_MULTIPROCESS
#endif // QWSLOCK_P_H
