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

#include "qglobal.h"

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"

#include <windows.h>
#include <direct.h>
#include <tchar.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>

static QString currentDirOfDrive( char ch )
{
    QString result;

    if ( qt_winunicode ) {
	TCHAR currentName[PATH_MAX];
	if ( _tgetdcwd( toupper( ch ) - 'A' + 1, currentName, PATH_MAX ) >= 0 ) {
	    result = qt_winQString(currentName);
	}
    } else {
	char currentName[PATH_MAX];
	if ( _getdcwd( toupper( ch ) - 'A' + 1, currentName, PATH_MAX ) >= 0 ) {
	    result = QString::fromLatin1(currentName);
	}
    }
    return result;
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
    if ( s[ 1 ] != ':' && s[ 1 ] != '/' ) {
	s.prepend( ":" );
	s.prepend( _getdrive() + 'A' - 1 );
    }
    if ( s[ 1 ] == ':' && s.length() > 3 && s[ 2 ] != '/' ) {
	QString d = currentDirOfDrive( (char)s[ 0 ].latin1() );
	slashify( d );
	s = d + "/" + s.mid( 2, 0xFFFFFF );
    }
}

extern QCString qt_win95Name(const QString s);

extern bool qt_file_access( const QString & fn, int t );

bool QFileInfo::isFile() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & STAT_MASK) == STAT_REG : FALSE;
}

bool QFileInfo::isDir() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & STAT_MASK) == STAT_DIR : FALSE;
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
    IShellLink *psl;                            // pointer to IShellLink i/f
    HRESULT hres;
    WIN32_FIND_DATA wfd;
    QString fileLinked;
    char szGotPath[MAX_PATH];
    // Get pointer to the IShellLink interface.

    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                IID_IShellLink, (LPVOID *)&psl);

    if (SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
        IPersistFile *ppf;
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
        if (SUCCEEDED(hres))  {
            hres = ppf->Load( (const unsigned short*)qt_winTchar(fn, TRUE), STGM_READ);
            if (SUCCEEDED(hres)) {        // Resolve the link.

                hres = psl->Resolve(0, SLR_ANY_MATCH);

                if (SUCCEEDED(hres)) {
                    qstrcpy(szGotPath, fn.latin1());

                    hres = psl->GetPath((TCHAR*)szGotPath, MAX_PATH,
                                (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH);

                    fileLinked = qt_winQString(szGotPath);
                    
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    return fileLinked;
}


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


bool QFileInfo::permission( int ) const
{
    return TRUE;
}

uint QFileInfo::size() const
{
    if ( !fic || !cache )
	doStat();
    if ( fic )
	return (uint)fic->st.st_size;
    else
	return 0;
}


QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;
    if ( !fic || !cache )
	doStat();
    if ( fic )
	dt.setTime_t( fic->st.st_mtime );
    return dt;
}

QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;
    if ( !fic || !cache )
	doStat();
    if ( fic )
	dt.setTime_t( fic->st.st_atime );
    return dt;
}


void QFileInfo::doStat() const
{
    UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS);
    QFileInfo *that = ((QFileInfo*)this);	// mutable function
    if ( !that->fic )
	that->fic = new QFileInfoCache;
    STATBUF *b = &that->fic->st;

    int r;
    if ( qt_winunicode )
	r = _tstat((const TCHAR*)qt_winTchar(fn,TRUE), (STATBUF4TSTAT*)b);
    else
	r = STAT(qt_win95Name(fn), b);
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
	    b->st_mode = STAT_DIR;
	    b->st_nlink = 1;
	    r = 0;
	}
    }

    if ( r != 0 ) {
	delete that->fic;
	that->fic = 0;
    }

    SetErrorMode(oldmode);
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
	return fn.mid(p+1);
    }
}

QString QFileInfo::absFilePath() const
{
    if ( QDir::isRelativePath(fn) && fn[ 1 ] != ':' ) {
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
