/****************************************************************************
** $Id$
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

#include "qplatformdefs.h"

#include "qlibrary.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"

#include <windows.h>
#ifndef Q_OS_TEMP
#include <direct.h>
#endif
#include <tchar.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>


// ### Can including accctrl.h cause problems on non-NT Win platforms?
#include <accctrl.h>


typedef DWORD (WINAPI *PtrGetNamedSecurityInfoW)(LPWSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION, PSID*, PSID*, PACL*, PACL*, PSECURITY_DESCRIPTOR*);
static PtrGetNamedSecurityInfoW ptrGetNamedSecurityInfoW = 0;
typedef DWORD (WINAPI *PtrGetNamedSecurityInfoA)(LPSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION, PSID*, PSID*, PACL*, PACL*, PSECURITY_DESCRIPTOR*);
static PtrGetNamedSecurityInfoA ptrGetNamedSecurityInfoA = 0;
typedef DECLSPEC_IMPORT BOOL (WINAPI *PtrLookupAccountSidW)(LPCWSTR, PSID, LPWSTR, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE);
static PtrLookupAccountSidW ptrLookupAccountSidW = 0;
typedef DECLSPEC_IMPORT BOOL (WINAPI *PtrLookupAccountSidA)(LPCSTR, PSID, LPSTR, LPDWORD, LPSTR, LPDWORD, PSID_NAME_USE);
static PtrLookupAccountSidA ptrLookupAccountSidA = 0;
typedef DECLSPEC_IMPORT BOOL (WINAPI *PtrAllocateAndInitializeSid)(PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID*);
static PtrAllocateAndInitializeSid ptrAllocateAndInitializeSid = 0;
typedef VOID (WINAPI *PtrBuildTrusteeWithSidW)(PTRUSTEE_W, PSID);
static PtrBuildTrusteeWithSidW ptrBuildTrusteeWithSidW = 0;
typedef VOID (WINAPI *PtrBuildTrusteeWithSidA)(PTRUSTEE_A, PSID);
static PtrBuildTrusteeWithSidA ptrBuildTrusteeWithSidA = 0;
typedef DWORD (WINAPI *PtrGetEffectiveRightsFromAclW)(PACL, PTRUSTEE_W, OUT PACCESS_MASK);
static PtrGetEffectiveRightsFromAclW ptrGetEffectiveRightsFromAclW = 0; 
typedef DWORD (WINAPI *PtrGetEffectiveRightsFromAclA)(PACL, PTRUSTEE_A, OUT PACCESS_MASK);
static PtrGetEffectiveRightsFromAclA ptrGetEffectiveRightsFromAclA = 0; 

static void resolveLibs()
{
    static bool triedResolve = FALSE;
    if ( !triedResolve ) {
	triedResolve = TRUE;
	if ( qWinVersion() & Qt::WV_NT_based ) {
	    QLibrary lib("advapi32");
	    lib.setAutoUnload( FALSE );

	    ptrGetNamedSecurityInfoW = (PtrGetNamedSecurityInfoW) lib.resolve( "GetNamedSecurityInfoW" );
	    ptrGetNamedSecurityInfoA = (PtrGetNamedSecurityInfoA) lib.resolve( "GetNamedSecurityInfoA" );
	    ptrLookupAccountSidW = (PtrLookupAccountSidW) lib.resolve( "LookupAccountSidW" );
	    ptrLookupAccountSidA = (PtrLookupAccountSidA) lib.resolve( "LookupAccountSidA" );
	    ptrAllocateAndInitializeSid = (PtrAllocateAndInitializeSid) lib.resolve( "AllocateAndInitializeSid" );
	    ptrBuildTrusteeWithSidW = (PtrBuildTrusteeWithSidW) lib.resolve( "BuildTrusteeWithSidW" );
	    ptrBuildTrusteeWithSidA = (PtrBuildTrusteeWithSidA) lib.resolve( "BuildTrusteeWithSidA" );
	    ptrGetEffectiveRightsFromAclW = (PtrGetEffectiveRightsFromAclW) lib.resolve( "GetEffectiveRightsFromAclW" );
	    ptrGetEffectiveRightsFromAclA = (PtrGetEffectiveRightsFromAclA) lib.resolve( "GetEffectiveRightsFromAclA" );
	}
    }
}


static QString currentDirOfDrive( char ch )
{
    QString result;

    if ( qt_winunicode ) {
	TCHAR currentName[PATH_MAX];
	if ( _tgetdcwd( toupper( (uchar) ch ) - 'A' + 1, currentName, PATH_MAX ) >= 0 ) {
	    result = qt_winQString(currentName);
	}
    } else {
	char currentName[PATH_MAX];
	if ( _getdcwd( toupper( (uchar) ch ) - 'A' + 1, currentName, PATH_MAX ) >= 0 ) {
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

bool QFileInfo::isSymLink() const
{
    if ( fn.right( 4 ) == ".lnk" )
        return TRUE;
    else
        return FALSE;
}

QString QFileInfo::readLink() const
{
#ifndef Q_OS_TEMP // ### What's this about, does this need supporting on CE?
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
            QString fn2 = fn + QChar();
            hres = ppf->Load( (LPOLESTR)fn2.unicode(), STGM_READ);
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
#else
    return QString();
#endif
}


QString QFileInfo::owner() const
{
    // ### What about CE?
    if ( qWinVersion() & Qt::WV_NT_based ) {

        PSID pOwner = 0;
	PSECURITY_DESCRIPTOR pSD;
	QString name;
	resolveLibs();

#if defined(UNICODE)
	if ( ptrGetNamedSecurityInfoW && ptrLookupAccountSidW ) {
	    if ( ptrGetNamedSecurityInfoW( (TCHAR*)qt_winTchar( fn, TRUE ), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pOwner, NULL, NULL, NULL, &pSD ) == ERROR_SUCCESS ) {
		DWORD lowner = 0, ldomain = 0; 
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0').
		ptrLookupAccountSidW( NULL, pOwner, NULL, &lowner, NULL, &ldomain, &use );
		TCHAR *owner = new TCHAR[lowner];
		TCHAR *domain = new TCHAR[ldomain];
		// Second call, size is without '\0'
		if ( ptrLookupAccountSidW( NULL, pOwner, (LPWSTR)owner, &lowner, (LPWSTR)domain, &ldomain, &use ) ) {
		    name = qt_winQString(owner);
		}
		LocalFree( pSD );
		delete owner;
		delete domain;
	    }
	}
#else
	if ( ptrGetNamedSecurityInfoA && ptrLookupAccountSidA ) {
	    if ( ptrGetNamedSecurityInfoA( (LPSTR)fn.local8Bit(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pOwner, NULL, NULL, NULL, &pSD ) == ERROR_SUCCESS ) {
		DWORD lowner = 0, ldomain = 0; 
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0'). 
		ptrLookupAccountSidA( NULL, pOwner, NULL, &lowner, NULL, &ldomain, &use );
		char *owner = new char[lowner];
		char *domain = new char[ldomain];
		// Second call, size is without '\0'
		if ( ptrLookupAccountSidA( NULL, pOwner, (LPSTR)owner, &lowner, (LPSTR)domain, &ldomain, &use ) ) {
		    name = owner;
		}
		LocalFree( pSD );
		delete owner;
		delete domain;
	    }
	}
#endif

	return name;

    } else 

	return QString::null;
}

static const uint nobodyID = (uint) -2;

uint QFileInfo::ownerId() const
{
    return nobodyID;
}

QString QFileInfo::group() const
{
    // ### What about CE?
    if ( qWinVersion() & Qt::WV_NT_based ) {

        PSID pGroup = 0;
	PSECURITY_DESCRIPTOR pSD;
	QString name;
	resolveLibs();

#if defined(UNICODE)
	if ( ptrGetNamedSecurityInfoW && ptrLookupAccountSidW ) {
	    if ( ptrGetNamedSecurityInfoW( (TCHAR*)qt_winTchar( fn, TRUE ), SE_FILE_OBJECT, GROUP_SECURITY_INFORMATION, NULL, &pGroup, NULL, NULL, &pSD ) == ERROR_SUCCESS ) {
		DWORD lgroup = 0, ldomain = 0; 
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0').
		ptrLookupAccountSidW( NULL, pGroup, NULL, &lgroup, NULL, &ldomain, &use );
		TCHAR *group = new TCHAR[lgroup];
		TCHAR *domain = new TCHAR[ldomain];
		// Second call, size is without '\0'
		if ( ptrLookupAccountSidW( NULL, pGroup, (LPWSTR)group, &lgroup, (LPWSTR)domain, &ldomain, &use ) ) {
		    name = qt_winQString(group);
		}
		LocalFree( pSD );
		delete group;
		delete domain;
	    }
	}
#else
	if ( ptrGetNamedSecurityInfoA && ptrLookupAccountSidA ) {
	    if ( ptrGetNamedSecurityInfoA( (LPSTR)fn.local8Bit(), SE_FILE_OBJECT, GROUP_SECURITY_INFORMATION, NULL, &pGroup, NULL, NULL, &pSD ) == ERROR_SUCCESS ) {
		DWORD lgroup = 0, ldomain = 0; 
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0'). 
		ptrLookupAccountSidA( NULL, pGroup, NULL, &lgroup, NULL, &ldomain, &use );
		char *group = new char[lgroup];
		char *domain = new char[ldomain];
		// Second call, size is without '\0'
		if ( ptrLookupAccountSidA( NULL, pGroup, (LPSTR)group, &lgroup, (LPSTR)domain, &ldomain, &use ) ) {
		    name = group;
		}
		LocalFree( pSD );
		delete group;
		delete domain;
	    }
	}
#endif

	return name;

    } else 

	return QString::null;
}

uint QFileInfo::groupId() const
{
    return nobodyID;
}


bool QFileInfo::permission( int p ) const
{
    // ###  What with CE version? Here is the previous function version:
/*#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
#if defined(UNICODE)
	if ( p & ( WriteUser | WriteGroup | WriteOther ) ) {
	    DWORD attr = GetFileAttributes( (TCHAR*)qt_winTchar( fn, TRUE ) );
	    if ( attr & FILE_ATTRIBUTE_READONLY )
		return FALSE;
	}
#else
#ifndef Q_OS_TEMP
	if ( p & ( WriteUser | WriteGroup | WriteOther ) ) {
	    DWORD attr = GetFileAttributesA( fn.local8Bit() );
	    if ( attr & FILE_ATTRIBUTE_READONLY )
		return FALSE;
	}
#endif
#endif
#ifndef Q_OS_TEMP
    } else { // only FAT anyway
	if ( p & ( WriteUser | WriteGroup | WriteOther ) ) {
	    DWORD attr = GetFileAttributesA( fn.local8Bit() );
	    if ( attr & FILE_ATTRIBUTE_READONLY )
		return FALSE;
	}
    }
#endif
    return TRUE;
*/

    if ( qWinVersion() & Qt::WV_NT_based ) {

	BOOL result = FALSE;
	PSID pOwner = 0, pGroup = 0;
	PACL pDacl;
	PSECURITY_DESCRIPTOR pSD;
	ACCESS_MASK access_mask;
	PACCESS_MASK pAccess = &access_mask; 
	enum { ReadMask = 0x00000001, WriteMask = 0x00000002, ExecMask = 0x00000020 };
    
#if defined(UNICODE)
        TRUSTEE_W trustee;
	PTRUSTEE_W pTrustee = &trustee;
	if ( ptrGetNamedSecurityInfoW && ptrAllocateAndInitializeSid && ptrBuildTrusteeWithSidW && ptrGetEffectiveRightsFromAclW ) {
	    if ( ptrGetNamedSecurityInfoW( (TCHAR*)qt_winTchar( fn, TRUE ), SE_FILE_OBJECT, 
			OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			&pOwner, &pGroup, &pDacl, NULL, &pSD ) == ERROR_SUCCESS ) {

		// Create SID for Everyone (World)
		SID_IDENTIFIER_AUTHORITY worldAuth = { SECURITY_WORLD_SID_AUTHORITY };
		PSID pWorld = 0;
		if ( ptrAllocateAndInitializeSid( &worldAuth, 1, SECURITY_WORLD_RID, 0,0,0,0,0,0,0, &pWorld ) ) {
    
		    result = TRUE;
		    
		    if ( p & ( ReadUser | WriteUser | ExeUser) ) {
			ptrBuildTrusteeWithSidW( pTrustee, pOwner );
			if ( ptrGetEffectiveRightsFromAclW( pDacl, pTrustee, pAccess ) != ERROR_SUCCESS )
			    *pAccess = 0;
			if ( ( p & ReadUser ) && !( *pAccess & ReadMask )   ||
			     ( p & WriteUser ) && !( *pAccess & WriteMask ) ||
			     ( p & ExeUser ) && !( *pAccess & ExecMask )      )
			    result = FALSE;
		    }
		    if ( p & ( ReadGroup | WriteGroup | ExeGroup) ) {
			ptrBuildTrusteeWithSidW( pTrustee, pGroup );
			if ( ptrGetEffectiveRightsFromAclW( pDacl, pTrustee, pAccess ) != ERROR_SUCCESS )
			    *pAccess = 0;
			if ( ( p & ReadGroup ) && !( *pAccess & ReadMask )   ||
			     ( p & WriteGroup ) && !( *pAccess & WriteMask ) ||
			     ( p & ExeGroup ) && !( *pAccess & ExecMask )      )
			    result = FALSE;
		    }
		    if ( p & ( ReadOther | WriteOther | ExeOther) ) {
			ptrBuildTrusteeWithSidW( pTrustee, pWorld );
			if ( ptrGetEffectiveRightsFromAclW( pDacl, pTrustee, pAccess ) != ERROR_SUCCESS )
			    *pAccess = 0;
			if ( ( p & ReadOther ) && !( *pAccess & ReadMask )   ||
			     ( p & WriteOther ) && !( *pAccess & WriteMask ) ||
			     ( p & ExeOther ) && !( *pAccess & ExecMask )      )
			    result = FALSE;
		    }
		    FreeSid( pWorld );
		}
		LocalFree( pSD );
	    } // if ( ptrGetNamedSecurityInfoW(..) == ERROR_SUCCESS )
	}
#else
        TRUSTEE_A trustee;
	PTRUSTEE_A pTrustee = &trustee;
	if ( ptrGetNamedSecurityInfoA && ptrAllocateAndInitializeSid && ptrBuildTrusteeWithSidA && ptrGetEffectiveRightsFromAclA ) {
	    if ( ptrGetNamedSecurityInfoA( (LPSTR)fn.local8Bit(), SE_FILE_OBJECT, 
			OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			&pOwner, &pGroup, &pDacl, NULL, &pSD ) == ERROR_SUCCESS ) {

		// Create SID for Everyone (World)
		SID_IDENTIFIER_AUTHORITY worldAuth = { SECURITY_WORLD_SID_AUTHORITY };
		PSID pWorld = 0;
		if ( ptrAllocateAndInitializeSid( &worldAuth, 1, SECURITY_WORLD_RID, 0,0,0,0,0,0,0, &pWorld ) ) {
    
		    result = TRUE;
		    
		    if ( p & ( ReadUser | WriteUser | ExeUser) ) {
			ptrBuildTrusteeWithSidA( pTrustee, pOwner );
			if ( ptrGetEffectiveRightsFromAclA( pDacl, pTrustee, pAccess ) != ERROR_SUCCESS )
			    *pAccess = 0;
			if ( ( p & ReadUser ) && !( *pAccess & ReadMask )   ||
			     ( p & WriteUser ) && !( *pAccess & WriteMask ) ||
			     ( p & ExeUser ) && !( *pAccess & ExecMask )      )
			    result = FALSE;
		    }
		    if ( p & ( ReadGroup | WriteGroup | ExeGroup) ) {
			ptrBuildTrusteeWithSidA( pTrustee, pGroup );
			if ( ptrGetEffectiveRightsFromAclA( pDacl, pTrustee, pAccess ) != ERROR_SUCCESS )
			    *pAccess = 0;
			if ( ( p & ReadGroup ) && !( *pAccess & ReadMask )   ||
			     ( p & WriteGroup ) && !( *pAccess & WriteMask ) ||
			     ( p & ExeGroup ) && !( *pAccess & ExecMask )      )
			    result = FALSE;
		    }
		    if ( p & ( ReadOther | WriteOther | ExeOther) ) {
			ptrBuildTrusteeWithSidA( pTrustee, pWorld );
			if ( ptrGetEffectiveRightsFromAclA( pDacl, pTrustee, pAccess ) != ERROR_SUCCESS )
			    *pAccess = 0;
			if ( ( p & ReadOther ) && !( *pAccess & ReadMask )   ||
			     ( p & WriteOther ) && !( *pAccess & WriteMask ) ||
			     ( p & ExeOther ) && !( *pAccess & ExecMask )      )
			    result = FALSE;
		    }
		    FreeSid( pWorld );
		}
		LocalFree( pSD );
	    } // if ( ptrGetNamedSecurityInfoA(..) == ERROR_SUCCESS )
	}
#endif

	return result;

    } else {

	// for non-NT versions, just check if it's ReadOnly
#if defined(UNICODE)
	if ( p & ( WriteUser | WriteGroup | WriteOther ) ) {
	    DWORD attr = GetFileAttributesW( (TCHAR*)qt_winTchar( fn, TRUE ) );
	    if ( attr & FILE_ATTRIBUTE_READONLY )
		return FALSE;
	}
#else
	if ( p & ( WriteUser | WriteGroup | WriteOther ) ) {
	    DWORD attr = GetFileAttributesA( fn.local8Bit() );
	    if ( attr & FILE_ATTRIBUTE_READONLY )
		return FALSE;
	}
#endif
	return TRUE;
    }
}

void QFileInfo::doStat() const
{
#ifndef Q_OS_TEMP
    UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
    QFileInfo *that = ((QFileInfo*)this);	// mutable function
    if ( !that->fic )
	that->fic = new QFileInfoCache;
    QT_STATBUF *b = &that->fic->st;

    int r;
    if ( qt_winunicode )
	r = _tstat((const TCHAR*)qt_winTchar(fn,TRUE), (QT_STATBUF4TSTAT*)b);
    else
	r = QT_STAT(qt_win95Name(fn), b);
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

#ifndef Q_OS_TEMP
    SetErrorMode(oldmode);
#endif
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
