/****************************************************************************
**
** Definition of QFileEngine and QFSFileEngine classes.
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
#ifndef __QFILEENGINE_H__
#define __QFILEENGINE_H__

#include "qioengine.h"
#include "qdir.h"

class QFileEnginePrivate;
class Q_CORE_EXPORT QFileEngine : public QIOEngine
{
    Q_DECLARE_PRIVATE(QFileEngine)
public:
    QFileEngine(QFileEnginePrivate &);
    virtual ~QFileEngine();

    static QFileEngine *createFileEngine(const QString &file);
    virtual void setFileName(const QString &file) = 0;

    virtual bool remove() = 0;
    virtual bool rename(const QString &newName) = 0;

    virtual int handle() const = 0;

    virtual uchar *map(Q_LONG len) = 0; //can we implement an mmap?

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const = 0;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const = 0;

    virtual QStringList entryList(int filterSpec, const QStringList &filters) const = 0;

    virtual bool caseSensitive() const = 0;

    virtual bool isRoot() const = 0;

    virtual bool isRelativePath() const = 0;

    enum FileInfo { 
        //perms (overlaps the QFileInfo permissions)
        ReadOwner = 0x4000, WriteOwner = 0x2000, ExeOwner = 0x1000,
        ReadUser  = 0x0400, WriteUser  = 0x0200, ExeUser  = 0x0100,
        ReadGroup = 0x0040, WriteGroup = 0x0020, ExeGroup = 0x0010,
        ReadOther = 0x0004, WriteOther = 0x0002, ExeOther = 0x0001,

        //types
        Link      = 0x10000, 
        File      = 0x20000, 
        Directory = 0x40000,

        //flags
        Hidden     = 0x0100000,
        Exists     = 0x0400000,

        //masks
        PermsMask  = 0x0000FFFF,
        TypeMask   = 0x000F0000,
        FlagsMask  = 0x0FF00000,
        FileInfoAll = FlagsMask | PermsMask | TypeMask
    };
    virtual uint fileFlags(uint type=FileInfoAll) const = 0;

    enum FileName { DefaultName, BaseName, PathName, AbsoluteName, AbsolutePathName, LinkName, CanonicalName };
    virtual QString fileName(FileName file=DefaultName) const = 0;

    enum FileOwner { User, Group };
    virtual uint ownerId(FileOwner) const = 0;
    virtual QString owner(FileOwner) const = 0;

    enum FileTime { CreationTime, ModificationTime, AccessTime };
    virtual QDateTime fileTime(FileTime time) const = 0;
};

class QFileEngineHandler
{
protected:
    QFileEngineHandler();
    virtual ~QFileEngineHandler();
    virtual QFileEngine *createFileEngine(const QString &path) = 0;

private:
    friend class QFileEngine;
};

class QFSFileEnginePrivate;
class QFSFileEngine : public QFileEngine
{
private:
    Q_DECLARE_PRIVATE(QFSFileEngine)
public:
    QFSFileEngine(const QString &file);
    QFSFileEngine();

    virtual void setFileName(const QString &file);

    virtual bool open(int flags);
    virtual bool close();
    virtual void flush();
    virtual QIODevice::Offset size() const;
    virtual QIODevice::Offset at() const;
    virtual bool atEnd() const;
    virtual bool seek(QIODevice::Offset);
    virtual int ungetch(int);
    virtual Q_LONG readBlock(char *data, Q_LONG maxlen);
    virtual Q_LONG writeBlock(const char *data, Q_LONG len);
    virtual QString errorMessage() const;
    virtual QIODevice::Status errorStatus() const;
    virtual Type type() const;

    virtual bool remove();
    virtual bool rename(const QString &newName);

    virtual bool isSequential() const;

    virtual int handle() const;

    virtual uchar *map(Q_LONG len);

    virtual bool mkdir(const QString &dirName, QDir::Recursion recurse) const;
    virtual bool rmdir(const QString &dirName, QDir::Recursion recurse) const;

    virtual QStringList entryList(int filterSpec, const QStringList &filters) const;

    virtual bool caseSensitive() const;

    virtual bool isRoot() const;

    virtual bool isRelativePath() const;

    virtual uint fileFlags(uint type) const;

    virtual QString fileName(QFileEngine::FileName file) const;

    virtual uint ownerId(QFileEngine::FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual QDateTime fileTime(FileTime time) const;

    //FS only!!
    bool open(int flags, int fd); 
    static bool setCurrentPath(const QString &path);
    static QString currentPath(const QString &path=QString::null);
    static QString homePath();
    static QString rootPath();
    static QString tempPath();
    static QFileInfoList drives();
};
#endif /* __QFILEENGINE_H__ */
