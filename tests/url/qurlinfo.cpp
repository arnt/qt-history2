#include "qurlinfo.h"

struct QUrlInfoPrivate
{
    QString name;
    int permissions;
    QString owner;
    QString group;
    uint size;
    QDateTime lastModified;
    QDateTime lastRead;
    bool isDir;
    bool isFile;
    bool isSymLink;
    bool isWritable;
    bool isReadable;
    bool isExecutable;
};   


QUrlInfo::QUrlInfo( const QUrl &url, int permissions, const QString &owner,
		    const QString &group, uint size, const QDateTime &lastModified,
		    const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
		    bool isWritable, bool isReadable, bool isExecutable )
{
    d = new QUrlInfoPrivate;
    d->name = url.path(); //### todo get name!
    d->permissions = permissions;
    d->owner = owner;
    d->group = group;
    d->size = size;
    d->lastModified = lastModified;
    d->lastRead = lastRead;
    d->isDir = isDir;
    d->isFile = isFile;
    d->isSymLink = isSymLink;
    d->isWritable = isWritable;
    d->isReadable = isReadable;
    d->isExecutable = isExecutable;
}

QUrlInfo::QUrlInfo( const QUrl &path, const QString &file )
{
    // ### todo
}
    
QUrlInfo::QUrlInfo( const QUrlInfo &ui )
{
    d = new QUrlInfoPrivate;
    *d = *ui.d;
}

QUrlInfo::~QUrlInfo()
{
    delete d;
}

QUrlInfo &QUrlInfo::operator=( const QUrlInfo &ui )
{
    *d = *ui.d;
    return *this;
}

QString QUrlInfo::name() const
{
    return d->name;
}

int QUrlInfo::permissions() const
{
    return d->permissions;
}

QString QUrlInfo::owner() const
{
    return d->owner;
}

QString QUrlInfo::group() const
{
    return d->group;
}

uint QUrlInfo::size() const
{
    return d->size;
}

QDateTime QUrlInfo::lastModified() const
{
    return d->lastModified;
}

QDateTime QUrlInfo::lastRead() const
{
    return d->lastRead;
}

bool QUrlInfo::isDir() const
{
    return d->isDir;
}

bool QUrlInfo::isFile() const
{
    return d->isFile;
}

bool QUrlInfo::isSymLink() const
{
    return d->isSymLink;
}  

bool QUrlInfo::isWritable() const
{
    return d->isWritable;
}

bool QUrlInfo::isReadable() const
{
    return d->isReadable;
}

bool QUrlInfo::isExecutable() const
{
    return d->isExecutable;
}
