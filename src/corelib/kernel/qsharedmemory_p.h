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

#ifndef QSHAREDMEMORY_P_H
#define QSHAREDMEMORY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_qws.cpp and qgfxvnc_qws.cpp.  This header file may
// change from version to version without notice, or even be removed.
//
// We mean it.
//

#if !defined (QT_QWS_NO_SHM)

#include "qplatformdefs.h"
#include "QtCore/qstring.h"

class Q_CORE_EXPORT QSharedMemory {
public:

    QSharedMemory();
    ~QSharedMemory();

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

    QSharedMemory(int, const QString &, char c = 'Q');
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

#endif

#endif // QSHAREDMEMORY_P_H
