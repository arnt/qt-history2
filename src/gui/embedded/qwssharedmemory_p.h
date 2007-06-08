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

#ifndef QWSSHAREDMEMORY_P_H
#define QWSSHAREDMEMORY_P_H

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

#include "qplatformdefs.h"
#include "QtCore/qstring.h"

#if !defined(QT_NO_QWS_MULTIPROCESS)

class QWSSharedMemory {
public:

    QWSSharedMemory();
    ~QWSSharedMemory();

    void setPermissions(mode_t mode);
    int size() const;
    void *address() { return shmBase; };

    int id() const { return shmId; }

    void detach();

    bool create(int size);
    bool attach(int id);

    //bool create(int size, const QString &filename, char c = 'Q');
    //bool attach(const QString &filename, char c = 'Q');
// old API

    QWSSharedMemory(int, const QString &, char c = 'Q');
    void * base() { return address(); };

    bool create();
    void destroy();

    bool attach();

private:
    void *shmBase;
    int shmSize;
    QString shmFile;
    char character;
    int shmId;
    key_t key;
};

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSSHAREDMEMORY_P_H
