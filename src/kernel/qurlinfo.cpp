/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurlinfo.cpp#8 $
**
** Implementation of QFileDialog class
**
** Created : 950429
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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


/*!
  \class QUrlInfo qurlinfo.h

  This class is just a container for storing information about a
  URL. That's why all informations have to be passed in the
  constructor.
*/

/*!
  #### todo
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
  #### todo
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
  #### todo
*/

QUrlInfo::QUrlInfo()
{
    d = new QUrlInfoPrivate;
    d->isDir = FALSE;
    d->isFile = TRUE;
    d->isReadable = TRUE;
}

/*!
  #### todo
*/

QUrlInfo::QUrlInfo( const QUrl &path, const QString &file )
{
    QString file_ = file;
    if ( file_.isEmpty() )
	file_ = ".";
    d = new QUrlInfoPrivate;
    QUrl u( path, file_ );
    QUrlInfo inf = path.info( file_ );
    *d = *inf.d;
}

/*!
  #### todo
*/

QUrlInfo::QUrlInfo( const QUrlInfo &ui )
{
    d = new QUrlInfoPrivate;
    *d = *ui.d;
}

/*!
  Sets the filename.
*/

void QUrlInfo::setName( const QString &name )
{
    d->name = name;
}

/*!
  #### todo
*/

void QUrlInfo::setDir( bool b )
{
    d->isDir = b;
}

/*!
  #### todo
*/

void QUrlInfo::setFile( bool b )
{
    d->isFile = b;
}

/*!
  #### todo
*/

void QUrlInfo::setSymLink( bool b )
{
    d->isSymLink = b;
}

/*!
  #### todo
*/

void QUrlInfo::setOwner( const QString &s )
{
    d->owner = s;
}

/*!
  #### todo
*/

void QUrlInfo::setGroup( const QString &s )
{
    d->group = s;
}

/*!
  #### todo
*/

void QUrlInfo::setSize( uint s )
{
    d->size = s;
}

/*!
  #### todo
*/

QUrlInfo::~QUrlInfo()
{
    delete d;
}

/*!
  #### todo
*/

QUrlInfo &QUrlInfo::operator=( const QUrlInfo &ui )
{
    *d = *ui.d;
    return *this;
}

/*!
  #### todo
*/

QString QUrlInfo::name() const
{
    return d->name;
}

/*!
  #### todo
*/

int QUrlInfo::permissions() const
{
    return d->permissions;
}

/*!
  #### todo
*/

QString QUrlInfo::owner() const
{
    return d->owner;
}

/*!
  #### todo
*/

QString QUrlInfo::group() const
{
    return d->group;
}

/*!
  #### todo
*/

uint QUrlInfo::size() const
{
    return d->size;
}

/*!
  #### todo
*/

QDateTime QUrlInfo::lastModified() const
{
    return d->lastModified;
}

/*!
  #### todo
*/

QDateTime QUrlInfo::lastRead() const
{
    return d->lastRead;
}

/*!
  #### todo
*/

bool QUrlInfo::isDir() const
{
    return d->isDir;
}

/*!
  #### todo
*/

bool QUrlInfo::isFile() const
{
    return d->isFile;
}

/*!
  #### todo
*/

bool QUrlInfo::isSymLink() const
{
    return d->isSymLink;
}

/*!
  #### todo
*/

bool QUrlInfo::isWritable() const
{
    return d->isWritable;
}

/*!
  #### todo
*/

bool QUrlInfo::isReadable() const
{
    return d->isReadable;
}

/*!
  #### todo
*/

bool QUrlInfo::isExecutable() const
{
    return d->isExecutable;
}

/*!
  #### todo
*/

bool QUrlInfo::greaterThan( const QUrlInfo &i1, const QUrlInfo &i2,
			    int sortBy )
{
    if ( i1.name() == ".." )
	return FALSE;
    if ( i2.name() == ".." )
	return TRUE;
    
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
  #### todo
*/

bool QUrlInfo::lessThan( const QUrlInfo &i1, const QUrlInfo &i2,
			 int sortBy )
{
    if ( i1.name() == ".." )
	return TRUE;
    if ( i2.name() == ".." )
	return FALSE;

    switch ( sortBy ) {
    case QDir::Name:
	return i1.name() < i2.name();
    case QDir::Time:
	return i1.lastModified() < i2.lastModified();
    case QDir::Size:
	return i1.size() < i2.size();
    default:
	return FALSE;
    }

    return FALSE;
}

/*!
  #### todo
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
