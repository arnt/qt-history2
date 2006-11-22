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

#include "qfileinfogatherer_p.h"
#include <qdebug.h>
#include <qfsfileengine.h>

/*!
    Creates thread
*/
QFileInfoGatherer::QFileInfoGatherer(QObject *parent) : QThread(parent)
,abort(false), watcher(0), m_resolveSymlinks(false), m_iconProvider(&defaultProvider)
{
    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(list(const QString &)));
    connect(watcher, SIGNAL(fileChanged(const QString &)), this, SLOT(updateFile(const QString &)));
    start(LowPriority);
}

/*!
    Distroys thread
*/
QFileInfoGatherer::~QFileInfoGatherer()
{
    mutex.lock();
    abort = true;
    mutex.unlock();
    condition.wakeOne();
    wait();
}

void QFileInfoGatherer::setResolveSymlinks(bool enable)
{
    mutex.lock();
    m_resolveSymlinks = enable;
    mutex.unlock();
}

bool QFileInfoGatherer::resolveSymlinks() const
{
    return m_resolveSymlinks;
}

void QFileInfoGatherer::setIconProvider(QFileIconProvider *provider)
{
    mutex.lock();
    m_iconProvider = provider;
    mutex.unlock();
}

QFileIconProvider *QFileInfoGatherer::iconProvider() const
{
    return m_iconProvider;
}

/*!
    Fetch extended information for all \a files in \a path

    \sa updateFile(), update(), resolvedName()
*/
void QFileInfoGatherer::fetchExtendedInformation(const QString &path, const QStringList &files)
{
    mutex.lock();
    this->path.push(path);
    this->files.push(files);
    mutex.unlock();
    condition.wakeAll();
}

/*!
    Fetch extended information for all \a filePath

    \sa fetchExtendedInformation()
*/
void QFileInfoGatherer::updateFile(const QString &filePath)
{
    QString dir = filePath.mid(0, filePath.lastIndexOf(QDir::separator()));
    QString fileName = filePath.mid(dir.length() + 1);
    fetchExtendedInformation(dir, QStringList(fileName));
}

/*
    List all files in \a directoryPath

    \sa listed()
*/
void QFileInfoGatherer::clear()
{
    mutex.lock();
    watcher->removePaths(watcher->files());
    watcher->removePaths(watcher->directories());
    mutex.unlock();
}

/*
    List all files in \a directoryPath

    \sa listed()
*/
void QFileInfoGatherer::list(const QString &directoryPath)
{
    mutex.lock();
    this->directoryPath.push(directoryPath);
    mutex.unlock();
    condition.wakeAll();
}

/*
    Until aborted wait to fetch a directory or files
*/
void QFileInfoGatherer::run()
{
    forever {
        bool listFiles = false;
        bool updateFiles = false;
        mutex.lock();
        if (abort)
            return;
        if (this->path.isEmpty() && directoryPath.isEmpty())
            condition.wait(&mutex);
        QString path;
        QStringList list;
        if (!this->path.isEmpty()) {
            path = this->path.pop();
            list = this->files.pop();
            updateFiles = true;
        }
        QString directoryPath;
        if (!this->directoryPath.isEmpty()) {
            directoryPath = this->directoryPath.pop();
            listFiles = true;
        }
        mutex.unlock();

        if (updateFiles) getFileInfos(path, list);
        if (listFiles) getDirList(directoryPath);
    }
}

/*
    Get specific file info's, batch the files so update when we have 100 items and every 200ms after that
 */
void QFileInfoGatherer::getFileInfos(const QString &path, const QStringList &files)
{
    if (files.isEmpty())
        return;

    // List extended information about drives
    if (path.isEmpty()){
        QFileInfoList infoList = QDir::drives();
        QList<QPair<QString,QExtendedInformation> > updatedFiles;
        for (int i = 0; i < infoList.count(); ++i) {
            QExtendedInformation info;
            QFileInfo fileInfo = infoList.at(i);
            QFSFileEngine fe(fileInfo.absoluteFilePath());
            info.caseSensitive = fe.caseSensitive();
            info.size = fileInfo.size();
            info.lastModified = fileInfo.lastModified();
            info.permissions = fileInfo.permissions();
            if (fileInfo.isDir()) info.fileType = QExtendedInformation::Dir;
            if (fileInfo.isFile()) info.fileType = QExtendedInformation::File;
            info.isHidden = false; // fileInfo.isHidden(); BUG windows file engine says drives are hidden
            info.isSymLink = fileInfo.isSymLink();
            info.icon = m_iconProvider->icon(fileInfo);
            info.displayType = m_iconProvider->type(fileInfo);

            QString driveName = translateDriveName(infoList.at(i));
            updatedFiles.append(QPair<QString,QExtendedInformation>(driveName, info));
        }
        emit updates(path, updatedFiles);
        return;
    }

    int base = QTime::currentTime().msec();
    QFileInfo fileInfo;
    QList<QPair<QString,QExtendedInformation> > updatedFiles;
    bool firstBatch = true;
    QStringList watchedFiles = watcher->files();
    for (int i = 0; !abort && i < files.count(); ++i) {
        QString fullFilePath = path + QDir::separator() + files.at(i);
        fileInfo.setFile(fullFilePath);

        QExtendedInformation info;
        QFSFileEngine fe(fullFilePath);
        info.caseSensitive = fe.caseSensitive();
        info.size = fileInfo.size();

        if (!fileInfo.exists() && !fileInfo.isSymLink()) {
            info.size = -1;
            watcher->removePath(fullFilePath);
        } else {
            //qDebug() << "adding" << fileInfo.fileName();
            if (fileInfo.exists() && fileInfo.isReadable() && !watchedFiles.contains(fullFilePath)) // <- unfortunettly addPath will always add even if it is already there
                watcher->addPath(fullFilePath); // <- add path unfortunettly creates a QFileInfo also
        }
        info.lastModified = fileInfo.lastModified();
        info.permissions = fileInfo.permissions();
        if (fileInfo.isDir()) info.fileType = QExtendedInformation::Dir;
        if (fileInfo.isFile()) info.fileType = QExtendedInformation::File;

        if (fileInfo.isSymLink() && m_resolveSymlinks) {
            QFileInfo resolvedInfo(fileInfo.canonicalFilePath());
            if (resolvedInfo.exists()) {
                emit nameResolved(files.at(i), resolvedInfo.fileName());
            } else {
                info.fileType = QExtendedInformation::System;
            }
        }

        if (!fileInfo.exists() && fileInfo.isSymLink()) {
            info.fileType = QExtendedInformation::System;
        }
        info.isHidden = fileInfo.isHidden();
        info.isSymLink = fileInfo.isSymLink();
        info.icon = m_iconProvider->icon(fileInfo);
        info.displayType = m_iconProvider->type(fileInfo);

        updatedFiles.append(QPair<QString,QExtendedInformation>(files.at(i), info));

        // batch
        int current = QTime::currentTime().msec();
        if (firstBatch && updatedFiles.count() > 50 || current - base > 200) {
            emit updates(path, updatedFiles);
            updatedFiles.clear();
            firstBatch = false;
            base = current;
        }
    }
    emit updates(path, updatedFiles);
}

QString QFileInfoGatherer::translateDriveName(const QFileInfo &drive)
{
    QString driveName = drive.absoluteFilePath();
#ifdef Q_OS_WIN
    if (driveName.startsWith(QLatin1Char('/'))) // UNC host
        return drive.fileName();
    if (driveName.endsWith(QLatin1Char('/')))
        driveName.chop(1);
#endif
    return driveName;
}

/*!
    get the full list of files in a directory
 */
void QFileInfoGatherer::getDirList(const QString &directoryPath)
{
    QDir dir(directoryPath);
    if (!dir.exists())
        return;
    if (!watcher->directories().contains(directoryPath) && !directoryPath.isEmpty()) {
        watcher->addPath(directoryPath);
    }

    // list drives
    if (directoryPath.isEmpty()) {
        QFileInfoList infoList = QDir::drives();
        QStringList drives;
        for (int i = 0; i < infoList.count(); ++i)
            drives.append(translateDriveName(infoList.at(i)));
        emit newListOfFiles(directoryPath, drives);
        return;
    }

    QStringList list = dir.entryList(QDir::AllEntries | QDir::System | QDir::Hidden);

    // It looks like a bug that path sometimes returns /.  QDir::path() should strip the /.
    QString dirPath = dir.path();
    if (dirPath.right(2) == "/.")
        dirPath = dirPath.left(dirPath.length() - 2);
    emit newListOfFiles(dirPath, list);
}

