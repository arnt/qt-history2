/****************************************************************************
** $Id: //depot/qt/main/tests/url/qurlinfo.cpp#11 $
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
    d->isDir = FALSE;
    d->isFile = TRUE;
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
