/****************************************************************************
**
** Implementation of QFileInfo class
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfileinfo.h"
#include <qplatformdefs.h>
#include <qglobal.h>
#include <qatomic.h>
#include <qfileinfoengine.h>
#include "qdir.h"

#define d d_func()
#define q q_func()

//************* QFileInfoPrivate
class QFileInfoPrivate
{
    QFileInfo *q_ptr;
    Q_DECLARE_PUBLIC(QFileInfo)

protected:
    QFileInfoPrivate(QFileInfo *, const QFileInfo *copy=0);
    ~QFileInfoPrivate();

    void initFileInfoEngine(const QString &);

    uint getFileInfo(QFileInfoEngine::FileInfo) const;
    QDateTime &getFileTime(QFileInfoEngine::FileTime) const;
private:
    enum { CachedPerms=0x01, CachedTypes=0x02, CachedFlags=0x04,
           CachedMTime=0x10, CachedCTime=0x20, CachedATime=0x40,
           CachedSize =0x08 };
    struct Data {
        inline Data() : fileInfoEngine(0) { ref = 1; clear(); }
        inline ~Data() { delete fileInfoEngine; }
        inline void clear() {
            fileInfo = 0;
            cached = 0;
        }
        mutable QAtomic ref;

        QFileInfoEngine *fileInfoEngine;
        mutable QString fileName;

        mutable uint fileSize;
        mutable QDateTime fileTimes[3];
        mutable uint fileInfo;
        mutable uchar cached;
    } *data;
    inline void reset() {
        detach();
        data->clear();
    }
    void detach();
};

QFileInfoPrivate::QFileInfoPrivate(QFileInfo *qq, const QFileInfo *copy) : q_ptr(qq)
{ 
    if(copy) {
        ++copy->d->data->ref;
        data = copy->d->data;
    } else {
        data = new QFileInfoPrivate::Data;
        data->clear();
    }
}

QFileInfoPrivate::~QFileInfoPrivate()
{
    if (!--data->ref)
        delete data;
    data = 0;
    q_ptr = 0; 
}

void QFileInfoPrivate::initFileInfoEngine(const QString &file)
{
    detach();
    delete data->fileInfoEngine;
    data->fileInfoEngine = 0;
    data->clear();
    data->fileInfoEngine = new QFSFileInfoEngine(file);
    data->fileName = file;
}

void QFileInfoPrivate::detach()
{
    if (data->ref != 1) {
        QFileInfoPrivate::Data *x = data;
        data = new QFileInfoPrivate::Data;
        initFileInfoEngine(x->fileName);
        --x->ref;
    }
}

uint QFileInfoPrivate::getFileInfo(QFileInfoEngine::FileInfo request) const
{
    uint masks = 0;
    if((request & QFileInfoEngine::TypeMask) && !(data->cached & CachedTypes)) {
        data->cached |= CachedTypes;
        masks |= QFileInfoEngine::TypeMask;
    }
    if((request & QFileInfoEngine::PermsMask) && !(data->cached & CachedPerms)) {
        data->cached |= CachedPerms;
        masks |= QFileInfoEngine::PermsMask;
    }
    if((request & QFileInfoEngine::FlagsMask) && !(data->cached & CachedFlags)) {
        data->cached |= CachedFlags;
        masks |= QFileInfoEngine::FlagsMask;
    }
    if(masks) 
        data->fileInfo |= (data->fileInfoEngine->fileFlags(masks) & masks); 
    return data->fileInfo & request;
}

QDateTime &QFileInfoPrivate::getFileTime(QFileInfoEngine::FileTime request) const
{
    if(request == QFileInfoEngine::CreationTime) {
        if(data->cached & CachedCTime)
            return data->fileTimes[request];
        return (data->fileTimes[request] = data->fileInfoEngine->fileTime(request));
    }
    if(request == QFileInfoEngine::ModificationTime) {
        if(data->cached & CachedMTime)
            return data->fileTimes[request];
        return (data->fileTimes[request] = data->fileInfoEngine->fileTime(request));
    }
    if(request == QFileInfoEngine::AccessTime) {
        if(data->cached & CachedATime)
            return data->fileTimes[request];
        return (data->fileTimes[request] = data->fileInfoEngine->fileTime(request));
    }
    return data->fileTimes[0]; //cannot really happen
}

//************* QFileInfo
QFileInfo::QFileInfo() : d_ptr(new QFileInfoPrivate(this))
{
}

QFileInfo::QFileInfo(const QString &file) : d_ptr(new QFileInfoPrivate(this))
{
    d->initFileInfoEngine(file);
}


QFileInfo::QFileInfo(const QFile &file) : d_ptr(new QFileInfoPrivate(this))
{
    d->initFileInfoEngine(file.name());
}

#ifndef QT_NO_DIR
QFileInfo::QFileInfo(const QDir &dir, const QString &file) : d_ptr(new QFileInfoPrivate(this))
{
    d->initFileInfoEngine(dir.filePath(file));
}
#endif

QFileInfo::QFileInfo(const QFileInfo &fileinfo) : d_ptr(new QFileInfoPrivate(this, &fileinfo))
{

}

QFileInfo::~QFileInfo()
{
    delete d_ptr;
    d_ptr = 0;
}

QFileInfo &QFileInfo::operator=(const QFileInfo &fileinfo)
{
    QFileInfoPrivate::Data *x = fileinfo.d->data;
    ++x->ref;
    x = qAtomicSetPtr(&d->data, x);
    if (!--x->ref)
        delete x;
    return *this;
}

void
QFileInfo::setFile(const QString &file)
{
    d->initFileInfoEngine(file);
}

void
QFileInfo::setFile(const QFile &file)
{
    d->initFileInfoEngine(file.name());
}

#ifndef QT_NO_DIR
void
QFileInfo::setFile(const QDir &dir, const QString &file)
{
    d->initFileInfoEngine(dir.filePath(file));
}

QString
QFileInfo::absFilePath() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::AbsoluteName);
}

QString
QFileInfo::dirPath(bool absPath) const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(absPath ? QFileInfoEngine::AbsoluteDirPath : QFileInfoEngine::DirPath);
}

QString
QFileInfo::canonicalPath() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::Canonical);
}

bool
QFileInfo::isRelative() const
{
    if(!d->data->fileInfoEngine)
        return true;
    return d->data->fileInfoEngine->isRelativePath();
}

bool
QFileInfo::convertToAbs()
{
    if(!d->data->fileInfoEngine)
        return false;
    QString absFileName = d->data->fileInfoEngine->fileName(QFileInfoEngine::AbsoluteName);
    if(QDir::isRelativePath(d->data->fileName))
        return false;
    d->detach();
    d->data->fileName = absFileName;
    d->data->fileInfoEngine->setFileName(absFileName);
    return true;
}
#endif

bool
QFileInfo::exists() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Exists);
}

void
QFileInfo::refresh()
{
    d->reset();
}

QString
QFileInfo::filePath() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileName;
}

QString
QFileInfo::fileName() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::BaseName);
}

QString
QFileInfo::baseName(bool complete) const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    QString ret = d->data->fileInfoEngine->fileName(QFileInfoEngine::BaseName);
    int pos = complete ? ret.lastIndexOf('.') : ret.indexOf('.');
    if(pos == -1)
        return ret;
    return ret.left(pos);
}

QString
QFileInfo::extension(bool complete) const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    QString ret = d->data->fileInfoEngine->fileName(QFileInfoEngine::BaseName);
    int pos = complete ? ret.indexOf('.') : ret.lastIndexOf('.');
    if(pos == -1)
        return QString("");
    return ret.mid(pos + 1);
}

#ifndef QT_NO_DIR
QDir QFileInfo::dir(bool absPath) const
{
    return QDir(dirPath(absPath));
}
#endif

bool
QFileInfo::isReadable() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::ReadUser);
}

bool
QFileInfo::isWritable() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::WriteUser);
}

bool
QFileInfo::isExecutable() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::ExeUser);
}

bool
QFileInfo::isHidden() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Hidden);
}

bool
QFileInfo::isFile() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::File);
}

bool
QFileInfo::isDir() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Directory);
}

bool
QFileInfo::isSymLink() const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo(QFileInfoEngine::Link);
}

QString
QFileInfo::readLink() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->fileName(QFileInfoEngine::LinkName);
}

QString
QFileInfo::owner() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->owner(QFileInfoEngine::User);
}

uint
QFileInfo::ownerId() const
{
    if(!d->data->fileInfoEngine)
        return 0;
    return d->data->fileInfoEngine->ownerId(QFileInfoEngine::User);
}

QString
QFileInfo::group() const
{
    if(!d->data->fileInfoEngine)
        return QString("");
    return d->data->fileInfoEngine->owner(QFileInfoEngine::Group);
}

uint
QFileInfo::groupId() const
{
    if(!d->data->fileInfoEngine)
        return 0;
    return d->data->fileInfoEngine->ownerId(QFileInfoEngine::Group);
}

bool
QFileInfo::permission(uint permissionSpec) const
{
    if(!d->data->fileInfoEngine)
        return false;
    return d->getFileInfo((QFileInfoEngine::FileInfo)permissionSpec) == permissionSpec;
}

QIODevice::Offset
QFileInfo::size() const
{
    if(!d->data->fileInfoEngine)
        return 0;
    if(!(d->data->cached & QFileInfoPrivate::CachedSize))
        d->data->fileSize = d->data->fileInfoEngine->size();
    return d->data->fileSize;
}

QDateTime
QFileInfo::created() const
{
    if(!d->data->fileInfoEngine)
        return QDateTime();
    return d->getFileTime(QFileInfoEngine::CreationTime);
}

QDateTime
QFileInfo::lastModified() const
{
    if(!d->data->fileInfoEngine)
        return QDateTime();
    return d->getFileTime(QFileInfoEngine::ModificationTime);
}

QDateTime
QFileInfo::lastRead() const
{
    if(!d->data->fileInfoEngine)
        return QDateTime();
    return d->getFileTime(QFileInfoEngine::AccessTime);
}

void 
QFileInfo::detach()
{
    d->detach();
}
