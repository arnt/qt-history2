/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QFileInfo class
**
** Created : 950628
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the tools
** module and therefore may only be used if the tools module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qglobal.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"

void QFileInfo::slashify( QString& )
{
    return;
}

void QFileInfo::makeAbs( QString& )
{
    return;
}

extern bool qt_file_access( const QString& fn, int t );

bool QFileInfo::isFile() const
{
    return true;
}

bool QFileInfo::isDir() const
{
    return true;
}

bool QFileInfo::isSymLink() const
{
    return true;
}


QString QFileInfo::readLink() const
{
    QString r;
    return r;
}

static const uint nobodyID = (uint) -2;

QString QFileInfo::owner() const
{
    return QString::null;
}

uint QFileInfo::ownerId() const
{
    return nobodyID;
}

QString QFileInfo::group() const
{
    return QString::null;
}

uint QFileInfo::groupId() const
{
    return nobodyID;
}


bool QFileInfo::permission( int permissionSpec ) const
{
    return TRUE;
}

uint QFileInfo::size() const
{
    return 0;
}


QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;
    return dt;
}

QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;
    return dt;
}


void QFileInfo::doStat() const
{
}

QString QFileInfo::dirPath( bool absPath ) const
{
    QString s;
    if ( absPath )
	s = absFilePath();
    else
	s = fn;
    int pos = s.findRev( '/' );
    if ( pos == -1 ) {
	return QString::fromLatin1(".");
    } else {
	if ( pos == 0 )
	    return QString::fromLatin1( "/" );
	return s.left( pos );
    }
}

QString QFileInfo::fileName() const
{
    int p = fn.findRev( '/' );
    if ( p == -1 ) {
	return fn;
    } else {
	return fn.mid(p+1);
    }
}

QString QFileInfo::absFilePath() const
{
    if ( QDir::isRelativePath(fn) ) {
	QString tmp = QDir::currentDirPath();
	tmp += '/';
	tmp += fn;
	makeAbs( tmp );
	return QDir::cleanDirPath( tmp );
    } else {
	QString tmp = fn;
	makeAbs( tmp );
	return QDir::cleanDirPath( tmp );
    }

}
