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

#ifndef QREADWRITELOCK_P_H
#define QREADWRITELOCK_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the implementation.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

struct QReadWriteLockPrivate
{
    QReadWriteLockPrivate()
        : accessCount(0), currentWriter(0), waitingReaders(0), waitingWriters(0)
    { }

    QMutex mutex;
    QWaitCondition readerWait;
    QWaitCondition writerWait;

    int accessCount;
    Qt::HANDLE currentWriter;
    int waitingReaders;
    int waitingWriters;
};

#endif // QREADWRITELOCK_P_H
