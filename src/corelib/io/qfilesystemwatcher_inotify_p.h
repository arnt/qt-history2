#ifndef QFILESYSTEMWATCHER_INOTIFY_P_H
#define QFILESYSTEMWATCHER_INOTIFY_P_H

#include "qfilesystemwatcher_p.h"

#include <QHash>
#include <QMutex>

class QInotifyFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
    Q_OBJECT

public:
    ~QInotifyFileSystemWatcherEngine();

    static QInotifyFileSystemWatcherEngine *create();

    void run();
    
    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void stop();
    
private slots:
    void readFromInotify();
    
private:
    QInotifyFileSystemWatcherEngine(int fd);
    int inotifyFd;
    QMutex mutex;
    QHash<QString, int> pathToID;
    QHash<int, QString> idToPath;
};

#endif
