/****************************************************************************
** $Id$
**
** Implementation of QFileInfo class
**
** Created : 950628
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
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

#include "qplatformdefs.h"

#include "qlibrary.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qapplication.h"

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#include <windows.h>
#ifndef Q_OS_TEMP
#include <direct.h>
#endif
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <ctype.h>
#include <limits.h>

#ifndef Q_OS_TEMP

#include <accctrl.h>
#define SECURITY_WIN32
#include <security.h>


typedef DWORD (WINAPI *PtrGetNamedSecurityInfoW)(LPWSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION, PSID*, PSID*, PACL*, PACL*, PSECURITY_DESCRIPTOR*);
static PtrGetNamedSecurityInfoW ptrGetNamedSecurityInfoW = 0;
typedef DECLSPEC_IMPORT BOOL (WINAPI *PtrLookupAccountSidW)(LPCWSTR, PSID, LPWSTR, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE);
static PtrLookupAccountSidW ptrLookupAccountSidW = 0;
typedef DECLSPEC_IMPORT BOOL (WINAPI *PtrAllocateAndInitializeSid)(PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID*);
static PtrAllocateAndInitializeSid ptrAllocateAndInitializeSid = 0;
typedef VOID (WINAPI *PtrBuildTrusteeWithSidW)(PTRUSTEE_W, PSID);
static PtrBuildTrusteeWithSidW ptrBuildTrusteeWithSidW = 0;
typedef VOID (WINAPI *PtrBuildTrusteeWithNameW)(PTRUSTEE_W, unsigned short*);
static PtrBuildTrusteeWithNameW ptrBuildTrusteeWithNameW = 0;
typedef DWORD (WINAPI *PtrGetEffectiveRightsFromAclW)(PACL, PTRUSTEE_W, OUT PACCESS_MASK);
static PtrGetEffectiveRightsFromAclW ptrGetEffectiveRightsFromAclW = 0;
typedef DECLSPEC_IMPORT PVOID (WINAPI *PtrFreeSid)(PSID);
static PtrFreeSid ptrFreeSid = 0;

static TRUSTEE_W currentUserTrusteeW;

#endif


static void resolveLibs()
{
}

static QString currentDirOfDrive( char ch )
{
    return QDir::currentDirPath();
}


void QFileInfo::slashify( QString &s )
{
    for (int i=0; i<(int)s.length(); i++) {
	if ( s[i] == '\\' )
	    s[i] = '/';
    }
    if ( s[ (int)s.length() - 1 ] == '/' && s.length() > 3 )
	s.remove( (int)s.length() - 1, 1 );
}

void QFileInfo::makeAbs( QString &s )
{
    if ( s[0] != '/' ) {
	QString d = QDir::currentDirPath();
	slashify( d );
	s.prepend( d + "/" );
    }
}

extern QCString qt_win95Name(const QString s);

bool QFileInfo::isFile() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & QT_STAT_MASK) == QT_STAT_REG : FALSE;
}

bool QFileInfo::isDir() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & QT_STAT_MASK) == QT_STAT_DIR : FALSE;
}

bool QFileInfo::isSymLink() const
{
    if ( fn.right( 4 ) == ".lnk" )
        return TRUE;
    else
        return FALSE;
}

QString QFileInfo::readLink() const
{
    return QString::null;
}

Q_EXPORT int qt_ntfs_permission_lookup = 1;
QString QFileInfo::owner() const
{
    return QString::null;
}

static const uint nobodyID = (uint) -2;
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

bool QFileInfo::permission( int p ) const
{
    // just check if it's ReadOnly
    if ( p & ( WriteUser | WriteGroup | WriteOther ) ) {
	DWORD attr = GetFileAttributes( (TCHAR*)fn.ucs2() );
	if ( attr & FILE_ATTRIBUTE_READONLY )
	    return FALSE;
    }
    return TRUE;
}

void QFileInfo::doStat() const
{
    if ( fn.isEmpty() )
	return;

    QFileInfo *that = ((QFileInfo*)this);	// mutable function
    if ( !that->fic )
	that->fic = new QFileInfoCache;
    QT_STATBUF *b = &that->fic->st;

    int r;
    QT_WA( {
	r = QT_TSTAT((TCHAR*)fn.ucs2(), (QT_STATBUF4TSTAT*)b);
    } , {
	r = QT_STAT(qt_win95Name(fn), b);
    } );
    if ( r!=0 ) {
	bool is_dir=FALSE;
	if ( fn[0] == '/' && fn[1] == '/'
	  || fn[0] == '\\' && fn[1] == '\\' )
	{
	    // UNC - stat doesn't work for all cases (Windows bug)
	    int s = fn.find(fn[0],2);
	    if ( s > 0 ) {
		// "\\server\..."
		s = fn.find(fn[0],s+1);
		if ( s > 0 ) {
		    // "\\server\share\..."
		    if ( fn[s+1] ) {
			// "\\server\share\notfound"
		    } else {
			// "\\server\share\"
			is_dir=TRUE;
		    }
		} else {
		    // "\\server\share"
		    is_dir=TRUE;
		}
	    } else {
		// "\\server"
		is_dir=TRUE;
	    }
	}
	if ( is_dir ) {
	    // looks like a UNC dir, is a dir.
	    memset(b,0,sizeof(*b));
	    b->st_mode = QT_STAT_DIR;
	    b->st_nlink = 1;
	    r = 0;
	}
    }

    if ( r != 0 ) {
	delete that->fic;
	that->fic = 0;
    }
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
	if ( s[ 2 ] == '/' )
	    return s.left( 3 );
	if ( s[ 1 ] == ':' ) {
	    if ( absPath )
		return s.left( 2 ) + "/";
	    return s.left( 2 );
	}
	return QString::fromLatin1(".");
    } else {
	if ( pos == 0 )
	    return QString::fromLatin1( "/" );
	if ( pos == 2 && s[ 1 ] == ':'  && s[ 2 ] == '/')
	    pos++;
	return s.left( pos );
    }
}

QString QFileInfo::fileName() const
{
    int p = fn.findRev( '/' );
    if ( p == -1 ) {
	int p = fn.findRev( ':' );
	if ( p != -1 )
	    return fn.mid( p + 1 );
	return fn;
    } else {
	return fn.mid( p + 1 );
    }
}

/*!
    Returns TRUE if the file is hidden; otherwise returns FALSE.

    On Unix-like operating systems, including Mac OS X, a file is
    hidden if its name begins with ".". On Windows a file is hidden if
    its hidden attribute is set.
*/
bool QFileInfo::isHidden() const
{
    return GetFileAttributes( (TCHAR*)fn.ucs2() ) & FILE_ATTRIBUTE_HIDDEN;
}
