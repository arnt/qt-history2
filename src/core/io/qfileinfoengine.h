/****************************************************************************
**
** Definition of QFileInfoEngine classes.
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
#ifndef __QFILEINFOENGINE_H__
#define __QFILEINFOENGINE_H__

#include "qfileinfo.h"

class QFileInfoEnginePrivate;
class QFileInfoEngine
{
protected:
    QFileInfoEnginePrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QFileInfoEngine)
public:
    QFileInfoEngine(QFileInfoEnginePrivate &);
    virtual ~QFileInfoEngine();

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

    virtual void setFileName(const QString &file) = 0;

    enum FileName { Default, BaseName, DirPath, AbsoluteName, AbsoluteDirPath, LinkName, Canonical };
    virtual QString fileName(FileName file=Default) const = 0;
    virtual bool isRelativePath() const = 0;

    enum FileOwner { User, Group };
    virtual uint ownerId(FileOwner) const = 0;
    virtual QString owner(FileOwner) const = 0;

    virtual uint size() const = 0;

    enum FileTime { CreationTime, ModificationTime, AccessTime };
    virtual QDateTime fileTime(FileTime time) const = 0;
};

class QFileInfoEngineHandler
{
protected:
    QFileInfoEngineHandler();
    virtual ~QFileInfoEngineHandler();

    virtual QFileInfoEngine *createFileInfoEngine(const QString &path) = 0;

private:
    friend QFileInfoEngine *qt_createFileInfoEngine(const QString &);
};

class QFSFileInfoEnginePrivate;
class QFSFileInfoEngine : public QFileInfoEngine
{
private:
    Q_DECLARE_PRIVATE(QFSFileInfoEngine)
public:
    QFSFileInfoEngine(const QString &file);

    virtual uint fileFlags(uint type) const;

    virtual void setFileName(const QString &file);
    virtual QString fileName(FileName file) const;
    virtual bool isRelativePath() const;

    virtual uint ownerId(FileOwner) const;
    virtual QString owner(FileOwner) const;

    virtual uint size() const;

    virtual QDateTime fileTime(FileTime time) const;
};

#endif /* __QFILEINFOENGINE_H__ */
