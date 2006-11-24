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

#ifndef QFILEINFOGATHERER_H
#define QFILEINFOGATHERER_H

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qfilesystemwatcher.h>
#if QT_VERSION >= 0x040300
#include <qfileiconprovider.h>
#else
#include <qdirmodel.h>
#endif
#include <qpair.h>
#include <qdatetime.h>
#include <qstack.h>
#include <qdir.h>

class QExtendedInformation {
public:
    enum Type { Dir, File, System };

    QExtendedInformation() : size(0), fileType(System), isHidden(false),
                             isSymLink(false), caseSensitive(true) {}

    int size;
    QString displayType;
    QIcon icon;
    QDateTime lastModified;
    QFile::Permissions permissions;
    Type fileType;
    bool isHidden : 1;
    bool isSymLink : 1;
    bool caseSensitive : 1;

    inline bool isDir() { return fileType == Dir; }
    inline bool isFile() { return fileType == File; }
    inline bool isSystem() { return fileType == System; }

    bool operator ==(const QExtendedInformation &fileInfo) const {
       return fileInfo.size == size
       && fileInfo.displayType == displayType
       && fileInfo.lastModified == lastModified
       && fileInfo.permissions == permissions
       && fileInfo.fileType == fileType
       && fileInfo.isHidden == isHidden
       && fileInfo.isSymLink == isSymLink
       && fileInfo.caseSensitive == caseSensitive;
    }
    void operator =(const QExtendedInformation &fileInfo) {
        size = fileInfo.size;
        displayType = fileInfo.displayType;
        icon = fileInfo.icon;
        lastModified = fileInfo.lastModified;
        permissions = fileInfo.permissions;
        fileType = fileInfo.fileType;
        isHidden = fileInfo.isHidden;
        isSymLink = fileInfo.isSymLink;
        caseSensitive = fileInfo.caseSensitive;
    }
};

class QFileIconProvider;

class QFileInfoGatherer : public QThread
{
Q_OBJECT

Q_SIGNALS:
    void updates(const QString &directory, const QList<QPair<QString, QExtendedInformation> > &updates);
    void newListOfFiles(const QString &directory, const QStringList &listOfFiles);
    void nameResolved(const QString &fileName, const QString &resolvedName);

public:
    QFileInfoGatherer(QObject *parent = 0);
    ~QFileInfoGatherer();

    void clear();

public Q_SLOTS:
    void list(const QString &directoryPath);
    void fetchExtendedInformation(const QString &path, const QStringList &files);
    void updateFile(const QString &path);
    void setResolveSymlinks(bool enable);
    bool resolveSymlinks() const;
    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

protected:
    void run();
    void getFileInfos(const QString &path, const QStringList &files);
    void getDirList(const QString &directoryPath);

private:
    QString translateDriveName(const QFileInfo &drive);
    QFile::Permissions translatePermissions(const QFileInfo &fileInfo);

    QMutex mutex;
    QWaitCondition condition;
    bool abort;

    QStack<QString> path;
    QStack<QStringList> files;

    QStack<QString> directoryPath;
    QFileSystemWatcher *watcher;
    bool m_resolveSymlinks;
    QFileIconProvider *m_iconProvider;
    QFileIconProvider defaultProvider;
#ifndef Q_OS_WIN
    uint userId;
    uint groupId;
#endif
};

#endif // QFILEINFOGATHERER_H

