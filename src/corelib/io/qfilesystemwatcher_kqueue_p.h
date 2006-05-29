/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FILEWATCHER_KQUEUE_P_H
#define FILEWATCHER_KQUEUE_P_H

#include "qfilesystemwatcher_p.h"

#include <QHash>
#include <QMutex>
#include <QThread>
#include <QVector>

struct kevent;

class QKqueueFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT
public:
    ~QKqueueFileSystemWatcherEngine();

    static QKqueueFileSystemWatcherEngine *create();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();

private:
    QKqueueFileSystemWatcherEngine(int kqfd);

    void run();

    int kqfd;
    int kqpipe[2];

    QMutex mutex;
    QHash<QString, int> pathToID;
    QHash<int, QString> idToPath;
};

#endif // FILEWATCHER_KQUEUE_P_H
