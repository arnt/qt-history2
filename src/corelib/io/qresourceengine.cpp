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

#if defined(Q_OS_SOLARIS)
static const QAtomic qt_static_workaround; // ### remove me when changing the resource engine
#endif

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
    ~QResourceFileEngineHandler();
    QFileEngine *createFileEngine(const QString &path);
};

QResourceFileEngineHandler::~QResourceFileEngineHandler()
{
}

QFileEngine *QResourceFileEngineHandler::createFileEngine(const QString &path)
{
    if (path.size() > 0 && path[0] == QLatin1Char(':'))
        return new QResourceFileEngine(path);
    return 0;
}

class QResourceFileEnginePrivate : public QFileEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QResourceFileEngine)
private:
    QString file;
    qint64 offset;
    mutable QResource *resource;
protected:
    QResourceFileEnginePrivate() : offset(0), resource(0) { }
};

bool QResourceFileEngine::mkdir(const QString &, bool) const
{
    return false;
}

bool QResourceFileEngine::rmdir(const QString &, bool) const
{
    return false;
}

bool QResourceFileEngine::setSize(qint64)
{
    return false;
}

QStringList QResourceFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    Q_D(const QResourceFileEngine);

    const bool doDirs     = (filters & QDir::Dirs) != 0;
    const bool doFiles    = (filters & QDir::Files) != 0;
    const bool doReadable = (filters & QDir::Readable) != 0;

    QStringList ret;
    if((!doDirs && !doFiles) || ((filters & QDir::PermissionMask) && !doReadable))
        return ret;
    if(!d->resource)
        d->resource = qt_find_resource(d->file);
    if(!d->resource || !d->resource->isContainer())
        return ret; // cannot read the directory

    QList<QResource*> entries = d->resource->children();
    for(int i = 0; i < entries.size(); i++) {
        QString fn = entries[i]->name();
#ifndef QT_NO_REGEXP
        if(!(filters & QDir::AllDirs && entries.at(i)->isContainer())) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filterNames.begin(); sit != filterNames.end(); ++sit) {
                QRegExp rx(*sit,
                           (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive,
                           QRegExp::Wildcard);
                if (rx.exactMatch(fn)) {
                    matched = true;
                    break;
                }
            }
            if(!matched)
                continue;
        }
#endif
        if  ((doDirs && entries.at(i)->isContainer()) ||
             (doFiles && !entries.at(i)->isContainer()))
            ret.append(fn);
    }
    return ret;
}

bool QResourceFileEngine::caseSensitive() const
{
    return true;
}

QResourceFileEngine::QResourceFileEngine(const QString &file) : QFileEngine(*new QResourceFileEnginePrivate)
{
    Q_D(QResourceFileEngine);
    d->file = file;
}

QResourceFileEngine::~QResourceFileEngine()
{
}

void QResourceFileEngine::setFileName(const QString &file)
{
    Q_D(QResourceFileEngine);
    if(file != d->file) {
        d->resource = 0;
        d->file = file;
    }
}

bool QResourceFileEngine::open(int flags)
{
    Q_D(QResourceFileEngine);
    if (d->file.isEmpty()) {
        qWarning("QFSFileEngine::open: No file name specified");
        return false;
    }
    if (flags & QIODevice::WriteOnly)
        return false;
    if(!(d->resource = qt_find_resource(d->file)))
       return false;
    return true;
}

bool QResourceFileEngine::close()
{
    return true;
}

void QResourceFileEngine::flush()
{

}

qint64 QResourceFileEngine::read(char *data, qint64 len)
{
    Q_D(QResourceFileEngine);
    if(len > d->resource->size()-d->offset) {
        len = d->resource->size()-d->offset;
        if(!len)
            return 0;
    }
    memcpy(data, d->resource->data()+d->offset, len);
    d->offset += len;
    return len;
}

qint64 QResourceFileEngine::write(const char *, qint64)
{
    return -1;
}

int QResourceFileEngine::ungetch(int)
{
    return -1;
}

bool QResourceFileEngine::remove()
{
    return false;
}

bool QResourceFileEngine::copy(const QString &)
{
    return false;
}

bool QResourceFileEngine::rename(const QString &)
{
    return false;
}

bool QResourceFileEngine::link(const QString &)
{
    return false;
}

qint64 QResourceFileEngine::size() const
{
    Q_D(const QResourceFileEngine);

    if(!d->resource && !(d->resource = QResource::find(d->file)))
        return 0;

    return d->resource->size();
}

qint64 QResourceFileEngine::at() const
{
    Q_D(const QResourceFileEngine);
    if(!d->resource)
        return 0;

    return d->offset;
}

bool QResourceFileEngine::atEnd() const
{
    Q_D(const QResourceFileEngine);
    if(!d->resource)
        return true;

    return d->offset == d->resource->size();
}

bool QResourceFileEngine::seek(qint64 pos)
{
    Q_D(QResourceFileEngine);
    if(!d->resource)
        return false;

    if(d->offset > d->resource->size())
        return false;
    d->offset = pos;
    return true;
}

bool QResourceFileEngine::isSequential() const
{
    return false;
}

QFileEngine::FileFlags QResourceFileEngine::fileFlags(QFileEngine::FileFlags type) const
{
    Q_D(const QResourceFileEngine);
    QFileEngine::FileFlags ret = 0;
    if(!d->resource && !(d->resource = qt_find_resource(d->file)))
        return ret;
    if(type & PermsMask)
        ret |= QFileEngine::FileFlags(ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm);
    if(type & TypesMask) {
        if(d->resource->isContainer())
            ret |= DirectoryType;
        else
            ret |= FileType;
    }
    if(type & FlagsMask) {
        ret |= ExistsFlag;
        if(d->resource && !d->resource->parent())
            ret |= RootFlag;
    }
    return ret;
}

bool QResourceFileEngine::chmod(uint)
{
    return false;
}

QString QResourceFileEngine::fileName(FileName file) const
{
    Q_D(const QResourceFileEngine);
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

bool QResourceFileEngine::isRelativePath() const
{
    return false;
}

uint QResourceFileEngine::ownerId(FileOwner) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString QResourceFileEngine::owner(FileOwner) const
{
    return QString();
}

QDateTime QResourceFileEngine::fileTime(FileTime) const
{
    return QDateTime();
}

QFileEngine::Type QResourceFileEngine::type() const
{
    return QFileEngine::Resource;
}

//Initialization and cleanup
Q_GLOBAL_STATIC(QResourceFileEngineHandler, resource_file_handler)
//yuck, but this will force the auto init in shared libraries
inline static int qt_force_resource_init() { resource_file_handler(); return 1; }
void qInitResourceIO() { resource_file_handler(); }
static int qt_forced_resource_init = qt_force_resource_init();
