/****************************************************************************
** $Id: $
**
** Implementation of QFileInfo class
**
** Created : 950628
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qglobal.h"

#ifdef Q_OS_MACX
#include <pwd.h>
#include <grp.h>
#endif

#include "qfileinfo.h"
#include "qplatformdefs.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"
#include <qt_mac.h>

void QFileInfo::slashify( QString& n )
{
    for ( int i = 0; i < (int)n.length(); i++ ) {
	if( n[i] == ':' )
	    n[i] = '/';
    }
}

void QFileInfo::makeAbs( QString & )
{
    return;
}

bool QFileInfo::isSymLink() const
{
#if defined(Q_OS_UNIX)
    if ( !fic || !cache )
	doStat();
    return symLink;
#else
    return FALSE;
#endif
}

QString QFileInfo::readLink() const
{
#if defined(Q_OS_MACX)
    char s[PATH_MAX+1];
    if ( !isSymLink() )
	return QString();
    int len = readlink( QFile::encodeName(QDir::convertSeparators(fn)).data(), s, PATH_MAX );
    if ( len >= 0 ) {
	s[len] = '\0';
	return QFile::decodeName(s);
    }
#endif
    return QString();
}

static const uint nobodyID = (uint) -2;

QString QFileInfo::owner() const
{
#ifdef Q_OS_MACX
    passwd *pw = getpwuid( ownerId() );
    if ( pw )
	return QFile::decodeName( pw->pw_name );
#endif
    return QString::null;
}

uint QFileInfo::ownerId() const
{
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return fic->st.st_uid;
    return nobodyID;
}

QString QFileInfo::group() const
{
#ifdef Q_OS_MACX
    struct group *gr = getgrgid( groupId() );
    if ( gr )
	return QFile::decodeName( gr->gr_name );
#endif
    return QString::null;
}

uint QFileInfo::groupId() const
{
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return fic->st.st_gid;
    return nobodyID;
}

bool QFileInfo::permission( int permissionSpec ) const
{
    if ( !fic || !cache )
	doStat();
    if ( fic ) {
	uint mask = 0;
	if ( permissionSpec & ReadUser)
	    mask |= S_IRUSR;
	if ( permissionSpec & WriteUser)
	    mask |= S_IWUSR;
	if ( permissionSpec & ExeUser)
	    mask |= S_IXUSR;
	if ( permissionSpec & ReadGroup)
	    mask |= S_IRGRP;
	if ( permissionSpec & WriteGroup)
	    mask |= S_IWGRP;
	if ( permissionSpec & ExeGroup)
	    mask |= S_IXGRP;
	if ( permissionSpec & ReadOther)
	    mask |= S_IROTH;
	if ( permissionSpec & WriteOther)
	    mask |= S_IWOTH;
	if ( permissionSpec & ExeOther)
	    mask |= S_IXOTH;
	if ( mask ) {
	   return (fic->st.st_mode & mask) == mask;
	} else {
#if defined(QT_CHECK_NULL)
	   qWarning( "QFileInfo::permission: permissionSpec is 0" );
#endif
	   return TRUE;
	}
    } else {
	return FALSE;
    }
}

void QFileInfo::doStat() const
{
    QFileInfo *that = ((QFileInfo*)this);	// mutable function
    if ( !that->fic )
	that->fic = new QFileInfoCache;
    QT_STATBUF *b = &that->fic->st;
#if defined(Q_OS_UNIX)
    that->symLink = FALSE;
#endif
    int r = QT_STAT( QFile::encodeName(QDir::convertSeparators(fn)), b );
    if ( r != 0 ) {
		delete that->fic;
		that->fic = 0;
    }
}

#ifndef QT_NO_DIR
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
#endif

QString QFileInfo::fileName() const
{
    int p = fn.findRev( '/' );
    if ( p == -1 ) {
	return fn;
    } else {
	return fn.mid( p + 1 );
    }
}
