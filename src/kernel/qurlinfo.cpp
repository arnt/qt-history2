/****************************************************************************
** $Id: $
**
** Implementation of QUrlInfo class
**
** Created : 950429
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qurlinfo.h"

#ifndef QT_NO_NETWORKPROTOCOL

#include "qurloperator.h"
#include "qdir.h"

class QUrlInfoPrivate
{
public:
    QUrlInfoPrivate() :
	permissions(0),
	size(0),
	isDir(FALSE),
	isFile(TRUE),
	isSymLink(FALSE),
	isWritable(TRUE),
	isReadable(TRUE),
	isExecutable(FALSE)
    {}

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
  \brief The QUrlInfo class stores information about URLs.

  \ingroup io
  \ingroup misc

  This class is just a container for storing information about
  URLs, which is why all information must be passed in the
  constructor.

  Unless you're reimplementing a network protocol you're unlikely to
  create QUrlInfo objects yourself, but you may get QUrlInfo objects
  from functions, e.g. QUrlOperator::info().
*/

/*!
  Constructs an invalid QUrlInfo object with default values.

  \sa isValid()
*/

QUrlInfo::QUrlInfo()
{
    d = 0;
}

/*!
  Constructs a QUrlInfo object with information about the file \a file
  in the \a path. It tries to find the information about the \a file in the
  QUrlOperator \a path.

  If the information is not found, this constructor creates an invalid
  QUrlInfo, i.e. isValid() returns FALSE. You should always check if the URL
  info is valid before relying on the return values of any getter functions.

  If \a file is empty, it defaults to the actual directory of the QUrlOperator
  \a path.

  \sa isValid() QUrlOperator::info()
*/

QUrlInfo::QUrlInfo( const QUrlOperator &path, const QString &file )
{
    QString file_ = file;
    if ( file_.isEmpty() )
	file_ = ".";

    QUrlInfo inf = path.info( file_ );
    if ( inf.d ) {
	d = new QUrlInfoPrivate;
	*d = *inf.d;
    } else {
	d = 0;
    }
}

/*!
  Copy constructor, copies \a ui to this URL info object.
*/

QUrlInfo::QUrlInfo( const QUrlInfo &ui )
{
    if ( ui.d ) {
	d = new QUrlInfoPrivate;
	*d = *ui.d;
    } else {
	d = 0;
    }
}

/*!
  Constructs a QUrlInfo object by specifying all the URL's information.
  The information that is passed is the \a name, the \a permissions,
  the \a owner and \a group, as well as the \a size, \a lastModified
  date/time and \a lastRead date/time. Flags are also passed,
  specifically, \a isDir, \a isFile, \a isSymLink, \a isWritable, \a
  isReadable and \a isExecutable.
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
  Constructs a QUrlInfo object by specifying all the URL's information.
  The information that is passed is the \a url, the \a permissions,
  the \a owner and \a group, as well as the \a size, \a lastModified
  date/time and \a lastRead date/time. Flags are also passed,
  specifically, \a isDir, \a isFile, \a isSymLink, \a isWritable, \a
  isReadable and \a isExecutable.
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
  Sets the name of the URL to \a name. The name is the full text, for
  example, "http://doc.trolltech.com/qurlinfo.html".

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setName( const QString &name )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->name = name;
}


/*!
  If \a b is TRUE then the URL is set to be a directory; if \b is FALSE then
  the URL is set not to be a directory (which normally means it is a file).
  (Note that a URL can refer both a file and a directory even though most file
  systems do not support this duality.)

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setDir( bool b )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->isDir = b;
}


/*!
  If \a b is TRUE then the URL is set to be a file; if \b is FALSE then the URL
  is set not to be a file (which normally means it is a directory). (Note that
  a URL can refer both a file and a directory even though most file systems do
  not support this duality.)

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setFile( bool b )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->isFile = b;
}


/*!
  Specifies that the URL refers to a symbolic link if \a b is TRUE and that it
  does not if \a b is FALSE.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setSymLink( bool b )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->isSymLink = b;
}


/*!
  Specifies that the URL is writable if \a b is TRUE and not writable if \a b
  is FALSE.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setWritable( bool b )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->isWritable = b;
}


/*!
  Specifies that the URL is readable if \a b is TRUE and not readable if \a b
  is FALSE.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setReadable( bool b )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->isReadable = b;
}

/*!
  Specifies that the owner of the URL is called \a s.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setOwner( const QString &s )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->owner = s;
}

/*!
  Specifies that the owning group of the URL is called \a s.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setGroup( const QString &s )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->group = s;
}

/*!
  Specifies that the URL has size \a s.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setSize( uint s )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->size = s;
}


// ### reggie - what's the permission type? As in Unix?

/*!
  Specifies that the URL has access permision \a p.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setPermissions( int p )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->permissions = p;
}

/*!
  Specifies that the object the URL refers to was last modified at \a dt.

  If you call this function for an invalid URL info, this function turns it
  into a valid one.

  \sa isValid()
*/

void QUrlInfo::setLastModified( const QDateTime &dt )
{
    if ( !d )
	d = new QUrlInfoPrivate;
    d->lastModified = dt;
}

/*!
  Destroys the URL ifno object. The QUrlOperator object to which this URL
  referred is not affected.
*/

QUrlInfo::~QUrlInfo()
{
    delete d;
}

/*!
  Assigns the values of \a ui to this QUrlInfo object.
*/

QUrlInfo &QUrlInfo::operator=( const QUrlInfo &ui )
{
    if ( ui.d ) {
	if ( !d )
	    d= new QUrlInfoPrivate;
	*d = *ui.d;
    } else {
	delete d;
	d = 0;
    }
    return *this;
}

/*!
  Returns the file name of the URL.

  \sa isValid()
*/

QString QUrlInfo::name() const
{
    if ( !d )
	return QString::null;
    return d->name;
}

/*!
  Returns the permissions of the URL.

  \sa isValid()
*/

int QUrlInfo::permissions() const
{
    if ( !d )
	return 0;
    return d->permissions;
}

/*!
  Returns the owner of the URL.

  \sa isValid()
*/

QString QUrlInfo::owner() const
{
    if ( !d )
	return QString::null;
    return d->owner;
}

/*!
  Returns the group of the URL.

  \sa isValid()
*/

QString QUrlInfo::group() const
{
    if ( !d )
	return QString::null;
    return d->group;
}

/*!
  Returns the size of the URL.

  \sa isValid()
*/

uint QUrlInfo::size() const
{
    if ( !d )
	return 0;
    return d->size;
}

/*!
  Returns the last modification date of the URL.

  \sa isValid()
*/

QDateTime QUrlInfo::lastModified() const
{
    if ( !d )
	return QDateTime();
    return d->lastModified;
}

/*!
  Returns the date when the URL was read the last time.

  \sa isValid()
*/

QDateTime QUrlInfo::lastRead() const
{
    if ( !d )
	return QDateTime();
    return d->lastRead;
}

/*!
  Returns TRUE if the URL is a directory; otherwise returns FALSE.

  \sa isValid()
*/

bool QUrlInfo::isDir() const
{
    if ( !d )
	return FALSE;
    return d->isDir;
}

/*!
  Returns TRUE if the URL is a file; otherwise returns FALSE.

  \sa isValid()
*/

bool QUrlInfo::isFile() const
{
    if ( !d )
	return FALSE;
    return d->isFile;
}

/*!
  Returns TRUE if the URL is a symbolic link; otherwise returns FALSE.

  \sa isValid()
*/

bool QUrlInfo::isSymLink() const
{
    if ( !d )
	return FALSE;
    return d->isSymLink;
}

/*!
  Returns TRUE if the URL is writable; otherwise returns FALSE.

  \sa isValid()
*/

bool QUrlInfo::isWritable() const
{
    if ( !d )
	return FALSE;
    return d->isWritable;
}

/*!
  Returns TRUE if the URL is readable; otherwise returns FALSE.

  \sa isValid()
*/

bool QUrlInfo::isReadable() const
{
    if ( !d )
	return FALSE;
    return d->isReadable;
}

/*!
  Returns TRUE if the URL is executable; otherwise returns FALSE.

  \sa isValid()
*/

bool QUrlInfo::isExecutable() const
{
    if ( !d )
	return FALSE;
    return d->isExecutable;
}

/*!
  Returns TRUE if \a i1 is greater than \a i2; otherwise returns
  FALSE. The objects are compared by the value, which is specified by
  \a sortBy. This must be one of QDir::Name, QDir::Time or
  QDir::Size.
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
}

/*!
  Returns TRUE if \a i1 is less than \a i2; otherwise returns FALSE.
  The objects are compared by the value, which is specified by \a
  sortBy. This must be one of QDir::Name, QDir::Time or QDir::Size.
*/

bool QUrlInfo::lessThan( const QUrlInfo &i1, const QUrlInfo &i2,
			 int sortBy )
{
    return !greaterThan( i1, i2, sortBy );
}

/*!
  Returns TRUE if \a i1 equals to \a i2; otherwise returns FALSE. The
  objects are compared by the value, which is specified by \a sortBy.
  This must be one of QDir::Name, QDir::Time or QDir::Size.
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
}

/*!
  Compares this QUrlInfo with \a i and returns TRUE if they
  are equal; otherwise returns FALSE.
*/

bool QUrlInfo::operator==( const QUrlInfo &i ) const
{
    if ( !d )
	return i.d == 0;
    if ( !i.d )
	return FALSE;

    return ( d->name == i.d->name &&
	     d->permissions == i.d->permissions &&
	     d->owner == i.d->owner &&
	     d->group == i.d->group &&
	     d->size == i.d->size &&
	     d->lastModified == i.d->lastModified &&
	     d->lastRead == i.d->lastRead &&
	     d->isDir == i.d->isDir &&
	     d->isFile == i.d->isFile &&
	     d->isSymLink == i.d->isSymLink &&
	     d->isWritable == i.d->isWritable &&
	     d->isReadable == i.d->isReadable &&
	     d->isExecutable == i.d->isExecutable );
}

/*!
  Returns TRUE if the URL info is valid; otherwise returns FALSE. Valid means
  that the QUrlInfo contains real information. E.g., a call to
  QUrlOperator::info() might return a an invalid QUrlInfo, if no information
  about the requested entry is available.

  You should always check if the URL info is valid before relying on the
  values.
*/
bool QUrlInfo::isValid() const
{
    return d != 0;
}

#endif // QT_NO_NETWORKPROTOCOL
