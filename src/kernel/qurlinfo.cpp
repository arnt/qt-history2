/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurlinfo.cpp#18 $
**
** Implementation of QUrlInfo class
**
** Created : 950429
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qurlinfo.h"
#include "qurloperator.h"
#include "qdir.h"

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


// NOT REVISED
/*!
  \class QUrlInfo qurlinfo.h

  This class is just a container for storing information about a
  URLs. That's why all informations have to be passed in the
  constructor.
*/

/*!
  Constructs an empty QUrlInfo object with default values.
*/

QUrlInfo::QUrlInfo()
{
    d = new QUrlInfoPrivate;
    d->isDir = FALSE;
    d->isFile = TRUE;
    d->isReadable = TRUE;
    d->size = 0;
    d->isWritable = TRUE;
}

/*!
  Constructs a QUrlInfo object with information about the file \a file
  in the \a path. This constructor tries to find the infos about
  \a file, which should be stored in the QUrlOperator \a path.
  If this is not the case, an empty QUrlInfo object is created.
*/

QUrlInfo::QUrlInfo( const QUrlOperator &path, const QString &file )
{
    QString file_ = file;
    if ( file_.isEmpty() )
	file_ = ".";
    d = new QUrlInfoPrivate;
    QUrlInfo inf = path.info( file_ );
    *d = *inf.d;
}

/*!
  Copy constructor.
*/

QUrlInfo::QUrlInfo( const QUrlInfo &ui )
{
    d = new QUrlInfoPrivate;
    *d = *ui.d;
}

/*!
  Constructs a QUrlInfo object by specifying all information of the URL.
*/

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


/*!
  Constructs a QUrlInfo object by specifying all information of the URL.
*/

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

/*!
  Sets the filename or the URL.
*/

void QUrlInfo::setName( const QString &name )
{
    d->name = name;
}

/*!
  Specifies if the URL is a directory.
*/

void QUrlInfo::setDir( bool b )
{
    d->isDir = b;
}

/*!
  Specifies if the URL is a file.
*/

void QUrlInfo::setFile( bool b )
{
    d->isFile = b;
}

/*!
  Specifies if the URL is a symbolic link.
*/

void QUrlInfo::setSymLink( bool b )
{
    d->isSymLink = b;
}

/*!
  Specifies if the URL is writeable.
*/

void QUrlInfo::setWritable( bool b )
{
    d->isWritable = b;
}

/*!
  Specifies if the URL is readable.
*/

void QUrlInfo::setReadable( bool b )
{
    d->isReadable = b;
}

/*!
  Sets the owner of the URL to \a s.
*/

void QUrlInfo::setOwner( const QString &s )
{
    d->owner = s;
}

/*!
  Sets the group if the URL to \a s.
*/

void QUrlInfo::setGroup( const QString &s )
{
    d->group = s;
}

/*!
  Sets the size of the URL to \a s.
*/

void QUrlInfo::setSize( uint s )
{
    d->size = s;
}

/*!
  Sets the permissions of the URL to \a p.
*/

void QUrlInfo::setPermissions( int p )
{
    d->permissions = p;
}

/*!
  Sets the last modification date of the URL to \a dt.
*/

void QUrlInfo::setLastModified( const QDateTime &dt )
{
    d->lastModified = dt;
}

/*!
  Destructor.
*/

QUrlInfo::~QUrlInfo()
{
    delete d;
}

/*!
  Assings the values of \a ui to this QUrlInfo object.
*/

QUrlInfo &QUrlInfo::operator=( const QUrlInfo &ui )
{
    *d = *ui.d;
    return *this;
}

/*!
  Returns the filename of the URL.
*/

QString QUrlInfo::name() const
{
    return d->name;
}

/*!
  Returns the permissions of the URL.
*/

int QUrlInfo::permissions() const
{
    return d->permissions;
}

/*!
  Returns the owner of the URL.
*/

QString QUrlInfo::owner() const
{
    return d->owner;
}

/*!
  Returns the group of the URL.
*/

QString QUrlInfo::group() const
{
    return d->group;
}

/*!
  Returns the size of the URL.
*/

uint QUrlInfo::size() const
{
    return d->size;
}

/*!
  Returns the last modification date of the URL.
*/

QDateTime QUrlInfo::lastModified() const
{
    return d->lastModified;
}

/*!
  Returns the date at which the URL was read the last time.
*/

QDateTime QUrlInfo::lastRead() const
{
    return d->lastRead;
}

/*!
  Returns TRUE, if the URL is a directory, else FALSE.
*/

bool QUrlInfo::isDir() const
{
    return d->isDir;
}

/*!
  Returns TRUE, if the URL is a file, else FALSE.
*/

bool QUrlInfo::isFile() const
{
    return d->isFile;
}

/*!
  Returns TRUE, if the URL is a symbolic link, else FALSE.
*/

bool QUrlInfo::isSymLink() const
{
    return d->isSymLink;
}

/*!
  Returns TRUE, if the URL is writable, else FALSE.
*/

bool QUrlInfo::isWritable() const
{
    return d->isWritable;
}

/*!
  Returns TRUE, if the URL is readable , else FALSE.
*/

bool QUrlInfo::isReadable() const
{
    return d->isReadable;
}

/*!
  Returns TRUE, if the URL is executable, else FALSE.
*/

bool QUrlInfo::isExecutable() const
{
    return d->isExecutable;
}

/*!
  Returns TRUE if \a u1 is greater than \a u2, else FALSE. The objects
  are compared by the value, which is specified by \a sortBy. This has
  to be one of QDir::Name, QDir::Time and QDir::Size.
*/

bool QUrlInfo::greaterThan( const QUrlInfo &i1, const QUrlInfo &i2,
			    int sortBy )
{
    switch ( sortBy ) {
    case QDir::Name:
	return i1.name() > i2.name();
    case QDir::Time:
	return i1.lastModified() > i2.lastModified();
    case QDir::Size:
	return i1.size() > i2.size();
    default:
	return FALSE;
    }

    return FALSE;
}

/*!
  Returns TRUE if \a u1 is less than \a u2, else FALSE. The objects
  are compared by the value, which is specified by \a sortBy. This has
  to be one of QDir::Name, QDir::Time and QDir::Size.
*/

bool QUrlInfo::lessThan( const QUrlInfo &i1, const QUrlInfo &i2,
			 int sortBy )
{
    return !greaterThan( i1, i2, sortBy );
}

/*!
  Returns TRUE if \a u1 is equal \a u2, else FALSE. The objects
  are compared by the value, which is specified by \a sortBy. This has
  to be one of QDir::Name, QDir::Time and QDir::Size.
*/

bool QUrlInfo::equal( const QUrlInfo &i1, const QUrlInfo &i2,
		      int sortBy )
{
    switch ( sortBy ) {
    case QDir::Name:
	return i1.name() == i2.name();
    case QDir::Time:
	return i1.lastModified() == i2.lastModified();
    case QDir::Size:
	return i1.size() == i2.size();
    default:
	return FALSE;
    }

    return FALSE;
}
