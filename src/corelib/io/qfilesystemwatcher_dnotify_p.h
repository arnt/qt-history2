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

#ifndef QFILESYSTEMWATCHER_DNOTIFY_P_H
#define QFILESYSTEMWATCHER_DNOTIFY_P_H

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

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qmutex.h>
#include <qhash.h>
#include <qdatetime.h>
#include <qfile.h>

class QDnotifyFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT

public:
    virtual ~QDnotifyFileSystemWatcherEngine();

    static QDnotifyFileSystemWatcherEngine *create();

    void run();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();

private Q_SLOTS:
    void refresh(int);

private:
    struct Directory {
        Directory() : fd(0), parentFd(0), isMonitored(false) {}
        Directory(const Directory &o) : path(o.path), 
                                        fd(o.fd),
                                        parentFd(o.parentFd),
                                        isMonitored(o.isMonitored), 
                                        files(o.files) {}
        QString path;
        int fd;
        int parentFd;
        bool isMonitored;

        struct File {
            File() {}
            File(const File &o) : name(o.name), 
                                  ownerId(o.ownerId),
                                  groupId(o.groupId),
                                  permissions(o.permissions),
                                  lastWrite(o.lastWrite) {}
            QString name;

            bool updateInfo();

            uint ownerId;
            uint groupId;
            QFile::Permissions permissions;
            QDateTime lastWrite;
        };

        QList<File> files;
    };

    QDnotifyFileSystemWatcherEngine();
  
    QMutex mutex;
    QHash<QString, int> pathToFD;
    QHash<int, Directory> fdToDirectory;
    QHash<int, int> parentToFD;
};


#endif // QT_NO_FILESYSTEMWATCHER
#endif // QFILESYSTEMWATCHER_DNOTIFY_P_H
