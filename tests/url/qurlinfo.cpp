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


QUrlInfo::QUrlInfo( const QString &name, int permissions, const QString &owner,
		    const QString &group, uint size, const QDateTime &lastModified,
		    const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
		    bool isWritable, bool isReadable, bool isExecutable )
{
    d = new QUrlInfoPrivate;
    d->name = name;
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

QUrlInfo::QUrlInfo( const QUrl &url, int permissions, const QString &owner,
		    const QString &group, uint size, const QDateTime &lastModified,
		    const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
		    bool isWritable, bool isReadable, bool isExecutable )
{
    d = new QUrlInfoPrivate;
    d->name = QFileInfo( url.path() ).fileName();
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

QUrlInfo::QUrlInfo()
{
    d = new QUrlInfoPrivate;
}

QUrlInfo::QUrlInfo( const QUrl &path, const QString &file )
{
    d = new QUrlInfoPrivate;
    QUrl u( path, file );
    QUrlInfo inf = path.info( file );
    *d = *inf.d;
}

QUrlInfo::QUrlInfo( const QUrlInfo &ui )
{
    d = new QUrlInfoPrivate;
    *d = *ui.d;
}

void QUrlInfo::setName( const QString &name )
{
    d->name = name;
}

void QUrlInfo::setDir( bool b )
{
    d->isDir = b;
}

void QUrlInfo::setFile( bool b )
{
    d->isFile = b;
}

void QUrlInfo::setOwner( const QString &s )
{
    d->owner = s;
}

void QUrlInfo::setGroup( const QString &s )
{
    d->group = s;
}

void QUrlInfo::setSize( uint s )
{
    d->size = s;
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

QString QUrlInfo::makeUrl( const QUrl &path, bool withProtocolWhenLocal ) const
{
    QString url = QString::null;
    if ( path.isLocalFile() ) {
	if ( withProtocolWhenLocal ) {
	    url = path.protocol();
	    url += "://";
	}
	url = path.path();
	url += "/" + d->name;
    } else if ( path.protocol() == "ftp" ) {
	url = path + /*.protocol() + "://" + path.host() + path.path() + "/" +*/ d->name;
    }

    return url;
}
