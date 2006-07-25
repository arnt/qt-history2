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

#ifndef QFILESYSTEMWATCHER_WIN_P_H
#define QFILESYSTEMWATCHER_WIN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qfilesystemwatcher_p.h"

#include <windows.h>

#include <QtCore/qdatetime.h>
#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtCore/qvector.h>

class QWindowsFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT

public:
    QWindowsFileSystemWatcherEngine();
    ~QWindowsFileSystemWatcherEngine();

    void run();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();

private:
    void wakeup();

    QMutex mutex;
    QVector<HANDLE> handles;
    int msg;
    QHash<QString, HANDLE> handleForDir;
    class PathInfo {
    public:
        QString absolutePath;
        QString path;
        bool isDir;
        QDateTime timestamp;
    };
    QHash<HANDLE, QHash<QString, PathInfo> > pathInfoForHandle;
};

#endif // QFILESYSTEMWATCHER_WIN_P_H
