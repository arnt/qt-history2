#ifndef QFILESYSTEMWATCHER_P_H
#define QFILESYSTEMWATCHER_P_H

#include "qfilesystemwatcher.h"

#include <private/qobject_p.h>

#include <QStringList>
#include <QThread>

class QFileSystemWatcherEngine : public QThread
{
    Q_OBJECT

protected:
    inline QFileSystemWatcherEngine()
    {
        moveToThread(this);
    }

public:
    // fills \a files and \a directires with the \a paths it could
    // watch, and returns a list of paths this engine could not watch
    virtual QStringList addPaths(const QStringList &paths,
                                 QStringList *files,
                                 QStringList *directories) = 0;
    // removes \a paths from \a files and \a directories, and returns
    // a list of paths this engine does not know about (either addPath
    // failed or wasn't called)
    virtual QStringList removePaths(const QStringList &paths,
                                    QStringList *files,
                                    QStringList *directories) = 0;

    virtual void stop() = 0;

Q_SIGNALS:
    void fileChanged(const QString &path, bool removed);
    void directoryChanged(const QString &path, bool removed);
};

class QFileSystemWatcherPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QFileSystemWatcher)

    static QFileSystemWatcherEngine *createNativeEngine();

public:
    QFileSystemWatcherPrivate();
    void init();

    QFileSystemWatcherEngine *native, *poller;
    QStringList files, directories;

    // private slots
    void _q_fileChanged(const QString &path, bool removed);
    void _q_directoryChanged(const QString &path, bool removed);
};

#endif // QFILESYSTEMWATCHER_P_H
