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
#include <qdatetime.h>
#include "qresource.h"
#include <qregexp.h>
#include <private/qfileengine_p.h>

#define d d_func()
#define q q_func()

inline static QResource *qt_find_resource(const QString &path)
{
    if(!path.startsWith(":/"))
        return 0;
    return QResource::find(path.mid(1));
}

class QResourceFileEngineHandler : public QFileEngineHandler
{
public:
    QResourceFileEngineHandler() { }
    QFileEngine *createFileEngine(const QString &path)
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
    QIODevice::Offset offset;
    mutable QResource *resource;
protected:
    QResourceFileEnginePrivate() : offset(0), resource(0) { }
};

/*!
  \reimp
*/
bool
QResourceFileEngine::mkdir(const QString &, QDir::Recursion) const
{
    return false;
}

/*!
  \reimp
*/
bool
QResourceFileEngine::rmdir(const QString &, QDir::Recursion) const
{
    return false;
}

/*!
  \reimp
*/
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

/*!
  \reimp
*/
bool
QResourceFileEngine::caseSensitive() const
{
    return true;
}

/*!
  \reimp
*/
bool
QResourceFileEngine::isRoot() const
{
    if(!d->resource)
        d->resource = qt_find_resource(d->file);
    return d->resource && d->resource->parent();
}

/*!
  \reimp
*/
QResourceFileEngine::QResourceFileEngine(const QString &file) : QFileEngine(*new QResourceFileEnginePrivate)
{
    d->file = file;
}

/*!
  \reimp
*/
void
QResourceFileEngine::setFileName(const QString &file)
{
    if(file != d->file) {
        d->resource = 0;
        d->file = file;
    }
}

/*!
  \reimp
*/
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

/*!
  \reimp
*/
bool
QResourceFileEngine::close()
{
    return true;
}

/*!
  \reimp
*/
void
QResourceFileEngine::flush()
{

}

/*!
  \reimp
*/
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

/*!
  \reimp
*/
Q_LONG
QResourceFileEngine::writeBlock(const char *, Q_LONG)
{
    return -1;
}

/*!
  \reimp
*/
int
QResourceFileEngine::ungetch(int)
{
    return -1;
}

/*!
  \reimp
*/
bool
QResourceFileEngine::remove()
{
    return false;
}

/*!
  \reimp
*/
bool
QResourceFileEngine::rename(const QString &)
{
    return false;
}

/*!
  \reimp
*/
QFile::Offset
QResourceFileEngine::size() const
{
    if(!d->resource)
	d->resource = QResource::find(d->file);
    return d->resource->size();
}

/*!
  \reimp
*/
QFile::Offset
QResourceFileEngine::at() const
{
    return d->offset;
}

/*!
  \reimp
*/
bool
QResourceFileEngine::atEnd() const
{
    return d->offset == d->resource->size();
}

/*!
  \reimp
*/
bool
QResourceFileEngine::seek(QFile::Offset pos)
{
    if(d->offset > d->resource->size())
        return false;
    d->offset = pos;
    return true;
}

/*!
  \reimp
*/
bool
QResourceFileEngine::isSequential() const
{
    return false;
}

/*!
  \reimp
*/
int
QResourceFileEngine::handle() const
{
    return (int)d->resource;
}

/*!
  \reimp
*/
uchar *
QResourceFileEngine::map(Q_LONG /*len*/)
{
    return 0;
}

/*!
  \reimp
*/
uint
QResourceFileEngine::fileFlags(uint type) const
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

/*!
  \reimp
*/
QString
QResourceFileEngine::fileName(FileName file) const
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
    } else if(file == PathName || file == AbsolutePathName) {
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

/*!
  \reimp
*/
bool
QResourceFileEngine::isRelativePath() const
{
    return false;
}

/*!
  \reimp
*/
uint
QResourceFileEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

/*!
  \reimp
*/
QString
QResourceFileEngine::owner(FileOwner) const
{
    return QString::null;
}

/*!
  \reimp
*/
QDateTime
QResourceFileEngine::fileTime(FileTime) const
{
    return QDateTime();
}

/*!
  \reimp
*/
QIOEngine::Type
QResourceFileEngine::type() const
{
    return QIOEngine::Resource;
}
