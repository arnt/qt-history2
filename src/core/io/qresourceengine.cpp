/****************************************************************************
**
** Implementation  of QResourceEngine classes.
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
#include <private/qresourceengine_p.h>
#include "qresource.h"
#include <qregexp.h>
#include <private/qdirengine_p.h>
#include <private/qfileengine_p.h>
#include <private/qfileinfoengine_p.h>

#define d d_func()
#define q q_func()

inline static QResource *qt_find_resource(const QString &path)
{
    if(!path.startsWith(":/"))
        return 0;
    return QResource::find(path.mid(1));
}


/* ***************** ResourceDirEngine ************** */
class QResourceDirEngineHandler : public QDirEngineHandler
{
public:
    QResourceDirEngineHandler() { }
    QResourceDirEngine *createDirEngine(const QString &path) 
    { 
        if(path.startsWith(":/")) 
            return new QResourceDirEngine(path); 
        return 0;
    }
};
static QResourceDirEngineHandler resource_dir_handler;
class QResourceDirEnginePrivate : public QDirEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QResourceDirEngine)
private:    
    QString path;
    mutable QResource *resource;
protected:
    QResourceDirEnginePrivate() : resource(0) { }
};

QResourceDirEngine::QResourceDirEngine(const QString &path) : QDirEngine(*new QResourceDirEnginePrivate)
{
    d->path = path;
}

void
QResourceDirEngine::setPath(const QString &path)
{
    if(path != d->path) {
        if(d->resource)
            d->resource = 0;
        d->path = path;
    }
}

bool
QResourceDirEngine::mkdir(const QString &, QDir::Recursivity) const
{
    return false;
}

bool
QResourceDirEngine::rmdir(const QString &, QDir::Recursivity) const
{
    return false;
}

bool
QResourceDirEngine::rename(const QString &, const QString &) const
{
    return false;
}

QStringList
QResourceDirEngine::entryList(int filterSpec, const QStringList &filters) const
{
    const bool doDirs     = (filterSpec & QDir::Dirs) != 0;
    const bool doFiles    = (filterSpec & QDir::Files) != 0;
    const bool doReadable = (filterSpec & QDir::Readable) != 0;

    QStringList ret;
    if((!doDirs && !doFiles) || ((filterSpec & QDir::RWEMask) && !doReadable))
        return ret;
    if(!d->resource)
        d->resource = qt_find_resource(d->path);
    if(!d->resource || !d->resource->isContainer())
        return ret; // cannot read the directory

    QList<QResource*> entries = d->resource->children();
    for(int i = 0; i < entries.size(); i++) {
        QString fn = entries[i]->name();
#ifndef QT_NO_REGEXP
        if(!(filterSpec & QDir::AllDirs && d->resource->isContainer())) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filters.begin(); sit != filters.end(); ++sit) {
                QRegExp rx(*sit, QString::CaseSensitive, QRegExp::Wildcard);
                if (rx.exactMatch(fn)) {
                    matched = true;
                }
            }
            if(!matched)
                continue;
        }
#endif
        if  ((doDirs && d->resource->isContainer()) || 
             (doFiles && !d->resource->isContainer()))
            ret.append(fn);
    }
    return ret;
}

bool
QResourceDirEngine::caseSensitive() const
{
    return true;
}

bool
QResourceDirEngine::isRoot() const
{
    if(!d->resource)
        d->resource = qt_find_resource(d->path);
    return d->resource && d->resource->parent();
}

/* ***************** ResourceFileEngine ************** */
class QResourceFileEngineHandler : public QFileEngineHandler
{
public:
    QResourceFileEngineHandler() { }
    QResourceFileEngine *createFileEngine(const QString &path) 
    { 
        if(path.startsWith(":/"))
            return new QResourceFileEngine(path);
        return 0;
    }
};
static QResourceFileEngineHandler resource_file_handler;
class QResourceFileEnginePrivate : public QFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QResourceFileEngine)
private:   
    QString file;
    QFile::Offset offset;
    mutable QResource *resource;
protected:
    QResourceFileEnginePrivate() : offset(0), resource(0) { }
};

QResourceFileEngine::QResourceFileEngine(const QString &file) : QFileEngine(*new QResourceFileEnginePrivate)
{
    d->file = file;
}

bool
QResourceFileEngine::isOpen() const
{
    return d->resource;
}

void
QResourceFileEngine::setFileName(const QString &file)
{
    d->file = file;
}

bool
QResourceFileEngine::open(int flags)
{
    if ((flags & QFile::WriteOnly) == QFile::WriteOnly || (flags & QFile::Async))
        return false;
    if(!(d->resource = qt_find_resource(d->file))) 
       return false;
    return true;
}

bool
QResourceFileEngine::close()
{
    d->resource = 0;
    return true;
}

void
QResourceFileEngine::flush()
{
    
}

Q_LONG
QResourceFileEngine::readBlock(uchar *data, Q_LONG len)
{
    if(len > d->resource->size()-d->offset) {
        len = d->resource->size()-d->offset;
        if(!len)
            return 0;
    }
    memcpy(data, d->resource->data()+d->offset, len);
    d->offset += len;
    return len;
}

Q_LONG
QResourceFileEngine::writeBlock(const uchar *, Q_LONG)
{
    return 0;
}

bool
QResourceFileEngine::remove()
{
    return false;
}

bool
QResourceFileEngine::rename(const QString &)
{
    return false;
}

QFile::Offset
QResourceFileEngine::size() const
{
    if(!d->resource) 
	d->resource = QResource::find(d->file);
    return d->resource->size();
}

QFile::Offset
QResourceFileEngine::at() const
{
    return d->offset;
}

bool
QResourceFileEngine::atEnd() const
{
    return d->offset == d->resource->size();
}

bool
QResourceFileEngine::seek(QFile::Offset pos)
{
    if(d->offset > d->resource->size())
        return false;
    d->offset = pos;
    return true;
}

bool
QResourceFileEngine::isSequential() const
{
    return true;
}

int
QResourceFileEngine::handle() const
{
    return (int)d->resource;
}

uchar *
QResourceFileEngine::map(Q_LONG /*len*/)
{
    return 0;
}


/* ***************** ResourceFileInfoEngine ************** */
class QResourceFileInfoEngineHandler : public QFileInfoEngineHandler
{
public:
    QResourceFileInfoEngineHandler() { }
    QResourceFileInfoEngine *createFileInfoEngine(const QString &path) 
    { 
        if(path.startsWith(":/")) 
            return new QResourceFileInfoEngine(path); 
        return 0;
    }
};
static QResourceFileInfoEngineHandler resource_file_info_handler;
class QResourceFileInfoEnginePrivate : public QFileInfoEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QResourceFileInfoEngine)
private:   
    QString file;
    mutable QResource *resource;
protected:
    QResourceFileInfoEnginePrivate() : resource(0) { }
};

QResourceFileInfoEngine::QResourceFileInfoEngine(const QString &file) : QFileInfoEngine(*new QResourceFileInfoEnginePrivate)
{
    d->file = file;
}


uint
QResourceFileInfoEngine::fileFlags(uint type) const
{
    uint ret = 0;
    if(!d->resource && !(d->resource = qt_find_resource(d->file)))
        return ret;
    if(type & PermsMask) 
        ret |= (ReadOwner|ReadUser|ReadGroup|ReadOther);
    if(type & TypeMask) {
        if(d->resource->isContainer())
            ret |= Directory;
        else
            ret |= File;
    }
    if(type & FlagsMask) 
        ret |= Exists;
    return ret;
}

void
QResourceFileInfoEngine::setFileName(const QString &file)
{
    d->file = file;
    d->resource = 0;
}


QString
QResourceFileInfoEngine::fileName(FileName file) const
{
    if(file == BaseName) {
	int slash = d->file.lastIndexOf('/');
	if (slash == -1) {
	    int colon = d->file.lastIndexOf(':');
	    if (colon != -1)
		return d->file.mid(colon + 1);
	    return d->file;
	}
	return d->file.mid(slash + 1);
    } else if(file == DirPath || file == AbsoluteDirPath) {
        if (!d->file.size())
            return d->file;
	int slash = d->file.lastIndexOf('/');
	if (slash == -1) {
	    if (d->file.at(1) == ':') 
		return d->file.left(2);
	    return QString::fromLatin1(".");
	} else {
	    if (!slash)
		return QString::fromLatin1("/");
	    if (slash == 2 && d->file.at(1) == ':')
		slash++;
	    return d->file.left(slash);
	}
    }
    return d->file;
    
}


bool
QResourceFileInfoEngine::isRelativePath() const
{
    return false;
}


uint
QResourceFileInfoEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}


QString
QResourceFileInfoEngine::owner(FileOwner) const
{
    return QString::null;
}


QIODevice::Offset
QResourceFileInfoEngine::size() const
{
    return d->resource->size();
}


QDateTime
QResourceFileInfoEngine::fileTime(FileTime) const
{
    return QDateTime();
}
