/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define _POSIX_
#include <qplatformdefs.h>
#include <qfileengine.h>
#include <private/qfsfileengine_p.h>

#include <qfile.h>
#include <qdir.h>
#include <qtemporaryfile.h>
#ifndef QT_NO_REGEXP
# include <qregexp.h>
#endif
#include <private/qmutexpool_p.h>

#define d d_func()
#define q q_func()

#include <sys/types.h>
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


Q_CORE_EXPORT int qt_ntfs_permission_lookup = 0;

#if !defined(QT_NO_COMPONENT)
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
    static bool triedResolve = false;
    if(!triedResolve) {
        // need to resolve the security info functions

        // protect initialization
#ifdef QT_THREAD_SUPPORT
        QMutexLocker locker(qt_global_mutexpool ?
            qt_global_mutexpool->get(&triedResolve) : 0);
        // check triedResolve again, since another thread may have already
        // done the initialization
        if(triedResolve) {
            // another thread did initialize the security function pointers,
            // so we shouldn't do it again.
            return;
        }
#endif

        triedResolve = true;
        if(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based) {
	    HINSTANCE advapiHnd = LoadLibraryW(L"advapi32");
            ptrGetNamedSecurityInfoW = (PtrGetNamedSecurityInfoW)GetProcAddress(advapiHnd, "GetNamedSecurityInfoW");
            ptrLookupAccountSidW = (PtrLookupAccountSidW)GetProcAddress(advapiHnd, "LookupAccountSidW");
            ptrAllocateAndInitializeSid = (PtrAllocateAndInitializeSid)GetProcAddress(advapiHnd, "AllocateAndInitializeSid");
            ptrBuildTrusteeWithSidW = (PtrBuildTrusteeWithSidW)GetProcAddress(advapiHnd, "BuildTrusteeWithSidW");
            ptrBuildTrusteeWithNameW = (PtrBuildTrusteeWithNameW)GetProcAddress(advapiHnd, "BuildTrusteeWithNameW");
            ptrGetEffectiveRightsFromAclW = (PtrGetEffectiveRightsFromAclW)GetProcAddress(advapiHnd, "GetEffectiveRightsFromAclW");
            ptrFreeSid = (PtrFreeSid)GetProcAddress(advapiHnd, "FreeSid");
            if(ptrBuildTrusteeWithNameW) {
		HINSTANCE versionHnd = LoadLibraryW(L"version");
                typedef DWORD (WINAPI *PtrGetFileVersionInfoSizeW)(LPWSTR lptstrFilename,LPDWORD lpdwHandle);
                PtrGetFileVersionInfoSizeW ptrGetFileVersionInfoSizeW = (PtrGetFileVersionInfoSizeW)GetProcAddress(versionHnd, "GetFileVersionInfoSizeW");
                typedef BOOL (WINAPI *PtrGetFileVersionInfoW)(LPWSTR lptstrFilename,DWORD dwHandle,DWORD dwLen,LPVOID lpData);
                PtrGetFileVersionInfoW ptrGetFileVersionInfoW = (PtrGetFileVersionInfoW)GetProcAddress(versionHnd, "GetFileVersionInfoW");
                typedef BOOL (WINAPI *PtrVerQueryValueW)(const LPVOID pBlock,LPWSTR lpSubBlock,LPVOID *lplpBuffer,PUINT puLen);
                PtrVerQueryValueW ptrVerQueryValueW = (PtrVerQueryValueW)GetProcAddress(versionHnd, "VerQueryValueW");
                if(ptrGetFileVersionInfoSizeW && ptrGetFileVersionInfoW && ptrVerQueryValueW) {
                    DWORD fakeHandle;
                    DWORD versionSize = ptrGetFileVersionInfoSizeW(L"secur32.dll", &fakeHandle);
                    if(versionSize) {
                        LPVOID versionData;
                        versionData = malloc(versionSize);
                        if(ptrGetFileVersionInfoW(L"secur32.dll", 0, versionSize, versionData)) {
                            UINT puLen;
                            VS_FIXEDFILEINFO *pLocalInfo;
                            if(ptrVerQueryValueW(versionData, L"\\", (void**)&pLocalInfo, &puLen)) {
                                WORD wVer1, wVer2, wVer3, wVer4;
                                wVer1 = HIWORD(pLocalInfo->dwFileVersionMS);
                                wVer2 = LOWORD(pLocalInfo->dwFileVersionMS);
                                wVer3 = HIWORD(pLocalInfo->dwFileVersionLS);
                                wVer4 = LOWORD(pLocalInfo->dwFileVersionLS);
                                // It will not work with secur32.dll version 5.0.2195.2862
                                if(!(wVer1 == 5 && wVer2 == 0 && wVer3 == 2195 && (wVer4 == 2862 || wVer4 == 4587))) {
				    HINSTANCE userHnd = LoadLibraryW(L"secur32");
                                    typedef BOOL (WINAPI *PtrGetUserNameExW)(EXTENDED_NAME_FORMAT nameFormat, ushort* lpBuffer, LPDWORD nSize);
                                    PtrGetUserNameExW ptrGetUserNameExW = (PtrGetUserNameExW)GetProcAddress(userHnd, "GetUserNameExW");
                                    if(ptrGetUserNameExW) {
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
}
#endif // QT_NO_COMPONENT

QString QFSFileEnginePrivate::fixToQtSlashes(const QString &path)
{
    if(!path.length())
        return path;

    QString ret;
    for (int i=0, lastSlash=-1; i<(int)path.length(); i++) {
	if(path[i] == '/' || path[i] == '\\') {
	    if(i <= 3 || (i < path.length()) && (lastSlash == -1 || i != lastSlash+1))
		ret += '/';
	    lastSlash = i;
	} else {
	    ret += path[i];
	}
    }
    return ret;
}

QByteArray QFSFileEnginePrivate::win95Name(const QString &path)
{
    QString ret(path);
    if(path.length() > 1 && path[0] == '/' && path[1] == '/') {
        // Win95 cannot handle slash-slash needs slosh-slosh.
        ret[0] = '\\';
        ret[1] = '\\';
        int n = ret.indexOf('/');
        if(n >= 0)
            ret[n] = '\\';
    } else if(path.length() > 3 && path[2] == '/' && path[3] == '/') {
        ret[2] = '\\';
        ret.remove(3, 1);
        int n = ret.indexOf('/');
        if(n >= 0)
            ret[n] = '\\';
    }
    return ret.toLocal8Bit();
}

bool isValidFile(const QString& fileName)
{
    // Only character : needs to be checked for, other invalid characters
    // are currently checked by open()
    int findColon = fileName.lastIndexOf(':');
    if(findColon == -1)
        return true;
    else if(findColon != 1)
        return false;
    else
        return fileName[0].isLetter();
}

void
QFSFileEnginePrivate::init()
{
}

int
QFSFileEnginePrivate::sysOpen(const QString &fileName, int flags)
{
    QT_WA({
	return ::_wopen((TCHAR*)fileName.utf16(), flags, _S_IREAD | _S_IWRITE);
    } , {
	return QT_OPEN(QFSFileEnginePrivate::win95Name(fileName), flags, _S_IREAD | _S_IWRITE);
    });
}
#include <sys/stat.h>
bool
QFSFileEngine::remove()
{
    QT_WA({
        return ::DeleteFileW((TCHAR*)d->file.utf16()) != 0;
    } , {
        return ::DeleteFileA(QFSFileEnginePrivate::win95Name(d->file)) != 0;
    });
}

bool
QFSFileEngine::copy(const QString &copyName)
{
    QT_WA({
        return ::CopyFileW((TCHAR*)d->file.utf16(), (TCHAR*)copyName.utf16(), false) != 0;
    } , {
        return ::CopyFileA(QFSFileEnginePrivate::win95Name(d->file),
                           QFSFileEnginePrivate::win95Name(copyName), false) != 0;
    });
}

bool
QFSFileEngine::rename(const QString &newName)
{
    QT_WA({
        return ::MoveFileW((TCHAR*)d->file.utf16(), (TCHAR*)newName.utf16()) != 0;
    } , {
        return ::MoveFileA(QFSFileEnginePrivate::win95Name(d->file),
                           QFSFileEnginePrivate::win95Name(newName)) != 0;
    });
}

qint64
QFSFileEngine::size() const
{
    QT_STATBUF st;
    int ret = 0;
    if(d->fd != -1) {
        ret = QT_FSTAT(d->fd, &st);
    } else {
        QT_WA({
            ret = QT_TSTAT((TCHAR*)d->file.utf16(), (QT_STATBUF4TSTAT*)&st);
        } , {
            ret = QT_STAT(QFSFileEnginePrivate::win95Name(d->file), &st);
        });
    }
    if(ret == -1)
        return 0;
    return st.st_size;
}

bool
QFSFileEngine::mkdir(const QString &name, bool createParentDirectories) const
{
    QString dirName = name;
    if(createParentDirectories) {
        dirName = QDir::convertSeparators(QDir::cleanPath(dirName));
        // We spefically search for / so \ would break it..
        for(int oldslash = -1, slash=0; slash != -1; oldslash = slash) {
            slash = dirName.indexOf(QDir::separator(), oldslash+1);
            if(slash == -1) {
                if(oldslash == dirName.length())
                    break;
                slash = dirName.length();
            }
            if(slash) {
                QString chunk = dirName.left(slash);
                QT_STATBUF st;
                QT_WA({
                    if(QT_TSTAT((TCHAR*)chunk.utf16(), (QT_STATBUF4TSTAT*)&st) != -1) {
                        if((st.st_mode & S_IFMT) != S_IFDIR) {
                            return false;
			}
                    } else if(::_wmkdir((TCHAR*)chunk.utf16()) == -1) {
                        if (errno != EEXIST) {
                            return false;
			}
                    }
                } , {
                    if(QT_STAT(QFSFileEnginePrivate::win95Name(chunk), &st) != -1) {
                        if((st.st_mode & S_IFMT) != S_IFDIR) {
                            return false;
			}
		    } else if(_mkdir(QFSFileEnginePrivate::win95Name(chunk)) == -1) {
			if (errno != EEXIST) {
			    return false;
			}
		    }
                });
            }
        }
        return true;
    }
    QT_WA({
        return ::_wmkdir((TCHAR*)QDir::convertSeparators(dirName).utf16()) != -1;
    }, {
        return _mkdir(QFSFileEnginePrivate::win95Name(dirName)) != -1;
    });
}

bool
QFSFileEngine::rmdir(const QString &name, bool recurseParentDirectories) const
{
    QString dirName = name;
    if(recurseParentDirectories) {
        dirName = QDir::convertSeparators(QDir::cleanPath(dirName));
        for(int oldslash = 0, slash=dirName.length(); slash > 0; oldslash = slash) {
            QString chunk = dirName.left(slash);
            if (chunk.length() == 2 && chunk.at(0).isLetter() && chunk.at(1) == QLatin1Char(':'))
                break;
            QT_STATBUF st;
            QT_WA({
                if(QT_TSTAT((TCHAR*)chunk.utf16(), (QT_STATBUF4TSTAT*)&st) != -1) {
                    if((st.st_mode & S_IFMT) != S_IFDIR)
                        return false;
                    else if(::_wrmdir((TCHAR*)chunk.utf16()) == -1)
                        return oldslash != 0;
                }
            } , {
                if(QT_STAT(QFSFileEnginePrivate::win95Name(chunk), &st) != -1) {
                    if((st.st_mode & S_IFMT) != S_IFDIR) {
                        return false;
                    } else if(_rmdir(QFSFileEnginePrivate::win95Name(chunk)) == -1)
                        return oldslash != 0;
                }
            });
            slash = dirName.lastIndexOf(QDir::separator(), oldslash-1);
        }
        return true;
    }
    QT_WA({
        return ::_wrmdir((TCHAR*)QDir::convertSeparators(dirName).utf16()) != -1;
    } , {
        return _rmdir(QFSFileEnginePrivate::win95Name(dirName)) != -1;
    });
}

QStringList
QFSFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    QStringList ret;

    bool doDirs     = (filters & QDir::Dirs)!= 0;
    bool doFiles    = (filters & QDir::Files) != 0;
    bool noSymLinks = (filters & QDir::NoSymLinks) != 0;
    bool doReadable = (filters & QDir::Readable) != 0;
    bool doWritable = (filters & QDir::Writable) != 0;
    bool doExecable = (filters & QDir::Executable) != 0;
    bool doModified = (filters & QDir::Modified) != 0;
    bool doSystem   = (filters & QDir::System) != 0;
    bool doHidden   = (filters & QDir::Hidden) != 0;
    // show hidden files if the user asks explicitly for e.g. .*
    if(!doHidden && !filterNames.size()) {
        QStringList::ConstIterator sit = filterNames.begin();
        while (sit != filterNames.end()) {
            if((*sit)[0] == '.') {
                doHidden = true;
                break;
            }
            ++sit;
        }
    }

    bool first = true;
    QString p = d->file;
    const int plen = p.length();
    HANDLE ff;
    WIN32_FIND_DATA finfo;
    QFileInfo fi;

#undef IS_SUBDIR
#undef IS_RDONLY
#undef IS_ARCH
#undef IS_HIDDEN
#undef IS_SYSTEM
#undef FF_ERROR

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH     FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
#define FF_ERROR    INVALID_HANDLE_VALUE

    if(plen == 0) {
        qWarning("QDir::readDirEntries: No directory name specified");
        return ret;
    }
    if(p.at(plen-1) != '/' && p.at(plen-1) != '\\')
        p += QLatin1Char('/');
    p += QLatin1String("*.*");

    QT_WA({
        ff = FindFirstFile((TCHAR*)p.utf16(), &finfo);
    }, {
        // Cast is safe, since char is at end of WIN32_FIND_DATA
        ff = FindFirstFileA(QFSFileEnginePrivate::win95Name(p),
			    (WIN32_FIND_DATAA*)&finfo);
    });

    if(ff == FF_ERROR)
        return ret; // cannot read the directory

    for (;;) {
        if(first)
            first = false;
        else {
            QT_WA({
                if(!FindNextFile(ff,&finfo))
                    break;
            } , {
                if(!FindNextFileA(ff,(WIN32_FIND_DATAA*)&finfo))
                    break;
            });
        }
        int  attrib = finfo.dwFileAttributes;

        bool isDir      = (attrib & IS_SUBDIR) != 0;
        bool isFile     = !isDir;
        bool isSymLink  = false;
        bool isReadable = true;
        bool isWritable = (attrib & IS_RDONLY) == 0;
        bool isExecable = false;
        bool isModified = (attrib & IS_ARCH)   != 0;
        bool isHidden   = (attrib & IS_HIDDEN) != 0;
        bool isSystem   = (attrib & IS_SYSTEM) != 0;

        QString fname;
        QT_WA({
            fname = QString::fromUtf16((unsigned short *)finfo.cFileName);
        } , {
            fname = QString::fromLocal8Bit((const char*)finfo.cFileName);
        });

#ifndef QT_NO_REGEXP
        if(!(filters & QDir::AllDirs && isDir)) {
            bool matched = false;
            for(QStringList::ConstIterator sit = filterNames.begin(); sit != filterNames.end(); ++sit) {
                QRegExp rx(*sit, Qt::CaseInsensitive, QRegExp::Wildcard);
                if(rx.exactMatch(fname))
                    matched = true;
            }
            if(!matched)
                continue;
        }
#else
        Q_UNUSED(filterNames);
#endif
        if  ((doDirs && isDir) || (doFiles && isFile)) {
            QString name = QFSFileEnginePrivate::fixToQtSlashes(fname);
            if(doExecable) {
                QString ext = name.right(4).toLower();
                if(ext == ".exe" || ext == ".com" || ext == ".bat" ||
                     ext == ".pif" || ext == ".cmd")
                    isExecable = true;
            }

            if(noSymLinks && isSymLink)
                continue;
            if((filters & QDir::PermissionMask) != 0)
                if((doReadable && !isReadable) ||
                     (doWritable && !isWritable) ||
                     (doExecable && !isExecable))
                    continue;
            if(doModified && !isModified)
                continue;
            if(!doHidden && isHidden)
                continue;
            if(!doSystem && isSystem)
                continue;
	    ret.append(name);
        }
    }
    FindClose(ff);

#undef        IS_SUBDIR
#undef        IS_RDONLY
#undef        IS_ARCH
#undef        IS_HIDDEN
#undef        IS_SYSTEM
#undef        FF_ERROR
    return ret;
}

bool
QFSFileEngine::caseSensitive() const
{
    return false;
}

bool
QFSFileEngine::setCurrentPath(const QString &path)
{
    if (!QDir(path).exists())
        return false;

    int r;
    QT_WA({
        r = ::SetCurrentDirectory((WCHAR*)path.utf16());
    } , {
        r = QT_CHDIR(QFSFileEnginePrivate::win95Name(path));
    });
    return r >= 0;
}

QString
QFSFileEngine::currentPath(const QString &fileName)
{
    QString ret;
    //if filename is a drive: then get the pwd of that drive
    if (fileName.length() >= 2 &&
        fileName.at(0).isLetter() && fileName.at(1) == ':') {
        int drv = fileName.toUpper().at(0).toLatin1() - 'A' + 1;
        if (_getdrive() != drv) {
            QT_WA({
                TCHAR buf[PATH_MAX];
                ::_wgetdcwd(drv, buf, PATH_MAX);
                ret.setUtf16((ushort*)buf, uint(::wcslen(buf)));
            }, {
                char buf[PATH_MAX];
                ::_getdcwd(drv, buf, PATH_MAX);
                ret = buf;
            });
        }
    }
    if (ret.isEmpty()) {
	//just the pwd
	QT_WA({
            DWORD size = 0;
	    WCHAR currentName[PATH_MAX];
            size = ::GetCurrentDirectoryW(PATH_MAX, currentName);
	    if (size !=0) {
                if (size > PATH_MAX) {
                    WCHAR * newCurrentName = new WCHAR[size];
                    if (::GetCurrentDirectoryW(PATH_MAX, newCurrentName) != 0)
                        ret = QString::fromUtf16((ushort*)newCurrentName);
                    delete newCurrentName;
                } else {
                    ret = QString::fromUtf16((ushort*)currentName);
                }
	    }
	} , {
	    char currentName[PATH_MAX];
	    if (QT_GETCWD(currentName,PATH_MAX) != 0) {
		ret = QString::fromLocal8Bit(currentName);
	    }
	});
    }
    if (ret.length() >= 2 && ret[1] == ':')
	ret[0] = ret.at(0).toUpper(); // Force uppercase drive letters.
    return QFSFileEnginePrivate::fixToQtSlashes(ret);
}

QString
QFSFileEngine::homePath()
{
    QString ret = QString::fromLocal8Bit(qgetenv("HOME"));
    if(ret.isEmpty() || !QFile::exists(ret)) {
        ret = QString::fromLocal8Bit(qgetenv("USERPROFILE"));
        if(ret.isEmpty() || !QFile::exists(ret)) {
            ret = QString::fromLocal8Bit(qgetenv("HOMEDRIVE")) + QString::fromLocal8Bit(qgetenv("HOMEPATH"));
            if(ret.isEmpty() || !QFile::exists(ret))
                ret = rootPath();
        }
    }
    return QFSFileEnginePrivate::fixToQtSlashes(ret);
}

QString
QFSFileEngine::rootPath()
{
#if defined(Q_FS_FAT)
    QString ret = QString::fromLatin1(qgetenv("SystemDrive"));
    if(ret.isEmpty())
        ret = "c:";
    ret += "/";
#elif defined(Q_OS_OS2EMX)
    char dir[4];
    _abspath(dir, "/", _MAX_PATH);
    QString ret(dir);
#endif
    return ret;
}

QString
QFSFileEngine::tempPath()
{
    QString ret;
    QT_WA({
	wchar_t tempPath[MAX_PATH];
	GetTempPathW(MAX_PATH, tempPath);
	ret = QString::fromUtf16((ushort*)tempPath);
    } , {
	char tempPath[MAX_PATH];
	GetTempPathA(MAX_PATH, tempPath);
	ret = QString(tempPath);
    });
    if(ret.isEmpty())
        ret = QString::fromLatin1("c:/tmp");
    return ret;
}

QFileInfoList
QFSFileEngine::drives()
{
    QFileInfoList ret;

#if defined(Q_OS_WIN32)
    quint32 driveBits = (quint32) GetLogicalDrives() & 0x3ffffff;
#elif defined(Q_OS_OS2EMX)
    quint32 driveBits, cur;
    if(DosQueryCurrentDisk(&cur,&driveBits) != NO_ERROR)
	exit(1);
    driveBits &= 0x3ffffff;
#endif
    char driveName[4];

#ifndef Q_OS_TEMP
    qstrcpy(driveName, "A:/");
#else
    qstrcpy(driveName, "/");
#endif
    while(driveBits) {
	if(driveBits & 1)
	    ret.append(QString::fromLatin1(driveName).toUpper());
	driveName[0]++;
	driveBits = driveBits >> 1;
    }
    return ret;
}

bool
QFSFileEnginePrivate::doStat() const
{
    if(!tried_stat) {
	tried_stat = true;
	could_stat = false;

        if(file.isEmpty())
            return could_stat;

	UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

        // Stat on windows doesn't accept drivename : without \ so append \ it if this is the case
        QString statName = file;
        if(statName.length() == 2 && statName.at(1) == ':')
            statName += '\\';
	else if(statName.at(statName.length()-1) == '\\')
	    statName = statName.left(statName.length()-2);
        if(d->fd != -1) {
            could_stat = (QT_FSTAT(d->fd, &st) != -1);
        } else {
            QT_WA({
                could_stat = (QT_TSTAT((TCHAR*)statName.utf16(), (QT_STATBUF4TSTAT*)&st) != -1);
            } , {
                could_stat = (QT_STAT(QFSFileEnginePrivate::win95Name(statName), &st) != -1);
            });
        }
        if(could_stat) {
            bool is_dir=false;
            if (file.length() >= 2
                && (file.at(0) == '/' && file.at(1) == '/'
                || file.at(0) == '\\' && file.at(1) == '\\'))
            {
                // UNC - stat doesn't work for all cases (Windows bug)
                int s = file.indexOf(file.at(0),2);
                if (s > 0) {
                    // "\\server\..."
                    s = file.indexOf(file.at(0),s+1);
                    if (s > 0) {
                        // "\\server\share\..."
                        if (s == file.size() - 1) {
                            // "\\server\share\"
                            is_dir=true;
                        } else {
                            // "\\server\share\notfound" 
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
                could_stat = true;;
            }
        }
        SetErrorMode(oldmode);
    }
    return could_stat;
}

QString
QFSFileEnginePrivate::getLink() const
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

        if(hres == CO_E_NOTINITIALIZED) { // COM was not initalized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLink, (LPVOID *)&psl);
        }
        if(SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if(SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)d->file.utf16(), STGM_READ);
                if(SUCCEEDED(hres)) {        // Resolve the link.

                    hres = psl->Resolve(0, SLR_ANY_MATCH);

                    if(SUCCEEDED(hres)) {
                        memcpy(szGotPath, (TCHAR*)d->file.utf16(), (d->file.length()+1)*sizeof(QChar));
                        hres = psl->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY);
                        ret = QString::fromUtf16((ushort*)szGotPath);
                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        if(neededCoInit)
            CoUninitialize();
    } , {
        bool neededCoInit = false;
        IShellLinkA *psl;                            // pointer to IShellLink i/f
        HRESULT hres;
        WIN32_FIND_DATAA wfd;
        char szGotPath[MAX_PATH];
        // Get pointer to the IShellLink interface.

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IShellLinkA, (LPVOID *)&psl);

        if(hres == CO_E_NOTINITIALIZED) { // COM was not initalized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLinkA, (LPVOID *)&psl);
        }
        if(SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if(SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)d->file.utf16(), STGM_READ);
                if(SUCCEEDED(hres)) {        // Resolve the link.

                    hres = psl->Resolve(0, SLR_ANY_MATCH);

                    if(SUCCEEDED(hres)) {
                        QByteArray lfn = d->file.toLocal8Bit();
                        memcpy(szGotPath, lfn.data(), (lfn.length()+1)*sizeof(char));
                        hres = psl->GetPath((char*)szGotPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY);
                        ret = QString::fromLocal8Bit(szGotPath);

                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        if(neededCoInit)
            CoUninitialize();
    });
    return ret;
#else
    return QString();
#endif // QT_NO_COMPONENT
}

bool QFSFileEngine::link(const QString &newName)
{
#if !defined(QT_NO_COMPONENT)
    bool ret = false;

    QString linkName = newName;
    //### assume that they add .lnk

    QT_WA({
        HRESULT hres;
        IShellLink *psl;
        bool neededCoInit = false;

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        if(hres == CO_E_NOTINITIALIZED) { // COM was not initalized
                neededCoInit = true;
                CoInitialize(NULL);
                hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        }
        if (SUCCEEDED(hres)) {
            hres = psl->SetPath((TCHAR*)fileName(AbsoluteName).utf16());
            if (SUCCEEDED(hres)) {
                IPersistFile *ppf;
                hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
                if (SUCCEEDED(hres)) {
                    hres = ppf->Save((TCHAR*)linkName.utf16(), TRUE);
                    if (SUCCEEDED(hres))
                         ret = true;
                    ppf->Release();
                }
            }
            psl->Release();
        }
        if(neededCoInit)
                CoUninitialize();
    } , {
        HRESULT hres;
        IShellLinkA *psl;
        bool neededCoInit = false;

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        if(hres == CO_E_NOTINITIALIZED) { // COM was not initalized
	    neededCoInit = true;
	    CoInitialize(NULL);
	    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        }
        if (SUCCEEDED(hres)) {
            hres = psl->SetPath((char *)QString::fromLocal8Bit(QFSFileEnginePrivate::win95Name(fileName(AbsoluteName))).utf16());
            if (SUCCEEDED(hres)) {
		IPersistFile *ppf;
                hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
                if (SUCCEEDED(hres)) {
                    hres = ppf->Save((LPCOLESTR)linkName.utf16(), TRUE);
                    if (SUCCEEDED(hres))
                         ret = true;
		    ppf->Release();
                }
            }
            psl->Release();
        }
        if(neededCoInit)
	    CoUninitialize();
    });
    return ret;
#else
    Q_UNUSED(newName);
    return false;
#endif // QT_NO_COMPONENT
}

QFileEngine::FileFlags
QFSFileEnginePrivate::getPermissions() const
{
    QFileEngine::FileFlags ret = 0;

#if !defined(QT_NO_COMPONENT)
    if((qt_ntfs_permission_lookup > 0) && ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_NT)) {
	PSID pOwner = 0;
	PSID pGroup = 0;
	PACL pDacl;
        PSECURITY_DESCRIPTOR pSD;
        ACCESS_MASK access_mask;

        enum { ReadMask = 0x00000001, WriteMask = 0x00000002, ExecMask = 0x00000020 };
        resolveLibs();
        if(ptrGetNamedSecurityInfoW && ptrAllocateAndInitializeSid && ptrBuildTrusteeWithSidW && ptrGetEffectiveRightsFromAclW && ptrFreeSid) {
            DWORD res = ptrGetNamedSecurityInfoW((wchar_t*)file.utf16(), SE_FILE_OBJECT,
						 OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
						 &pOwner, &pGroup, &pDacl, 0, &pSD);

            if(res == ERROR_SUCCESS) {
                TRUSTEE_W trustee;
                { //user
                    if(ptrGetEffectiveRightsFromAclW(pDacl, &currentUserTrusteeW, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QFileEngine::ReadUserPerm;
		    if(access_mask & WriteMask)
			ret |= QFileEngine::WriteUserPerm;
		    if(access_mask & ExecMask)
			ret |= QFileEngine::ExeUserPerm;
                }
                { //owner
                    ptrBuildTrusteeWithSidW(&trustee, pOwner);
                    if(ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QFileEngine::ReadOwnerPerm;
		    if(access_mask & WriteMask)
			ret |= QFileEngine::WriteOwnerPerm;
		    if(access_mask & ExecMask)
			ret |= QFileEngine::ExeOwnerPerm;
                }
                { //group
                    ptrBuildTrusteeWithSidW(&trustee, pGroup);
                    if(ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QFileEngine::ReadGroupPerm;
		    if(access_mask & WriteMask)
			ret |= QFileEngine::WriteGroupPerm;
		    if(access_mask & ExecMask)
			ret |= QFileEngine::ExeGroupPerm;
                }
                { //other (world)
                    // Create SID for Everyone (World)
                    SID_IDENTIFIER_AUTHORITY worldAuth = { SECURITY_WORLD_SID_AUTHORITY };
                    PSID pWorld = 0;
                    if(ptrAllocateAndInitializeSid(&worldAuth, 1, SECURITY_WORLD_RID, 0,0,0,0,0,0,0, &pWorld)) {
                        ptrBuildTrusteeWithSidW(&trustee, pWorld);
                        if(ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                            access_mask = (ACCESS_MASK)-1; // ###
			if(access_mask & ReadMask)
			    ret |= QFileEngine::ReadOtherPerm;
			if(access_mask & WriteMask)
			    ret |= QFileEngine::WriteOtherPerm;
			if(access_mask & ExecMask)
			    ret |= QFileEngine::ExeOtherPerm;
                    }
                    ptrFreeSid(pWorld);
                }
                LocalFree(pSD);
            }
        }
    } else
#endif
           {
	//### what to do with permissions if we don't use ntfs or are not on a NT system
	// for now just add all permissions and what about exe missions ??
	// also qt_ntfs_permission_lookup is now not set by defualt ... should it ?
    	ret |= QFileEngine::ReadOtherPerm | QFileEngine::ReadGroupPerm
	    | QFileEngine::ReadOwnerPerm | QFileEngine::ReadUserPerm
	    | QFileEngine::WriteUserPerm | QFileEngine::WriteOwnerPerm
	    | QFileEngine::WriteGroupPerm | QFileEngine::WriteOtherPerm
	    | QFileEngine::ExeUserPerm | QFileEngine::ExeOwnerPerm
	    | QFileEngine::ExeGroupPerm | QFileEngine::ExeOtherPerm;
    }

    if(ret & (QFileEngine::WriteOwnerPerm | QFileEngine::WriteUserPerm |
	      QFileEngine::WriteGroupPerm | QFileEngine::WriteOtherPerm)) {
	QT_WA({
	    DWORD attr = GetFileAttributes((TCHAR*)file.utf16());
	    if(attr & FILE_ATTRIBUTE_READONLY)
	        ret &= ~(QFileEngine::WriteOwnerPerm | QFileEngine::WriteUserPerm |
		       QFileEngine::WriteGroupPerm | QFileEngine::WriteOtherPerm);
	} , {
	    DWORD attr = GetFileAttributesA(file.toLocal8Bit());
	    if(attr & FILE_ATTRIBUTE_READONLY)
		ret &= ~(QFileEngine::WriteOwnerPerm | QFileEngine::WriteUserPerm |
			 QFileEngine::WriteGroupPerm | QFileEngine::WriteOtherPerm);
	});
    }

    return ret;
}

QFileEngine::FileFlags
QFSFileEngine::fileFlags(QFileEngine::FileFlags type) const
{
    QFileEngine::FileFlags ret = 0;
    if(type & PermsMask) {
	ret |= d->getPermissions();
        // ### Workaround pascals ### above. Since we always set all properties to true
        // we need to disable read and exec access if the file does not exists
        if (d->doStat())
            ret |= ExistsFlag;
        else
            ret &= 0x2222;
    }
    if(type & TypesMask) {
	if(d->doStat()) {
	    if(d->file.endsWith(".lnk"))
		ret |= LinkType;
	    else if((d->st.st_mode & S_IFMT) == S_IFREG)
		ret |= FileType;
	    else if((d->st.st_mode & S_IFMT) == S_IFDIR)
		ret |= DirectoryType;
	}
    }
    if(type & FlagsMask) {
	if(d->doStat()) {
	    ret |= QFileEngine::FileFlags(ExistsFlag | LocalDiskFlag);
	    if(fileName(BaseName)[0] == QChar('.')) {
		QT_WA({
		    if(GetFileAttributesW((TCHAR*)d->file.utf16()) & FILE_ATTRIBUTE_HIDDEN)
			ret |= HiddenFlag;
		} , {
		    if(GetFileAttributesA(d->file.toLocal8Bit()) & FILE_ATTRIBUTE_HIDDEN)
			ret |= HiddenFlag;
		});
            }
            if(d->file == "/" || d->file == "//" ||
               (d->file[0].isLetter() && d->file.mid(1,d->file.length()) == ":/"))
                ret |= RootFlag;
	}
    }
    return ret;
}

QString
QFSFileEngine::fileName(FileName file) const
{
    if(file == BaseName) {
	int slash = d->file.lastIndexOf('/');
	if(slash == -1) {
	    int colon = d->file.lastIndexOf(':');
	    if(colon != -1)
		return d->file.mid(colon + 1);
	    return d->file;
	}
	return d->file.mid(slash + 1);
    } else if(file == PathName) {
        if(!d->file.size())
            return d->file;
	int slash = d->file.lastIndexOf('/');
	if(slash == -1) {
	    if(d->file.at(1) == ':')
		return d->file.left(2);
	    return QString::fromLatin1(".");
	} else {
	    if(!slash)
		return QString::fromLatin1("/");
	    if(slash == 2 && d->file.at(1) == ':')
		slash++;
	    return d->file.left(slash);
	}
    } else if(file == AbsoluteName || file == AbsolutePathName) {
        QString ret;
        if (!isRelativePath())
            ret = d->file;
        else
            ret = QDir::cleanPath(QDir::currentPath() + QLatin1Char('/') + d->file);

        // The path should be absolute at this point.
        // From the docs :
        // Absolute paths begin with the directory separator "/"
        // (optionally preceded by a drive specification under Windows).
        if (ret.at(0) != QLatin1Char('/')) {
            Q_ASSERT(ret.length() >= 2);
            Q_ASSERT(ret.at(0).isLetter());
            Q_ASSERT(ret.at(1) == QLatin1Char(':'));

            // Force uppercase drive letters.
            ret[0] = ret.at(0).toUpper();
        }

        if (file == AbsolutePathName) {
            int slash = ret.lastIndexOf(QLatin1Char('/'));
            Q_ASSERT(slash < 0 || slash >= 2);
            if (slash < 0)
                return ret;
            else
                return ret.left(slash);
        }
        return ret;
    } else if(file == CanonicalName || file == CanonicalPathName) {
        QString ret;
        if (!(fileFlags(ExistsFlag) & ExistsFlag))
            return ret;
        QString abs;
        if (fileFlags(DirectoryType) & DirectoryType)
            abs = fileName(AbsoluteName);
        else
            abs = fileName(AbsolutePathName);
        QT_WA({
            TCHAR cur[PATH_MAX];
            ::_wgetcwd(cur, PATH_MAX);
            if (::_wchdir((TCHAR*)abs.utf16()) >= 0) {
                TCHAR real[PATH_MAX];
                if(::_wgetcwd(real, PATH_MAX))
                    ret = QString::fromUtf16((ushort*)real);
            }
            ::_wchdir(cur);
        } , {
            char cur[PATH_MAX];
            QT_GETCWD(cur, PATH_MAX);
            if(QT_CHDIR(QFSFileEnginePrivate::win95Name(abs)) >= 0) {
                char real[PATH_MAX];
                if(QT_GETCWD(real, PATH_MAX) != 0)
                    ret = QString::fromLocal8Bit(real);
            }
            QT_CHDIR(cur);
        });
        if (file == CanonicalName && !(fileFlags(DirectoryType) & DirectoryType))
            ret += QLatin1Char('/') + fileName(BaseName);
        // Force uppercase drive letters.
        ret[0] = ret.at(0).toUpper();
        return QFSFileEnginePrivate::fixToQtSlashes(ret);
    } else if(file == LinkName) {
	return QFSFileEnginePrivate::fixToQtSlashes(d->getLink());
    }
    return d->file;
}

bool
QFSFileEngine::isRelativePath() const
{
    return !(d->file.startsWith("/")
        || (d->file.length() >= 2
        && ((d->file.at(0).isLetter() && d->file.at(1) == ':')
        || (d->file.at(0) == '/' && d->file.at(1) == '/'))));                // drive, e.g. a:
}

uint
QFSFileEngine::ownerId(FileOwner /*own*/) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

QString
QFSFileEngine::owner(FileOwner own) const
{
#if !defined(QT_NO_COMPONENT)
    if((qt_ntfs_permission_lookup > 0) && ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_NT)) {
	PSID pOwner = 0;
	PSECURITY_DESCRIPTOR pSD;
	QString name;
	resolveLibs();

	if(ptrGetNamedSecurityInfoW && ptrLookupAccountSidW) {
	    if(ptrGetNamedSecurityInfoW((wchar_t*)d->file.utf16(), SE_FILE_OBJECT,
					 own == OwnerGroup ? GROUP_SECURITY_INFORMATION : OWNER_SECURITY_INFORMATION,
					 NULL, &pOwner, NULL, NULL, &pSD) == ERROR_SUCCESS) {
		DWORD lowner = 0, ldomain = 0;
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0').
		ptrLookupAccountSidW(NULL, pOwner, NULL, &lowner, NULL, &ldomain, (SID_NAME_USE*)&use);
		wchar_t *owner = new wchar_t[lowner];
		wchar_t *domain = new wchar_t[ldomain];
		// Second call, size is without '\0'
		if(ptrLookupAccountSidW(NULL, pOwner, (LPWSTR)owner, &lowner,
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
#else
    Q_UNUSED(own);
#endif
    return QString("");
}

bool QFSFileEngine::chmod(uint perms)
{
    bool ret = false;
    int mode = 0;

    if (perms & QFile::ReadOwner || perms & QFile::ReadUser || perms & QFile::ReadGroup || perms & QFile::ReadOther)
        mode |= _S_IREAD;
    if (perms & QFile::WriteOwner || perms & QFile::WriteUser || perms & QFile::WriteGroup || perms & QFile::WriteOther)
        mode |= _S_IWRITE;

    if (mode == 0) // not supported
        return false;

   QT_WA({
        ret = ::_wchmod((TCHAR*)d->file.utf16(), mode) == 0;
   } , {
        ret = ::_chmod(d->file.toLocal8Bit(), mode) == 0;
   });
   return ret;
}

bool QFSFileEngine::setSize(qint64 size)
{
    if (d->fd != -1) {
        // resize open file
        HANDLE fh = (HANDLE)_get_osfhandle(d->fd);
        if (fh == INVALID_HANDLE_VALUE)
            return false;

        qint64 currentPos = at();
        if (seek(size) && SetEndOfFile(fh)) {
            seek(qMin(currentPos, size));
            return true;
        }
        seek(currentPos);
    } else {
        // resize file on disk
        QFile file1(QFile::encodeName(d->file));
        if (file1.open(QFile::ReadOnly)) {
            QTemporaryFile file2;
            if (file2.open()) {
                qint64 newSize = 0;
                char block[1024];
                while (!file1.atEnd() && newSize < size) {
                    qint64 in = file1.read(block, 1024);
                    if (in == -1)
                        break;
                    in = qMin(size - newSize, in);
                    if (in != file2.write(block, in)) {
                        return false;
                    }
                    newSize += in;
                }
                if (newSize < size) {
                    QByteArray ba(size - newSize, char(0));
                    if (file2.write(ba) != ba.size())
                        return false;
                }
                file1.close();
                file2.close();
                if (file2.rename(file1.fileName()))
                    return true;
            }
        }
    }
    return false;
}


