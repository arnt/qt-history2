/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QFileInfo class
**
** Created : 950628
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

#include "qglobal.h"
#include "qfileinfo.h"
#include "qfiledefs.h"
#include "qdatetime.h"
#include "qdir.h"

void QFileInfo::slashify( QString& )
{
    return;
}

extern bool qt_file_access( const QString& fn, int t );

bool QFileInfo::isFile() const
{
    return true;
}

/*!
  Returns TRUE if we are pointing to a directory or a symbolic link to
  a directory.
  \sa isFile(), isSymLink()
*/

bool QFileInfo::isDir() const
{
    return true;
}

/*!
  Returns TRUE if we are pointing to a symbolic link.
  \sa isFile(), isDir(), readLink()
*/

bool QFileInfo::isSymLink() const
{
    return true;
}


/*!
  Returns the name a symlink points to, or a null QString if the
  object does not refer to a symbolic link.

  This name may not represent an existing file; it is only a string.
  QFileInfo::exists() returns TRUE if the symlink points to an
  existing file.

  \sa exists(), isSymLink(), isDir(), isFile()
*/

QString QFileInfo::readLink() const
{
    QString r;
    return r;
}

static const uint nobodyID = (uint) -2;

/*!
  Returns the owner of the file.

  On systems where files do not have owners this function returns 0.

  Note that this function can be time-consuming under UNIX. (in the order
  of milliseconds on a 486 DX2/66 running Linux).

  \sa ownerId(), group(), groupId()
*/

QString QFileInfo::owner() const
{
    return QString::null;
}

static const uint nobodyID = (uint) -2;

/*!
  Returns the id of the owner of the file.

  On systems where files do not have owners this function returns ((uint) -2).

  \sa owner(), group(), groupId()
*/

uint QFileInfo::ownerId() const
{
    return nobodyID;
}

/*!
  Returns the group the file belongs to.

  On systems where files do not have groups this function always
  returns 0.

  Note that this function can be time-consuming under UNIX (in the order of
  milliseconds on a 486 DX2/66 running Linux).

  \sa groupId(), owner(), ownerId()
*/

QString QFileInfo::group() const
{
    return QString::null;
}

/*!
  Returns the id of the group the file belongs to.

  On systems where files do not have groups this function always
  returns ((uind) -2).

  \sa group(), owner(), ownerId()
*/

uint QFileInfo::groupId() const
{
    return nobodyID;
}


/*!
  \fn bool QFileInfo::permission( int permissionSpec ) const

  Tests for file permissions.  The \e permissionSpec argument can be several
  flags of type PermissionSpec or'ed together to check for permission
  combinations.

  On systems where files do not have permissions this function always
  returns TRUE.

  Example:
  \code
    QFileInfo fi( "/tmp/tonsils" );
    if ( fi.permission( QFileInfo::WriteUser | QFileInfo::ReadGroup ) )
	qWarning( "Tonsils can be changed by me, and the group can read them.");
    if ( fi.permission( QFileInfo::WriteGroup | QFileInfo::WriteOther ) )
	qWarning( "Danger! Tonsils can be changed by the group or others!" );
  \endcode

  \sa isReadable(), isWritable(), isExecutable()
*/

bool QFileInfo::permission( int permissionSpec ) const
{
    return TRUE;
}

/*!
  Returns the file size in bytes, or 0 if the file does not exist if the size
  cannot be fetched.
*/

uint QFileInfo::size() const
{
    return 0;
}


/*!
  Returns the date and time when the file was last modified.
  \sa lastRead()
*/

QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;
    return dt;
}

/*!
  Returns the date and time when the file was last read (accessed).

  On systems that do not support last read times, the modification time is
  returned.

  \sa lastModified()
*/

QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;
    return dt;
}


void QFileInfo::doStat() const
{
}
