/****************************************************************************
**
** Implementation of QFSFileInfoEngine class for Windows
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define _POSIX_
#include "qfileinfoengine.h"
#include "qfileinfoengine_p.h"
#include <qplatformdefs.h>
#include <qlibrary.h>
#include <qdir.h>

#include <private/qmutexpool_p.h>

#include <windows.h>
#include <direct.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <ctype.h>
#include <limits.h>

#include <accctrl.h>
#define SECURITY_WIN32
#include <security.h>

extern QString qt_fixToQtSlashes(const QString &path);
extern QByteArray qt_win95Name(const QString s);
Q_CORE_EXPORT int qt_ntfs_permission_lookup = 0;

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

static void resolveLibs()
{
#if !defined(QT_NO_COMPONENT)
    static bool triedResolve = false;
    if (!triedResolve) {
        // need to resolve the security info functions

        // protect initialization
        QMutexLocker locker(qt_global_mutexpool ?
            qt_global_mutexpool->get(&triedResolve) : 0);
        // check triedResolve again, since another thread may have already
        // done the initialization
        if (triedResolve) {
            // another thread did initialize the security function pointers,
            // so we shouldn't do it again.
            return;
        }

        triedResolve = true;
        if (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based) {
            QLibrary lib("advapi32");
            ptrGetNamedSecurityInfoW = (PtrGetNamedSecurityInfoW) lib.resolve("GetNamedSecurityInfoW");
            ptrLookupAccountSidW = (PtrLookupAccountSidW) lib.resolve("LookupAccountSidW");
            ptrAllocateAndInitializeSid = (PtrAllocateAndInitializeSid) lib.resolve("AllocateAndInitializeSid");
            ptrBuildTrusteeWithSidW = (PtrBuildTrusteeWithSidW) lib.resolve("BuildTrusteeWithSidW");
            ptrBuildTrusteeWithNameW = (PtrBuildTrusteeWithNameW) lib.resolve("BuildTrusteeWithNameW");
            ptrGetEffectiveRightsFromAclW = (PtrGetEffectiveRightsFromAclW) lib.resolve("GetEffectiveRightsFromAclW");
            ptrFreeSid = (PtrFreeSid) lib.resolve("FreeSid");
            if (ptrBuildTrusteeWithNameW) {
                QLibrary versionLib("version");
                typedef DWORD (WINAPI *PtrGetFileVersionInfoSizeW)(LPWSTR lptstrFilename,LPDWORD lpdwHandle);
                PtrGetFileVersionInfoSizeW ptrGetFileVersionInfoSizeW = (PtrGetFileVersionInfoSizeW)versionLib.resolve("GetFileVersionInfoSizeW");
                typedef BOOL (WINAPI *PtrGetFileVersionInfoW)(LPWSTR lptstrFilename,DWORD dwHandle,DWORD dwLen,LPVOID lpData);
                PtrGetFileVersionInfoW ptrGetFileVersionInfoW = (PtrGetFileVersionInfoW)versionLib.resolve("GetFileVersionInfoW");
                typedef BOOL (WINAPI *PtrVerQueryValueW)(const LPVOID pBlock,LPWSTR lpSubBlock,LPVOID *lplpBuffer,PUINT puLen);
                PtrVerQueryValueW ptrVerQueryValueW = (PtrVerQueryValueW)versionLib.resolve("VerQueryValueW");
                if (ptrGetFileVersionInfoSizeW && ptrGetFileVersionInfoW && ptrVerQueryValueW) {
                    DWORD fakeHandle;
                    DWORD versionSize = ptrGetFileVersionInfoSizeW(L"secur32.dll", &fakeHandle);
                    if (versionSize) {
                        LPVOID versionData;
                        versionData = malloc(versionSize);
                        if (ptrGetFileVersionInfoW(L"secur32.dll", 0, versionSize, versionData)) {
                            UINT puLen;
                            VS_FIXEDFILEINFO *pLocalInfo;
                            if (ptrVerQueryValueW(versionData, L"\\", (void**)&pLocalInfo, &puLen)) {
                                WORD wVer1, wVer2, wVer3, wVer4;
                                wVer1 = HIWORD(pLocalInfo->dwFileVersionMS);
                                wVer2 = LOWORD(pLocalInfo->dwFileVersionMS);
                                wVer3 = HIWORD(pLocalInfo->dwFileVersionLS);
                                wVer4 = LOWORD(pLocalInfo->dwFileVersionLS);
                                // It will not work with secur32.dll version 5.0.2195.2862
                                if (!(wVer1 == 5 && wVer2 == 0 && wVer3 == 2195 && (wVer4 == 2862 || wVer4 == 4587))) {
                                    QLibrary userLib("secur32");
                                    typedef BOOL (WINAPI *PtrGetUserNameExW)(EXTENDED_NAME_FORMAT nameFormat, ushort* lpBuffer, LPDWORD nSize);
                                    PtrGetUserNameExW ptrGetUserNameExW = (PtrGetUserNameExW)userLib.resolve("GetUserNameExW");
                                    if (ptrGetUserNameExW) {
                                        static TCHAR buffer[258];
                                        DWORD bufferSize = 257;
                                        ptrGetUserNameExW(NameSamCompatible, (ushort*)buffer, &bufferSize);
                                        ptrBuildTrusteeWithNameW(&currentUserTrusteeW, (ushort*)buffer);
                                    }
                                }
                            }
                        }
                        free(versionData);
                    }
                }
            }
        }
    }
#endif // QT_NO_COMPONENT
}

static QString currentDirOfDrive(char ch)
{
    QString ret;
    QT_WA({
        TCHAR currentName[PATH_MAX];
        if (_wgetdcwd(toupper((uchar) ch) - 'A' + 1, currentName, PATH_MAX) >= 0)
            ret = QString::fromUtf16((ushort*)currentName);
    } , {
        char currentName[PATH_MAX];
        if (_getdcwd(toupper((uchar) ch) - 'A' + 1, currentName, PATH_MAX) >= 0)
            ret = QString::fromLocal8Bit(currentName);
    });
    return qt_fixToQtSlashes(ret);
}

#define d d_func()
#define q q_func()

void
QFSFileInfoEnginePrivate::init()
{
}

bool
QFSFileInfoEnginePrivate::doStat() const
{
    if(!tried_stat) {
	tried_stat = true;
	could_stat = true;

        if (file.isEmpty())
            return could_stat;

	UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

        // Stat on windows doesn't accept drivename : without \ so append \ it if this is the case
        QString statName = file;
        if (file.length() == 2 && file.at(1) == ':')
            statName += '\\';

        int r;
	QT_WA({
	    r = QT_TSTAT((TCHAR*)statName.utf16(), (QT_STATBUF4TSTAT*)&st);
	} , {
	    r = QT_STAT(qt_win95Name(statName), &st);
	});
	if (r) {
	    bool is_dir=false;
	    if (file.at(0) == '/' && file.at(1) == '/'
		|| file.at(0) == '\\' && file.at(1) == '\\')
	    {
		// UNC - stat doesn't work for all cases (Windows bug)
		int s = file.indexOf(file.at(0),2);
		if (s > 0) {
		    // "\\server\..."
		    s = file.indexOf(file.at(0),s+1);
		    if (s > 0) {
			// "\\server\share\..."
			if (file.at(s+1) != 0) {
			    // "\\server\share\notfound"
			} else {
			    // "\\server\share\"
			    is_dir=true;
			}
		    } else {
			// "\\server\share"
			is_dir=true;
		    }
		} else {
		    // "\\server"
		    is_dir=true;
		}
	    }
	    if (is_dir) {
		// looks like a UNC dir, is a dir.
		memset(&st,0,sizeof(st));
		st.st_mode = QT_STAT_DIR;
		st.st_nlink = 1;
		r = 0;
	    }
	}
	if (r != 0)
	    could_stat = false;
	SetErrorMode(oldmode);
        return !r;
    }
    return could_stat;
}

QString
QFSFileInfoEnginePrivate::getLink() const
{
#if !defined(QT_NO_COMPONENT)
    QString ret;
    QT_WA({
        bool neededCoInit = false;
        IShellLink *psl;                            // pointer to IShellLink i/f
        HRESULT hres;
        WIN32_FIND_DATA wfd;
        TCHAR szGotPath[MAX_PATH];
        // Get pointer to the IShellLink interface.
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IShellLink, (LPVOID *)&psl);

        if (hres == CO_E_NOTINITIALIZED) { // COM was not initalized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLink, (LPVOID *)&psl);
        }
        if (SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if (SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)d->file.utf16(), STGM_READ);
                if (SUCCEEDED(hres)) {        // Resolve the link.

                    hres = psl->Resolve(0, SLR_ANY_MATCH);

                    if (SUCCEEDED(hres)) {
                        memcpy(szGotPath, (TCHAR*)d->file.utf16(), (d->file.length()+1)*sizeof(QChar));
                        hres = psl->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH);
                        ret = QString::fromUtf16((ushort*)szGotPath);
                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        if (neededCoInit)
            CoUninitialize();
    } , {
        bool neededCoInit = false;
        IShellLinkA *psl;                            // pointer to IShellLink i/f
        HRESULT hres;
        WIN32_FIND_DATAA wfd;
        QString ret;
        char szGotPath[MAX_PATH];
        // Get pointer to the IShellLink interface.

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IShellLinkA, (LPVOID *)&psl);

        if (hres == CO_E_NOTINITIALIZED) { // COM was not initalized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLinkA, (LPVOID *)&psl);
        }
        if (SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if (SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)d->file.utf16(), STGM_READ);
                if (SUCCEEDED(hres)) {        // Resolve the link.

                    hres = psl->Resolve(0, SLR_ANY_MATCH);

                    if (SUCCEEDED(hres)) {
                        QByteArray lfn = d->file.toLocal8Bit();
                        memcpy(szGotPath, lfn.data(), (lfn.length()+1)*sizeof(char));
                        hres = psl->GetPath((char*)szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH);
                        ret = QString::fromLocal8Bit(szGotPath);

                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        if (neededCoInit)
            CoUninitialize();
    });
    return ret;
#else
    return QString();
#endif // QT_NO_COMPONENT
}

uint
QFSFileInfoEnginePrivate::getPermissions() const
{
    int ret = 0;
    if ((qt_ntfs_permission_lookup > 0) && ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_NT)) {
	PSID pOwner = 0;
	PSID pGroup = 0;
	PACL pDacl;
        PSECURITY_DESCRIPTOR pSD;
        ACCESS_MASK access_mask;

        enum { ReadMask = 0x00000001, WriteMask = 0x00000002, ExecMask = 0x00000020 };
        resolveLibs();

        if (ptrGetNamedSecurityInfoW && ptrAllocateAndInitializeSid && ptrBuildTrusteeWithSidW && ptrGetEffectiveRightsFromAclW && ptrFreeSid) {
            DWORD res = ptrGetNamedSecurityInfoW((wchar_t*)file.utf16(), SE_FILE_OBJECT,
						 OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
						 &pOwner, &pGroup, &pDacl, 0, &pSD);

            if (res == ERROR_SUCCESS) {
                TRUSTEE_W trustee;
                { //user
                    if (ptrGetEffectiveRightsFromAclW(pDacl, &currentUserTrusteeW, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QFileInfoEngine::ReadUser;
		    if(access_mask & WriteMask)
			ret |= QFileInfoEngine::WriteUser;
		    if(access_mask & ExecMask)
			ret |= QFileInfoEngine::ExeUser;
                }
                { //owner
                    ptrBuildTrusteeWithSidW(&trustee, pOwner);
                    if (ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QFileInfoEngine::ReadOwner;
		    if(access_mask & WriteMask)
			ret |= QFileInfoEngine::WriteOwner;
		    if(access_mask & ExecMask)
			ret |= QFileInfoEngine::ExeOwner;
                }
                { //group
                    ptrBuildTrusteeWithSidW(&trustee, pGroup);
                    if (ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QFileInfoEngine::ReadGroup;
		    if(access_mask & WriteMask)
			ret |= QFileInfoEngine::WriteGroup;
		    if(access_mask & ExecMask)
			ret |= QFileInfoEngine::ExeGroup;
                }
                { //other (world)
                    // Create SID for Everyone (World)
                    SID_IDENTIFIER_AUTHORITY worldAuth = { SECURITY_WORLD_SID_AUTHORITY };
                    PSID pWorld = 0;
                    if (ptrAllocateAndInitializeSid(&worldAuth, 1, SECURITY_WORLD_RID, 0,0,0,0,0,0,0, &pWorld)) {
                        ptrBuildTrusteeWithSidW(&trustee, pWorld);
                        if (ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                            access_mask = (ACCESS_MASK)-1; // ###
			if(access_mask & ReadMask)
			    ret |= QFileInfoEngine::ReadOther;
			if(access_mask & WriteMask)
			    ret |= QFileInfoEngine::WriteOther;
			if(access_mask & ExecMask)
			    ret |= QFileInfoEngine::ExeOther;
                    }
                    ptrFreeSid(pWorld);
                }
                LocalFree(pSD);
            }
        }
    }
    if(ret & (QFileInfoEngine::WriteOwner | QFileInfoEngine::WriteUser |
	      QFileInfoEngine::WriteGroup | QFileInfoEngine::WriteOther)) {
	QT_WA({
	    DWORD attr = GetFileAttributes((TCHAR*)file.utf16());
	    if(attr & FILE_ATTRIBUTE_READONLY)
		if (attr & FILE_ATTRIBUTE_READONLY)
		    ret &= ~(QFileInfoEngine::WriteOwner | QFileInfoEngine::WriteUser |
			      QFileInfoEngine::WriteGroup | QFileInfoEngine::WriteOther);
	} , {
	    DWORD attr = GetFileAttributesA(file.local8Bit());
	    if (attr & FILE_ATTRIBUTE_READONLY)
		ret &= ~(QFileInfoEngine::WriteOwner | QFileInfoEngine::WriteUser |
			 QFileInfoEngine::WriteGroup | QFileInfoEngine::WriteOther);
	});
    }
    return ret;
}

uint
QFSFileInfoEngine::fileFlags(uint type) const
{
    uint ret = 0;
    if(type & PermsMask)
	ret = d->getPermissions();
    if(type & TypeMask) {
	if(d->doStat()) {
	    if((d->st.st_mode & S_IFMT) == S_IFREG)
		ret |= File;
	    if((d->st.st_mode & S_IFMT) == S_IFDIR)
		ret |= Directory;
	    if (d->file.endsWith(".lnk"))
		ret |= Link;
	}
    }
    if(type & FlagsMask) {
	if(d->doStat()) {
	    ret |= Exists;
	    if(fileName(BaseName)[0] == QChar('.')) {
		QT_WA({
		    if(GetFileAttributesW((TCHAR*)d->file.utf16()) & FILE_ATTRIBUTE_HIDDEN)
			ret |= Hidden;
		} , {
		    if(GetFileAttributesA(d->file.local8Bit()) & FILE_ATTRIBUTE_HIDDEN)
			ret |= Hidden;
		});
	    }
	}
    }
    return ret;
}

QString
QFSFileInfoEngine::fileName(FileName file) const
{
    if(file == BaseName) {
	int slash = d->file.lastIndexOf('/');
	if (slash == -1) {
	    int colon = d->file.lastIndexOf(':');
	    if (colon != -1)
		return d->file.mid(colon + 1);
	    return d->file;
	}
	return d->file.mid(slash + 1);
    } else if(file == DirPath) {
        if (!d->file.size())
            return d->file;
	int slash = d->file.lastIndexOf('/');
	if (slash == -1) {
	    if (d->file.at(1) == ':')
		return d->file.left(2);
	    return QString::fromLatin1(".");
	} else {
	    if (!slash)
		return QString::fromLatin1("/");
	    if (slash == 2 && d->file.at(1) == ':')
		slash++;
	    return d->file.left(slash);
	}
    } else if(file == AbsoluteName || file == AbsoluteDirPath) {
        QString ret;
        if (d->file.isEmpty()
            || (d->file.length() >=2 && d->file.at(0) != '/' && d->file.at(1) != ':')) {
            ret = QDir::currentDirPath();
        }
        if(!d->file.isEmpty() && d->file != ".") {
            if (!ret.isEmpty() && ret.right(1) != QString::fromLatin1("/"))
                ret += '/';
            ret += d->file;
        }
	if (ret[1] != ':' && ret[1] != '/') {
	    ret.prepend(":");
	    ret.prepend(_getdrive() + 'A' - 1);
	}
	if (ret[1] == ':' && ret.length() > 3 && ret[2] != '/') {
	    QString pwd = currentDirOfDrive((char)ret[0].latin1());
	    ret = pwd + "/" + ret.mid(2, 0xFFFFFF);
	}
        if(file == AbsoluteDirPath) {
            int slash = ret.lastIndexOf('/');
            if (slash == -1)
                return QString::fromLatin1(".");
            else if (!slash)
                return QString::fromLatin1("/");
            return ret.left(slash);
        }
        return ret;
    } else if(file == Canonical) {
        QString ret;
        QT_WA({
            TCHAR cur[PATH_MAX];
            ::_wgetcwd(cur, PATH_MAX);
            if (::_wchdir((TCHAR*)d->file.utf16()) >= 0) {
                TCHAR real[PATH_MAX];
                if (::_wgetcwd(real, PATH_MAX))
                    ret = QString::fromUtf16((ushort*)real);
            }
            ::_wchdir(cur);
        } , {
            char cur[PATH_MAX];
            QT_GETCWD(cur, PATH_MAX);
            if (QT_CHDIR(qt_win95Name(d->file)) >= 0) {
                char real[PATH_MAX];
                if (QT_GETCWD(real, PATH_MAX))
                    ret = QString::fromLocal8Bit(real);
            }
            QT_CHDIR(cur);
        });
        return qt_fixToQtSlashes(ret);
    } else if(file == LinkName) {
	return qt_fixToQtSlashes(d->getLink());
    }
    return d->file;
}

bool
QFSFileInfoEngine::isRelativePath() const
{
    if (d->file.length() >= 2) {
        return !((d->file.at(0).isLetter() && d->file.at(1) == ':') ||
                 (d->file.at(0) == '\\' && d->file.at(1) == '\\') ||
                 (d->file.at(0) == '/' && d->file.at(1) == '/'));                // drive, e.g. a:
    }
    return true;
}

uint
QFSFileInfoEngine::ownerId(FileOwner own) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString
QFSFileInfoEngine::owner(FileOwner own) const
{
    if ((qt_ntfs_permission_lookup > 0) && ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_NT)) {
	PSID pOwner = 0;
	PSECURITY_DESCRIPTOR pSD;
	QString name;
	resolveLibs();

	if (ptrGetNamedSecurityInfoW && ptrLookupAccountSidW) {
	    if (ptrGetNamedSecurityInfoW((wchar_t*)d->file.utf16(), SE_FILE_OBJECT,
					 own == Group ? GROUP_SECURITY_INFORMATION : OWNER_SECURITY_INFORMATION,
					 NULL, &pOwner, NULL, NULL, &pSD) == ERROR_SUCCESS) {
		DWORD lowner = 0, ldomain = 0;
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0').
		ptrLookupAccountSidW(NULL, pOwner, NULL, &lowner, NULL, &ldomain, (SID_NAME_USE*)&use);
		wchar_t *owner = new wchar_t[lowner];
		wchar_t *domain = new wchar_t[ldomain];
		// Second call, size is without '\0'
		if (ptrLookupAccountSidW(NULL, pOwner, (LPWSTR)owner, &lowner,
					 (LPWSTR)domain, &ldomain, (SID_NAME_USE*)&use)) {
		    name = QString::fromUtf16((ushort*)owner);
		}
		LocalFree(pSD);
		delete [] owner;
		delete [] domain;
	    }
	}
	return name;
    }
    return QString("");
}

