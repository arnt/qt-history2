/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <private/qresourceengine_p.h>
#include <qdatetime.h>
#include "qresource.h"
#include <qregexp.h>
#include <private/qfileengine_p.h>

#define d d_func()
#define q q_func()

inline static QResource *qt_find_resource(const QString &path)
{
    if (path.size() > 0 && path[0] == QLatin1Char(':'))
        return QResource::find(path.mid(1));
    return 0;

}

class QResourceFileEngineHandler : public QFileEngineHandler
{
public:
    QResourceFileEngineHandler() { }
    QFileEngine *createFileEngine(const QString &path)
    {
        if (path.size() > 0 && path[0] == QLatin1Char(':'))
            return new QResourceFileEngine(path);
        return 0;
    }
};

class QResourceFileEnginePrivate : public QFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QResourceFileEngine)
private:
    QString file;
    QIODevice::Offset offset;
    mutable QResource *resource;
protected:
    QResourceFileEnginePrivate() : offset(0), resource(0) { }
};

bool
QResourceFileEngine::mkdir(const QString &, QDir::Recursion) const
{
    return false;
}

bool
QResourceFileEngine::rmdir(const QString &, QDir::Recursion) const
{
    return false;
}

bool
QResourceFileEngine::setSize(QIODevice::Offset) 
{
    return false;
}

QStringList
QResourceFileEngine::entryList(int filterSpec, const QStringList &filters) const
{
    const bool doDirs     = (filterSpec & QDir::Dirs) != 0;
    const bool doFiles    = (filterSpec & QDir::Files) != 0;
    const bool doReadable = (filterSpec & QDir::Readable) != 0;

    QStringList ret;
    if((!doDirs && !doFiles) || ((filterSpec & QDir::RWEMask) && !doReadable))
        return ret;
    if(!d->resource)
        d->resource = qt_find_resource(d->file);
    if(!d->resource || !d->resource->isContainer())
        return ret; // cannot read the directory

    QList<QResource*> entries = d->resource->children();
    for(int i = 0; i < entries.size(); i++) {
        QString fn = entries[i]->name();
#ifndef QT_NO_REGEXP
        if(!(filterSpec & QDir::AllDirs && d->resource->isContainer())) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filters.begin(); sit != filters.end(); ++sit) {
                QRegExp rx(*sit, Qt::CaseSensitive, QRegExp::Wildcard);
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
QResourceFileEngine::caseSensitive() const
{
    return true;
}

QResourceFileEngine::QResourceFileEngine(const QString &file) : QFileEngine(*new QResourceFileEnginePrivate)
{
    d->file = file;
}

void
QResourceFileEngine::setFileName(const QString &file)
{
    if(file != d->file) {
        d->resource = 0;
        d->file = file;
    }
}

bool
QResourceFileEngine::open(int flags)
{
    if (d->file.isEmpty()) {
        qWarning("QFSFileEngine::open: No file name specified");
        return false;
    }
    if ((flags & QFile::WriteOnly) == QFile::WriteOnly || (flags & QFile::Async))
        return false;
    if(!(d->resource = qt_find_resource(d->file)))
       return false;
    return true;
}

bool
QResourceFileEngine::close()
{
    return true;
}

void
QResourceFileEngine::flush()
{

}

Q_LONG
QResourceFileEngine::readBlock(char *data, Q_LONG len)
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
QResourceFileEngine::writeBlock(const char *, Q_LONG)
{
    return -1;
}

int
QResourceFileEngine::ungetch(int)
{
    return -1;
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
    return false;
}

uchar *
QResourceFileEngine::map(Q_LONG /*len*/)
{
    return 0;
}

uint
QResourceFileEngine::fileFlags(uint type) const
{
    uint ret = 0;
    if(!d->resource && !(d->resource = qt_find_resource(d->file)))
        return ret;
    if(type & PermsMask)
        ret |= (ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm);
    if(type & TypesMask) {
        if(d->resource->isContainer())
            ret |= DirectoryType;
        else
            ret |= FileType;
    }
    if(type & FlagsMask) {
        ret |= ExistsFlag;
        if(d->resource && d->resource->parent())
            ret |= RootFlag;
    }
    return ret;
}

bool
QResourceFileEngine::chmod(uint perms) 
{
    return false;
}

QString
QResourceFileEngine::fileName(FileName file) const
{
    if(file == BaseName) {
	int slash = d->file.lastIndexOf(QLatin1Char('/'));
	if (slash == -1) {
	    int colon = d->file.lastIndexOf(QLatin1Char(':'));
	    if (colon != -1)
		return d->file.mid(colon + 1);
	    return d->file;
	}
	return d->file.mid(slash + 1);
    } else if(file == PathName || file == AbsolutePathName) {
        if (!d->file.size())
            return d->file;
	int slash = d->file.lastIndexOf(QLatin1Char('/'));
	if (slash == -1) {
	    if (d->file.at(1) == QLatin1Char(':'))
		return d->file.left(2);
	    return QLatin1String(".");
	} else {
	    if (!slash)
		return QLatin1String("/");
	    if (slash == 2 && d->file.at(1) == QLatin1Char(':'))
		slash++;
	    return d->file.left(slash);
	}
    }
    return d->file;

}

bool
QResourceFileEngine::isRelativePath() const
{
    return false;
}

uint
QResourceFileEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString
QResourceFileEngine::owner(FileOwner) const
{
    return QString::null;
}

QDateTime
QResourceFileEngine::fileTime(FileTime) const
{
    return QDateTime();
}

QIOEngine::Type
QResourceFileEngine::type() const
{
    return QIOEngine::Resource;
}

//Initialization and cleanup
static QResourceFileEngineHandler *resource_file_handler = 0;
void qCleanupResourceIO()
{
    delete resource_file_handler;
    resource_file_handler = 0;
}
void qInitResourceIO()
{
    if(!resource_file_handler) 
        resource_file_handler = new QResourceFileEngineHandler;
}
//yuck, but this will force the auto init in shared libraries
inline static int qt_force_resource_init() { qInitResourceIO(); return 1; }
static int qt_forced_resource_init = qt_force_resource_init(); 
