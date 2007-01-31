/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define _POSIX_
#include "qplatformdefs.h"
#include "qabstractfileengine.h"
#include "private/qfsfileengine_p.h"
#include <qdebug.h>

#include "qfile.h"
#include "qdir.h"
#include "qtemporaryfile.h"
#ifndef QT_NO_REGEXP
# include "qregexp.h"
#endif
#include "private/qmutexpool_p.h"
#include "qvarlengtharray.h"
#include "qdatetime.h"
#include "qt_windows.h"

#include <sys/types.h>
#include <direct.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <accctrl.h>
#include <ctype.h>
#include <limits.h>
#define SECURITY_WIN32
#include <security.h>

#ifndef INVALID_FILE_ATTRIBUTES
#  define INVALID_FILE_ATTRIBUTES (DWORD (-1))
#endif

static QString readLink(const QString &link);

Q_CORE_EXPORT int qt_ntfs_permission_lookup = 0;

#if !defined(QT_NO_LIBRARY)
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

typedef BOOL (WINAPI *PtrOpenProcessToken)(HANDLE, DWORD, PHANDLE );
static PtrOpenProcessToken ptrOpenProcessToken = 0;
typedef BOOL (WINAPI *PtrGetUserProfileDirectoryW)( HANDLE, LPWSTR, LPDWORD);
static PtrGetUserProfileDirectoryW ptrGetUserProfileDirectoryW = 0;
typedef BOOL (WINAPI *PtrSetFilePointerEx)(HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD);
static PtrSetFilePointerEx ptrSetFilePointerEx = 0;


void QFSFileEnginePrivate::resolveLibs()
{
    static bool triedResolve = false;
    if(!triedResolve) {
        // need to resolve the security info functions

        // protect initialization
#ifndef QT_NO_THREAD
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
            ptrOpenProcessToken = (PtrOpenProcessToken)GetProcAddress(advapiHnd, "OpenProcessToken");
	        HINSTANCE userenvHnd = LoadLibraryW(L"userenv");
            if (userenvHnd) {
                ptrGetUserProfileDirectoryW = (PtrGetUserProfileDirectoryW)GetProcAddress(userenvHnd, "GetUserProfileDirectoryW");
            }
            HINSTANCE kernelHnd = LoadLibraryW(L"kernel32");
            if (kernelHnd)
                ptrSetFilePointerEx = (PtrSetFilePointerEx)GetProcAddress(kernelHnd, "SetFilePointerEx");
        }
    }
}
#endif // QT_NO_LIBRARY

// UNC functions NT
typedef DWORD (WINAPI *PtrNetShareEnum_NT)(LPWSTR, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD, LPDWORD);
static PtrNetShareEnum_NT ptrNetShareEnum_NT = 0;
typedef DWORD (WINAPI *PtrNetApiBufferFree_NT)(LPVOID);
static PtrNetApiBufferFree_NT ptrNetApiBufferFree_NT = 0;
typedef struct _SHARE_INFO_1_NT {
    LPWSTR shi1_netname;
    DWORD shi1_type;
    LPWSTR shi1_remark;
} SHARE_INFO_1_NT;


bool QFSFileEnginePrivate::resolveUNCLibs_NT()
{
    static bool triedResolve = false;
    if (!triedResolve) {
#ifndef QT_NO_THREAD
        QMutexLocker locker(qt_global_mutexpool ?
            qt_global_mutexpool->get(&triedResolve) : 0);
        if (triedResolve) {
            return ptrNetShareEnum_NT && ptrNetApiBufferFree_NT;
        }
#endif
        triedResolve = true;
        HINSTANCE hLib = LoadLibraryW(L"Netapi32");
        if (hLib) {
            ptrNetShareEnum_NT = (PtrNetShareEnum_NT)GetProcAddress(hLib, "NetShareEnum");
            if (ptrNetShareEnum_NT)
                ptrNetApiBufferFree_NT = (PtrNetApiBufferFree_NT)GetProcAddress(hLib, "NetApiBufferFree");
        }
    }
    return ptrNetShareEnum_NT && ptrNetApiBufferFree_NT;
}

// UNC functions 9x
typedef DWORD (WINAPI *PtrNetShareEnum_9x)(const char FAR *, short, char FAR *, unsigned short, unsigned short FAR *, unsigned short FAR *);
static PtrNetShareEnum_9x ptrNetShareEnum_9x = 0;
#ifdef LM20_NNLEN
# define LM20_NNLEN_9x LM20_NNLEN
#else
# define LM20_NNLEN_9x 12
#endif
typedef struct _SHARE_INFO_1_9x {
  char shi1_netname[LM20_NNLEN_9x+1];
  char shi1_pad1;
  unsigned short shi1_type;
  char FAR* shi1_remark;
} SHARE_INFO_1_9x;

bool QFSFileEnginePrivate::resolveUNCLibs_9x()
{
    static bool triedResolve = false;
    if (!triedResolve) {
#ifndef QT_NO_THREAD
        QMutexLocker locker(qt_global_mutexpool ?
            qt_global_mutexpool->get(&triedResolve) : 0);
        if (triedResolve) {
            return ptrNetShareEnum_9x;
        }
#endif
        triedResolve = true;
        HINSTANCE hLib = LoadLibraryA("Svrapi");
        if (hLib)
            ptrNetShareEnum_9x = (PtrNetShareEnum_9x)GetProcAddress(hLib, "NetShareEnum");
    }
    return ptrNetShareEnum_9x;
}

bool QFSFileEnginePrivate::uncListSharesOnServer(const QString &server, QStringList *list)
{
    if (resolveUNCLibs_NT()) {
        SHARE_INFO_1_NT *BufPtr, *p;
        DWORD res;
        DWORD er=0,tr=0,resume=0, i;
        do {
            res = ptrNetShareEnum_NT((wchar_t*)server.utf16(), 1, (LPBYTE *)&BufPtr, DWORD(-1), &er, &tr, &resume);
            if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA) {
                p=BufPtr;
                for (i = 1; i <= er; ++i) {
                    if (list && p->shi1_type == 0)
                        list->append(QString::fromUtf16((unsigned short *)p->shi1_netname));
                    p++;
                }
            }
            ptrNetApiBufferFree_NT(BufPtr);
        } while (res==ERROR_MORE_DATA);
        return res == ERROR_SUCCESS;

    } else if (resolveUNCLibs_9x()) {
        SHARE_INFO_1_9x *pBuf = 0;
        short cbBuffer;
        unsigned short nEntriesRead = 0;
        unsigned short nTotalEntries = 0;
        short numBuffs = 20;
        DWORD nStatus = 0;
        do {
            cbBuffer = numBuffs * sizeof(SHARE_INFO_1_9x);
            pBuf = (SHARE_INFO_1_9x *)malloc(cbBuffer);
            if (pBuf) {
                nStatus = ptrNetShareEnum_9x(server.toLocal8Bit().constData(), 1, (char FAR *)pBuf, cbBuffer, &nEntriesRead, &nTotalEntries);
                if ((nStatus == ERROR_SUCCESS)) {
                    for (int i = 0; i < nEntriesRead; ++i) {
                        if (list && pBuf[i].shi1_type == 0)
                            list->append(QString::fromLocal8Bit(pBuf[i].shi1_netname));
                    }
                    free(pBuf);
                    break;
                }
                free(pBuf);
                numBuffs *=2;
            }
        } while (nStatus == ERROR_MORE_DATA);
        return nStatus == ERROR_SUCCESS;
    }
    return false;
}

static bool isUncRoot(const QString &server)
{
    QString localPath = QDir::toNativeSeparators(server);
    QStringList parts = localPath.split(QLatin1Char('\\'), QString::SkipEmptyParts);
    return localPath.startsWith(QLatin1String("\\\\")) && parts.count() <= 1;
}

static bool isUncPath(const QString &path)
{
    // Starts with // or \\, but not \\. or //.
    return (path.startsWith(QLatin1String("//"))
            || path.startsWith(QLatin1String("\\\\")))
        && (path.size() > 2 && path.at(2) != QLatin1Char('.'));
}

static bool isRelativePath(const QString &path)
{
    return !(path.startsWith(QLatin1Char('/'))
           || (path.length() >= 2
           && ((path.at(0).isLetter() && path.at(1) == QLatin1Char(':'))
           || (path.at(0) == QLatin1Char('/') && path.at(1) == QLatin1Char('/'))))); // drive, e.g. a:
}

static QString fixIfRelativeUncPath(const QString &path)
{
    if (isRelativePath(path)) {
        QString currentPath = QDir::currentPath() + QLatin1Char('/');
        if (currentPath.startsWith(QLatin1String("//")))
            return QString(path).prepend(currentPath);
    }
    return path;
}

// can be //server or //server/share
static bool uncShareExists(const QString &server)
{
    QStringList parts = server.split(QLatin1Char('\\'), QString::SkipEmptyParts);
    if (parts.count()) {
        QStringList shares;
        if (QFSFileEnginePrivate::uncListSharesOnServer(QLatin1String("\\\\") + parts.at(0), &shares)) {
            if (parts.count() >= 2)
                return shares.contains(parts.at(1), Qt::CaseInsensitive);
            else
                return true;
        }
    }
    return false;
}

// If you change this function, remember to also change the UNICODE version
static QString nativeAbsoluteFilePathA(const QString &path)
{
    QString ret;
    QVarLengthArray<char, MAX_PATH> buf(MAX_PATH);
    char *fileName = 0;
    QByteArray ba = path.toLocal8Bit();
    DWORD retLen = GetFullPathNameA(ba.constData(), buf.size(), buf.data(), &fileName);
    if (retLen > (DWORD)buf.size()) {
        buf.resize(retLen);
        retLen = GetFullPathNameA(ba.constData(), buf.size(), buf.data(), &fileName);
    }
    if (retLen != 0)
        ret = QString::fromLocal8Bit(buf.data(), retLen);
    return ret;
}

// If you change this function, remember to also change the NON-UNICODE version
static QString nativeAbsoluteFilePathW(const QString &path)
{
    QString ret;
    QVarLengthArray<wchar_t, MAX_PATH> buf(MAX_PATH);
    wchar_t *fileName = 0;
    DWORD retLen = GetFullPathNameW((wchar_t*)path.utf16(), buf.size(), buf.data(), &fileName);
    if (retLen > (DWORD)buf.size()) {
        buf.resize(retLen);
        retLen = GetFullPathNameW((wchar_t*)path.utf16(), buf.size(), buf.data(), &fileName);
    }
    if (retLen != 0)
        ret = QString::fromUtf16((unsigned short *)buf.data(), retLen);
    return ret;
}

static QString nativeAbsoluteFilePath(const QString &path)
{
    QString absPath = QT_WA_INLINE(nativeAbsoluteFilePathW(path), nativeAbsoluteFilePathA(path));
    // This is really ugly, but GetFullPathName strips off whitespace at the end.
    // If you for instance write ". " in the lineedit of QFileDialog,
    // (which is an invalid filename) this function will strip the space off and viola,
    // the file is later reported as existing. Therefore, we re-add the whitespace that
    // was at the end of path in order to keep the filename invalid.
    int i = path.size() - 1;
    while (i >= 0 && path.at(i) == QLatin1Char(' ')) --i;
    int extraws = path.size() - 1 - i;
    if (extraws >= 0) {
        while (extraws) {
            absPath.append(QLatin1Char(' '));
            --extraws;
        }
    }
    return absPath;
}

QByteArray QFSFileEnginePrivate::win95Name(const QString &path)
{
    QString ret(path);
    if(path.length() > 1 && path[0] == QLatin1Char('/') && path[1] == QLatin1Char('/')) {
        // Win95 cannot handle slash-slash needs slosh-slosh.
        ret[0] = QLatin1Char('\\');
        ret[1] = QLatin1Char('\\');
        int n = ret.indexOf(QLatin1Char('/'));
        if(n >= 0)
            ret[n] = QLatin1Char('\\');
    } else if(path.length() > 3 && path[2] == QLatin1Char('/') && path[3] == QLatin1Char('/')) {
        ret[2] = QLatin1Char('\\');
        ret.remove(3, 1);
        int n = ret.indexOf(QLatin1Char('/'));
        if(n >= 0)
            ret[n] = QLatin1Char('\\');
    }
    return ret.toLocal8Bit();
}

/*!
    \internal
*/
QString QFSFileEnginePrivate::longFileName(const QString &path)
{
    if (path.startsWith(QLatin1String("\\\\.\\")))
        return path;

    QString absPath = nativeAbsoluteFilePath(path);
    QString prefix = QLatin1String("\\\\?\\");
    if (isUncPath(path)) {
        prefix = QLatin1String("\\\\?\\UNC\\");
        absPath.remove(0, 2);
    }
    return prefix + absPath;
}

/*!
    \internal
*/
void QFSFileEnginePrivate::nativeInitFileName()
{
    QT_WA({
        QString path = longFileName(QDir::toNativeSeparators(fixIfRelativeUncPath(filePath)));
        nativeFilePath = QByteArray((const char *)path.utf16(), path.size() * 2 + 1);
    }, {
        QString path = fixIfRelativeUncPath(filePath);
        nativeFilePath = win95Name(QDir::toNativeSeparators(path));
    });
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeOpen(QIODevice::OpenMode openMode)
{
    Q_Q(QFSFileEngine);

    // All files are opened in share mode (both read and write).
    DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

    // All files on Windows can be read; there's no such thing as an
    // unreadable file. Add GENERIC_WRITE if WriteOnly is passed.
    int accessRights = GENERIC_READ;
    if (openMode & QIODevice::WriteOnly)
        accessRights |= GENERIC_WRITE;

    SECURITY_ATTRIBUTES securityAtts = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // Regular file mode. In Unbuffered mode, pass the no-buffering flag.
    DWORD flagsAndAtts = FILE_ATTRIBUTE_NORMAL;
    if (openMode & QIODevice::Unbuffered)
        flagsAndAtts |= FILE_FLAG_NO_BUFFERING;

    // WriteOnly can create files, ReadOnly cannot.
    DWORD creationDisp = (openMode & QIODevice::WriteOnly)
                         ? OPEN_ALWAYS : OPEN_EXISTING;

    // Create the file handle.
    QT_WA({
        fileHandle = CreateFileW((TCHAR *)nativeFilePath.constData(),
                                 accessRights,
                                 shareMode,
                                 &securityAtts,
                                 creationDisp,
                                 flagsAndAtts,
                                 NULL);
    }, {
        fileHandle = CreateFileA(nativeFilePath.constData(),
                                 accessRights,
                                 shareMode,
                                 &securityAtts,
                                 creationDisp,
                                 flagsAndAtts,
                                 NULL);
    });

    // Bail out on error.
    if (fileHandle == INVALID_HANDLE_VALUE) {
        q->setError(QFile::OpenError, qt_error_string());
        return false;
    }

    // Truncate the file after successfully opening it if Truncate is passed.
    if (openMode & QIODevice::Truncate)
        q->setSize(0);

    return true;
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeClose()
{
    Q_Q(QFSFileEngine);
    if (fh || fd != -1) {
        // stdlib / stdio mode.
        return closeFdFh();
    }

    // Windows native mode.
    if (fileHandle == INVALID_HANDLE_VALUE || !CloseHandle(fileHandle)) {
        q->setError(QFile::UnspecifiedError, qt_error_string());
        return false;
    }
    fileHandle = INVALID_HANDLE_VALUE;
    return true;
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeFlush()
{
    Q_Q(QFSFileEngine);
    if (fh) {
        // Buffered stdlib mode.
        return flushFh();
    }
    if (fd != -1) {
        // Unbuffered stdio mode; always succeeds (no buffer).
        return true;
    }

    // Windows native mode; flushing is
    // unnecessary. FlushFileBuffers(), the equivalent of sync() or
    // fsync() on Unix, does a low-level flush to the disk, and we
    // don't expose an API for this.
    return true;
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativeSize() const
{
    Q_Q(const QFSFileEngine);
    QFSFileEngine *thatQ = const_cast<QFSFileEngine *>(q);

    // ### Don't flush; for buffered files, we should get away with ftell.
    thatQ->flush();

    // Buffered stdlib mode.
    if (fh) {
        // ### use _ftelli64?
        long oldPos = ftell(fh);
        fseek(fh, 0, SEEK_END);
        long fileSize = ftell(fh);
        fseek(fh, oldPos, SEEK_SET);
        return fileSize;
    }

    // Not-open mode, where the file name is known: We'll check the
    // file system directly.
    if (openMode == QIODevice::NotOpen && !nativeFilePath.isEmpty()) {
        bool ok = false;
        WIN32_FILE_ATTRIBUTE_DATA attribData;
        QT_WA({
            ok = ::GetFileAttributesExW((TCHAR *)nativeFilePath.constData(),
                                        GetFileExInfoStandard, &attribData);
        } , {
            ok = ::GetFileAttributesExA(nativeFilePath.constData(),
                                        GetFileExInfoStandard, &attribData);
        });
        if (ok) {
            LARGE_INTEGER lInt;
            lInt.LowPart = attribData.nFileSizeLow;
            lInt.HighPart = attribData.nFileSizeHigh;
            return lInt.QuadPart;
        }
        thatQ->setError(QFile::UnspecifiedError, qt_error_string());
        return 0;
    }

    // Unbuffed stdio mode.
    if(fd != -1) {
        HANDLE handle = (HANDLE)_get_osfhandle(fd);
        if (handle != INVALID_HANDLE_VALUE) {
            BY_HANDLE_FILE_INFORMATION fileInfo;
            if (GetFileInformationByHandle(handle, &fileInfo)) {
                LARGE_INTEGER lInt;
                lInt.LowPart = fileInfo.nFileSizeLow;
                lInt.HighPart = fileInfo.nFileSizeHigh;
                return lInt.QuadPart;
            }
        }
        thatQ->setError(QFile::UnspecifiedError, qt_error_string());
       return 0;
    }

    // Windows native mode.
    if (fileHandle == INVALID_HANDLE_VALUE)
        return 0;

    BY_HANDLE_FILE_INFORMATION fileInfo;
    if (!GetFileInformationByHandle(fileHandle, &fileInfo)) {
        thatQ->setError(QFile::UnspecifiedError, qt_error_string());
        return 0;
    }

    LARGE_INTEGER lInt;
    lInt.LowPart = fileInfo.nFileSizeLow;
    lInt.HighPart = fileInfo.nFileSizeHigh;
    return qint64(lInt.QuadPart);
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativePos() const
{
    Q_Q(const QFSFileEngine);
    QFSFileEngine *thatQ = const_cast<QFSFileEngine *>(q);

    if (fh || fd != -1) {
        // stdlib / stido mode.
        return posFdFh();
    }

    // Windows native mode.
    if (fileHandle == INVALID_HANDLE_VALUE)
        return 0;

#if !defined(QT_NO_LIBRARY)
    QFSFileEnginePrivate::resolveLibs();
    if (!ptrSetFilePointerEx) {
#endif
        DWORD newFilePointer = SetFilePointer(fileHandle, 0, NULL, FILE_CURRENT);
        if (newFilePointer == 0xFFFFFFFF) {
            thatQ->setError(QFile::UnspecifiedError, qt_error_string());
            return 0;
        }

        // Note: returns <4GB; does not work with large files. This is the
        // case for MOC, UIC, qmake and other bootstrapped tools, and for
        // Win9x/ME.
        return qint64(newFilePointer);
#if !defined(QT_NO_LIBRARY)
    }

    // This approach supports large files.
    LARGE_INTEGER currentFilePos;
    LARGE_INTEGER offset;
    offset.LowPart = 0;
    offset.HighPart = 0;
    if (!ptrSetFilePointerEx(fileHandle, offset, &currentFilePos, FILE_CURRENT)) {
        thatQ->setError(QFile::UnspecifiedError, qt_error_string());
        return 0;
    }

    return qint64(currentFilePos.QuadPart);
#endif
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeSeek(qint64 pos)
{
    Q_Q(const QFSFileEngine);
    QFSFileEngine *thatQ = const_cast<QFSFileEngine *>(q);

    if (fh || fd != -1) {
        // stdlib / stdio mode.
        return seekFdFh(pos);
    }

#if !defined(QT_NO_LIBRARY)
    QFSFileEnginePrivate::resolveLibs();
    if (!ptrSetFilePointerEx) {
#endif
        LONG seekToPos = LONG(pos); // <- lossy
        DWORD newFilePointer = SetFilePointer(fileHandle, seekToPos, NULL, FILE_BEGIN);
        if (newFilePointer == 0xFFFFFFFF) {
            thatQ->setError(QFile::UnspecifiedError, qt_error_string());
            return false;
        }

        // Note: does not work with large files. This is the case for MOC,
        // UIC, qmake and other bootstrapped tools, and for Win9x/ME.
        return true;
#if !defined(QT_NO_LIBRARY)
    }

    // This approach supports large files.
    LARGE_INTEGER currentFilePos;
    LARGE_INTEGER offset;
    offset.LowPart = (unsigned int)(quint64(pos) & Q_UINT64_C(0xffffffff));
    offset.HighPart = (unsigned int)((quint64(pos) >> 32) & Q_UINT64_C(0xffffffff));
    if (ptrSetFilePointerEx(fileHandle, offset, &currentFilePos, FILE_BEGIN) == 0) {
        thatQ->setError(QFile::UnspecifiedError, qt_error_string());
        return false;
    }
    return true;
#endif
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativeRead(char *data, qint64 maxlen)
{
    Q_Q(QFSFileEngine);

    if (fh || fd != -1) {
        // stdio / stdlib mode.
        return readFdFh(data, maxlen);
    }

    // Windows native mode.
    if (fileHandle == INVALID_HANDLE_VALUE)
        return -1;

    DWORD bytesToRead = DWORD(maxlen); // <- lossy

    // Reading on Windows fails with ERROR_NO_SYSTEM_RESOURCES when
    // the chunks are too large, so we limit the block size to 32MB.
    static const DWORD maxBlockSize = 32 * 1024 * 1024;

    qint64 totalRead = 0;
    do {
        DWORD blockSize = qMin<DWORD>(bytesToRead, maxBlockSize);
        DWORD bytesRead;
        if (!ReadFile(fileHandle, data + totalRead, blockSize, &bytesRead, NULL)) {
            if (totalRead == 0) {
                // Note: only return failure if the first ReadFile fails.
                q->setError(QFile::ReadError, qt_error_string());
                return -1;
            }
            break;
        }
        if (bytesRead == 0)
            break;
        totalRead += bytesRead;
        bytesToRead -= bytesRead;
    } while (totalRead < maxlen);
    return qint64(totalRead);
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativeReadLine(char *data, qint64 maxlen)
{
    Q_Q(QFSFileEngine);

    if (fh || fd != -1) {
        // stdio / stdlib mode.
        return readLineFdFh(data, maxlen);
    }

    // Windows native mode.
    if (fileHandle == INVALID_HANDLE_VALUE)
        return -1;

    // ### No equivalent in Win32?
    return q->QAbstractFileEngine::readLine(data, maxlen);
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativeWrite(const char *data, qint64 len)
{
    Q_Q(QFSFileEngine);

    if (fh || fd != -1) {
        // stdio / stdlib mode.
        return writeFdFh(data, len);
    }

    // Windows native mode.
    if (fileHandle == INVALID_HANDLE_VALUE)
        return -1;

    qint64 bytesToWrite = DWORD(len); // <- lossy

    // Writing on Windows fails with ERROR_NO_SYSTEM_RESOURCES when
    // the chunks are too large, so we limit the block size to 32MB.
    static const DWORD maxBlockSize = 32 * 1024 * 1024;

    qint64 totalWritten = 0;
    do {
        DWORD blockSize = qMin<DWORD>(bytesToWrite, maxBlockSize);
        DWORD bytesWritten;
        if (!WriteFile(fileHandle, data + totalWritten, blockSize, &bytesWritten, NULL)) {
            if (totalWritten == 0) {
                // Note: Only return error if the first WriteFile failed.
                q->setError(QFile::WriteError, qt_error_string());
                return -1;
            }
            break;
        }
        if (bytesWritten == 0)
            break;
        totalWritten += bytesWritten;
        bytesToWrite -= bytesWritten;
    } while (totalWritten < len);
    return qint64(totalWritten);
}

/*!
    \internal
*/
int QFSFileEnginePrivate::nativeHandle() const
{
    if (fh || fd != -1)
        return fh ? QT_FILENO(fh) : fd;
#if defined (Q_OS_WIN64)
    // int(fileHandle) is broken
    return -1;
#else
    // int(fileHandle) works
    return int(fileHandle);
#endif
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeIsSequential() const
{
    // stdlib / Windows native mode.
    if (fh || fileHandle != INVALID_HANDLE_VALUE) {
        if (fh == stdin || fh == stdout || fh == stderr)
            return true;

        HANDLE handle = fileHandle;
        if (fileHandle == INVALID_HANDLE_VALUE) {
            // Rare case: using QFile::open(FILE*) to open a pipe.
            handle = (HANDLE)_get_osfhandle(QT_FILENO(fh));
        }

        DWORD fileType = GetFileType(handle);
        return fileType == FILE_TYPE_PIPE;
    }

    // stdio mode.
    if (fd != -1)
        return isSequentialFdFh();
    return false;
}

/*!
    \reimp
*/
bool QFSFileEngine::remove()
{
    Q_D(QFSFileEngine);
    QT_WA({
        return ::DeleteFileW((TCHAR*)QFSFileEnginePrivate::longFileName(d->filePath).utf16()) != 0;
    } , {
        return ::DeleteFileA(QFSFileEnginePrivate::win95Name(d->filePath)) != 0;
    });
}

/*!
    \reimp
*/
bool QFSFileEngine::copy(const QString &copyName)
{
    Q_D(QFSFileEngine);
    QT_WA({
        return ::CopyFileW((TCHAR*)QFSFileEnginePrivate::longFileName(d->filePath).utf16(),
                           (TCHAR*)QFSFileEnginePrivate::longFileName(copyName).utf16(), true) != 0;
    } , {
        return ::CopyFileA(QFSFileEnginePrivate::win95Name(d->filePath),
                           QFSFileEnginePrivate::win95Name(copyName), true) != 0;
    });
}

/*!
    \reimp
*/
bool QFSFileEngine::rename(const QString &newName)
{
    Q_D(QFSFileEngine);
    QT_WA({
        return ::MoveFileW((TCHAR*)QFSFileEnginePrivate::longFileName(d->filePath).utf16(),
                           (TCHAR*)QFSFileEnginePrivate::longFileName(newName).utf16()) != 0;
    } , {
        return ::MoveFileA(QFSFileEnginePrivate::win95Name(d->filePath),
                           QFSFileEnginePrivate::win95Name(newName)) != 0;
    });
}

static inline bool mkDir(const QString &path)
{
    QT_WA({
        return ::CreateDirectoryW((TCHAR*)QFSFileEnginePrivate::longFileName(path).utf16(), 0);
    } , {
        return ::CreateDirectoryA(QFSFileEnginePrivate::win95Name(QFileInfo(path).absoluteFilePath()), 0);
    });
}

/*!
    \reimp
*/
static inline bool rmDir(const QString &path)
{
    QT_WA({
        return ::RemoveDirectoryW((TCHAR*)QFSFileEnginePrivate::longFileName(path).utf16());
    } , {
        return ::RemoveDirectoryA(QFSFileEnginePrivate::win95Name(QFileInfo(path).absoluteFilePath()));
    });
}

/*!
    \reimp
*/
static inline bool isDirPath(const QString &dirPath, bool *existed)
{
    QString path = dirPath;
    if (path.length() == 2 &&path.at(1) == QLatin1Char(':'))
        path += QLatin1Char('\\');

    DWORD fileAttrib = INVALID_FILE_ATTRIBUTES;
    QT_WA({
        fileAttrib = ::GetFileAttributesW((TCHAR*)QFSFileEnginePrivate::longFileName(path).utf16());
    } , {
        fileAttrib = ::GetFileAttributesA(QFSFileEnginePrivate::win95Name(QFileInfo(path).absoluteFilePath()));
    });

    if (existed)
        *existed = fileAttrib != INVALID_FILE_ATTRIBUTES;

    if (fileAttrib == INVALID_FILE_ATTRIBUTES)
        return false;

    return fileAttrib & FILE_ATTRIBUTE_DIRECTORY;
}

/*!
    \reimp
*/
bool QFSFileEngine::mkdir(const QString &name, bool createParentDirectories) const
{
    QString dirName = name;
    if (createParentDirectories) {
        dirName = QDir::toNativeSeparators(QDir::cleanPath(dirName));
        // We spefically search for / so \ would break it..
        int oldslash = -1;
        if (dirName.startsWith(QLatin1String("\\\\"))) {
            // Don't try to create the root path of a UNC path;
            // CreateDirectory() will just return ERROR_INVALID_NAME.
            for (int i = 0; i < dirName.size(); ++i) {
                if (dirName.at(i) != QDir::separator()) {
                    oldslash = i;
                    break;
                }
            }
            if (oldslash != -1)
                oldslash = dirName.indexOf(QDir::separator(), oldslash);
        }
        for (int slash=0; slash != -1; oldslash = slash) {
            slash = dirName.indexOf(QDir::separator(), oldslash+1);
            if (slash == -1) {
                if(oldslash == dirName.length())
                    break;
                slash = dirName.length();
            }
            if (slash) {
                QString chunk = dirName.left(slash);
                bool existed = false;
                if (!isDirPath(chunk, &existed) && !existed) {
                    if (!mkDir(chunk))
                        return false;
                }
            }
        }
        return true;
    }
    return mkDir(name);
}

/*!
    \reimp
*/
bool QFSFileEngine::rmdir(const QString &name, bool recurseParentDirectories) const
{
    QString dirName = name;
    if (recurseParentDirectories) {
        dirName = QDir::toNativeSeparators(QDir::cleanPath(dirName));
        for (int oldslash = 0, slash=dirName.length(); slash > 0; oldslash = slash) {
            QString chunk = dirName.left(slash);
            if (chunk.length() == 2 && chunk.at(0).isLetter() && chunk.at(1) == QLatin1Char(':'))
                break;
            if (!isDirPath(chunk, 0))
                return false;
            if (!rmDir(chunk))
                return oldslash != 0;
            slash = dirName.lastIndexOf(QDir::separator(), oldslash-1);
        }
        return true;
    }
    return rmDir(name);
}

/*!
    \reimp
*/
bool QFSFileEngine::caseSensitive() const
{
    return false;
}

/*!
    Sets the current path (e.g., for QDir), to \a path.

    \sa currentPath()
*/
bool QFSFileEngine::setCurrentPath(const QString &path)
{
    if (!QDir(path).exists())
        return false;

    int r;
    QT_WA({
        r = ::SetCurrentDirectoryW((WCHAR*)path.utf16());
    } , {
        r = ::SetCurrentDirectoryA(QFSFileEnginePrivate::win95Name(path));
    });
    return r != 0;
}

/*!
    Returns the canonicalized form of the current path used by the file
    engine for the drive specified by \a fileName.

    On Windows, each drive has its own current directory, so a different
    path is returned for file names that include different drive names
    (e.g. A: or C:).

    \sa setCurrentPath()
*/
QString QFSFileEngine::currentPath(const QString &fileName)
{
    QString ret;
    //if filename is a drive: then get the pwd of that drive
    if (fileName.length() >= 2 &&
        fileName.at(0).isLetter() && fileName.at(1) == QLatin1Char(':')) {
        int drv = fileName.toUpper().at(0).toLatin1() - 'A' + 1;
        if (_getdrive() != drv) {
            QT_WA({
                TCHAR buf[PATH_MAX];
                ::_wgetdcwd(drv, buf, PATH_MAX);
                ret.setUtf16((ushort*)buf, uint(::wcslen(buf)));
            }, {
                char buf[PATH_MAX];
                ::_getdcwd(drv, buf, PATH_MAX);
                ret = QString::fromLatin1(buf);
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
                    delete [] newCurrentName;
                } else {
                    ret = QString::fromUtf16((ushort*)currentName);
                }
            }
        } , {
            DWORD size = 0;
            char currentName[PATH_MAX];
            size = ::GetCurrentDirectoryA(PATH_MAX, currentName);
            if (size !=0)
                ret = QString::fromLocal8Bit(currentName);
        });
    }
    if (ret.length() >= 2 && ret[1] == QLatin1Char(':'))
        ret[0] = ret.at(0).toUpper(); // Force uppercase drive letters.
    return QDir::fromNativeSeparators(ret);
}

/*!
    Returns the home path of the current user.

    \sa rootPath()
*/
QString QFSFileEngine::homePath()
{
    QString ret;
#if !defined(QT_NO_LIBRARY)
    QT_WA (
    {
        QFSFileEnginePrivate::resolveLibs();
		if (ptrOpenProcessToken && ptrGetUserProfileDirectoryW) {
			HANDLE hnd = ::GetCurrentProcess();
			HANDLE token = 0;
			BOOL ok = ::ptrOpenProcessToken(hnd, TOKEN_QUERY, &token);
			if (ok) {
				DWORD dwBufferSize = 0;
				// First call, to determine size of the strings (with '\0').
				ok = ::ptrGetUserProfileDirectoryW(token, NULL, &dwBufferSize);
				if (!ok && dwBufferSize != 0) {		// We got the required buffer size
					wchar_t *userDirectory = new wchar_t[dwBufferSize];
					// Second call, now we can fill the allocated buffer.
					ok = ::ptrGetUserProfileDirectoryW(token, userDirectory, &dwBufferSize);
					if (ok)
						ret = QString::fromUtf16((ushort*)userDirectory);

					delete [] userDirectory;
				}
				::CloseHandle(token);
			}
		}
    }
    ,
    {
        // GetUserProfileDirectory is only available from NT 4.0,
		// so fall through for Win98 and friends version.
    })
#endif
    if(ret.isEmpty() || !QFile::exists(ret)) {
        ret = QString::fromLocal8Bit(qgetenv("USERPROFILE").constData());
        if(ret.isEmpty() || !QFile::exists(ret)) {
            ret = QString::fromLocal8Bit(qgetenv("HOMEDRIVE").constData()) + QString::fromLocal8Bit(qgetenv("HOMEPATH").constData());
            if(ret.isEmpty() || !QFile::exists(ret)) {
                ret = QString::fromLocal8Bit(qgetenv("HOME").constData());
                if(ret.isEmpty() || !QFile::exists(ret))
                    ret = rootPath();
            }
        }
    }
    return QDir::fromNativeSeparators(ret);
}

/*!
    Returns the root path.

    \sa homePath()
*/
QString QFSFileEngine::rootPath()
{
#if defined(Q_FS_FAT)
    QString ret = QString::fromLatin1(qgetenv("SystemDrive").constData());
    if(ret.isEmpty())
        ret = QLatin1String("c:");
    ret += QLatin1String("/");
#elif defined(Q_OS_OS2EMX)
    char dir[4];
    _abspath(dir, QLatin1String("/"), _MAX_PATH);
    QString ret(dir);
#endif
    return ret;
}

/*!
    Returns the temporary path (i.e., a path in which it is safe to store
    temporary files).
*/
QString QFSFileEngine::tempPath()
{
    QString ret;
    QT_WA({
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        ret = QString::fromUtf16((ushort*)tempPath);
    } , {
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        ret = QString::fromLocal8Bit(tempPath);
    });
    if (ret.isEmpty()) {
        ret = QString::fromLatin1("c:/tmp");
    } else {
        ret = QDir::fromNativeSeparators(ret);
        while (ret.at(ret.length()-1) == QLatin1Char('/'))
            ret = ret.left(ret.length()-1);
    }
    return ret;
}

/*!
    Returns the list of drives in the file system as a list of QFileInfo
    objects. On unix and Mac OS X, only the root path is returned. On Windows,
    this function returns all drives (A:\, C:\, D:\, etc.).
*/
QFileInfoList QFSFileEngine::drives()
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

bool QFSFileEnginePrivate::doStat() const
{
    if (!tried_stat) {
        tried_stat = true;
        could_stat = false;

        if (filePath.isEmpty())
            return could_stat;
        QString fname = filePath.endsWith(QLatin1String(".lnk")) ? readLink(filePath) : filePath;
        fname = fixIfRelativeUncPath(fname);

        UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

        if (fd != -1) {
            HANDLE fh = (HANDLE)_get_osfhandle(fd);
            if (fh != INVALID_HANDLE_VALUE) {
                BY_HANDLE_FILE_INFORMATION fileInfo;
                if (GetFileInformationByHandle(fh, &fileInfo)) {
                    could_stat = true;
                    fileAttrib = fileInfo.dwFileAttributes;
                }
            }
        } else {
            QT_WA({
                fileAttrib = GetFileAttributesW((TCHAR*)QFSFileEnginePrivate::longFileName(fname).utf16());
            } , {
                fileAttrib = GetFileAttributesA(QFSFileEnginePrivate::win95Name(QFileInfo(fname).absoluteFilePath()));
            });
            could_stat = fileAttrib != INVALID_FILE_ATTRIBUTES;
            if (!could_stat) {
                if (!fname.isEmpty() && fname.at(0).isLetter() && fname.mid(1, fname.length()) == QLatin1String(":/")) {
                    // an empty drive ??
                    DWORD drivesBitmask = ::GetLogicalDrives();
                    int drivebit = 1 << (fname.at(0).toUpper().unicode() - QLatin1Char('A').unicode());
                    if (drivesBitmask & drivebit) {
                        fileAttrib = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM;
                        could_stat = true;
                    }
                } else {
                    QString path = QDir::toNativeSeparators(fname);
                    bool is_dir = false;
                    if (path.startsWith(QLatin1String("\\\\"))) {
                        // UNC - stat doesn't work for all cases (Windows bug)
                        int s = path.indexOf(path.at(0),2);
                        if (s > 0) {
                            // "\\server\..."
                            s = path.indexOf(path.at(0),s+1);
                            if (s > 0) {
                                // "\\server\share\..."
                                if (s == path.size() - 1) {
                                    // "\\server\share\"
                                    is_dir = true;
                                } else {
                                    // "\\server\share\notfound"
                                }
                            } else {
                                // "\\server\share"
                                is_dir = true;
                            }
                        } else {
                            // "\\server"
                            is_dir = true;
                        }
                    }
                    if (is_dir && uncShareExists(path)) {
                        // looks like a UNC dir, is a dir.
                        fileAttrib = FILE_ATTRIBUTE_DIRECTORY;
                        could_stat = true;
                    }
                }
            }
        }
        SetErrorMode(oldmode);
    }
    return could_stat;
}


static QString readLink(const QString &link)
{
#if !defined(QT_NO_LIBRARY)
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

        if(hres == CO_E_NOTINITIALIZED) { // COM was not initialized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLink, (LPVOID *)&psl);
        }
        if(SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if(SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)link.utf16(), STGM_READ);
                //The original path of the link is retrieved. If the file/folder
                //was moved, the return value still have the old path.
                if(SUCCEEDED(hres)) {
                    if (psl->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY) == NOERROR)
                        ret = QString::fromUtf16((ushort*)szGotPath);
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

        if(hres == CO_E_NOTINITIALIZED) { // COM was not initialized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLinkA, (LPVOID *)&psl);
        }
        if(SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if(SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)QFileInfo(link).absoluteFilePath().utf16(), STGM_READ);
                //The original path of the link is retrieved. If the file/folder
                //was moved, the return value still have the old path.
                 if(SUCCEEDED(hres)) {
                    if (psl->GetPath((char*)szGotPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY) == NOERROR)
                        ret = QString::fromLocal8Bit(szGotPath);
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
    Q_UNUSED(link);
    return QString();
#endif // QT_NO_LIBRARY
}

/*!
    \internal
*/
QString QFSFileEnginePrivate::getLink() const
{
    return readLink(filePath);
}

/*!
    \reimp
*/
bool QFSFileEngine::link(const QString &newName)
{
#if !defined(QT_NO_LIBRARY)
    bool ret = false;

    QString linkName = newName;
    //### assume that they add .lnk

    QT_WA({
        HRESULT hres;
        IShellLink *psl;
        bool neededCoInit = false;

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        if(hres == CO_E_NOTINITIALIZED) { // COM was not initialized
                neededCoInit = true;
                CoInitialize(NULL);
                hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        }
        if (SUCCEEDED(hres)) {
            hres = psl->SetPath((wchar_t *)fileName(AbsoluteName).replace(QLatin1Char('/'), QLatin1Char('\\')).utf16());
            if (SUCCEEDED(hres)) {
                hres = psl->SetWorkingDirectory((wchar_t *)fileName(AbsolutePathName).replace(QLatin1Char('/'), QLatin1Char('\\')).utf16());
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
            }
            psl->Release();
        }
        if(neededCoInit)
                CoUninitialize();
    } , {
        // the SetPath() call _sometimes_ changes the current path and when it does it sometimes
        // does not let us change it back unless we call currentPath() many times.
        QString cwd = currentPath();
        HRESULT hres;
        IShellLinkA *psl;
        bool neededCoInit = false;

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        if(hres == CO_E_NOTINITIALIZED) { // COM was not initialized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        }
        if (SUCCEEDED(hres)) {
            currentPath();
            hres = psl->SetPath((char *)QString::fromLocal8Bit(QFSFileEnginePrivate::win95Name(fileName(AbsoluteName))).utf16());
            currentPath();
            if (SUCCEEDED(hres)) {
                hres = psl->SetWorkingDirectory((char *)QString::fromLocal8Bit(QFSFileEnginePrivate::win95Name(fileName(AbsolutePathName))).utf16());
                currentPath();
                if (SUCCEEDED(hres)) {
                    IPersistFile *ppf;
                    hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
                    if (SUCCEEDED(hres)) {
                        currentPath();
                        hres = ppf->Save((LPCOLESTR)linkName.utf16(), TRUE);
                        currentPath();
                        if (SUCCEEDED(hres))
                            ret = true;
                        ppf->Release();
                    }
                }
                psl->Release();
            }
        }
        if(neededCoInit)
            CoUninitialize();
        setCurrentPath(cwd);
    });
    return ret;
#else
    Q_UNUSED(newName);
    return false;
#endif // QT_NO_LIBRARY
}

/*!
    \internal
*/
QAbstractFileEngine::FileFlags QFSFileEnginePrivate::getPermissions() const
{
    QAbstractFileEngine::FileFlags ret = 0;

#if !defined(QT_NO_LIBRARY)
    if((qt_ntfs_permission_lookup > 0) && ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_NT)) {
	PSID pOwner = 0;
	PSID pGroup = 0;
	PACL pDacl;
        PSECURITY_DESCRIPTOR pSD;
        ACCESS_MASK access_mask;

        enum { ReadMask = 0x00000001, WriteMask = 0x00000002, ExecMask = 0x00000020 };
        resolveLibs();
        if(ptrGetNamedSecurityInfoW && ptrAllocateAndInitializeSid && ptrBuildTrusteeWithSidW && ptrGetEffectiveRightsFromAclW && ptrFreeSid) {

            QString fname = filePath.endsWith(QLatin1String(".lnk")) ? readLink(filePath) : filePath;
            DWORD res = ptrGetNamedSecurityInfoW((wchar_t*)fname.utf16(), SE_FILE_OBJECT,
						 OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
						 &pOwner, &pGroup, &pDacl, 0, &pSD);

            if(res == ERROR_SUCCESS) {
                TRUSTEE_W trustee;
                { //user
                    if(ptrGetEffectiveRightsFromAclW(pDacl, &currentUserTrusteeW, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QAbstractFileEngine::ReadUserPerm;
		    if(access_mask & WriteMask)
			ret |= QAbstractFileEngine::WriteUserPerm;
		    if(access_mask & ExecMask)
			ret |= QAbstractFileEngine::ExeUserPerm;
                }
                { //owner
                    ptrBuildTrusteeWithSidW(&trustee, pOwner);
                    if(ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QAbstractFileEngine::ReadOwnerPerm;
		    if(access_mask & WriteMask)
			ret |= QAbstractFileEngine::WriteOwnerPerm;
		    if(access_mask & ExecMask)
			ret |= QAbstractFileEngine::ExeOwnerPerm;
                }
                { //group
                    ptrBuildTrusteeWithSidW(&trustee, pGroup);
                    if(ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS)
                        access_mask = (ACCESS_MASK)-1;
		    if(access_mask & ReadMask)
			ret |= QAbstractFileEngine::ReadGroupPerm;
		    if(access_mask & WriteMask)
			ret |= QAbstractFileEngine::WriteGroupPerm;
		    if(access_mask & ExecMask)
			ret |= QAbstractFileEngine::ExeGroupPerm;
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
			    ret |= QAbstractFileEngine::ReadOtherPerm;
			if(access_mask & WriteMask)
			    ret |= QAbstractFileEngine::WriteOtherPerm;
			if(access_mask & ExecMask)
			    ret |= QAbstractFileEngine::ExeOtherPerm;
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
    	ret |= QAbstractFileEngine::ReadOtherPerm | QAbstractFileEngine::ReadGroupPerm
	    | QAbstractFileEngine::ReadOwnerPerm | QAbstractFileEngine::ReadUserPerm
	    | QAbstractFileEngine::WriteUserPerm | QAbstractFileEngine::WriteOwnerPerm
	    | QAbstractFileEngine::WriteGroupPerm | QAbstractFileEngine::WriteOtherPerm;
    }

    if (doStat()) {
        if (ret & (QAbstractFileEngine::WriteOwnerPerm | QAbstractFileEngine::WriteUserPerm |
            QAbstractFileEngine::WriteGroupPerm | QAbstractFileEngine::WriteOtherPerm)) {
            if (fileAttrib & FILE_ATTRIBUTE_READONLY)
                ret &= ~(QAbstractFileEngine::WriteOwnerPerm | QAbstractFileEngine::WriteUserPerm |
                QAbstractFileEngine::WriteGroupPerm | QAbstractFileEngine::WriteOtherPerm);
        }

        QString ext = filePath.right(4).toLower();
        if (ext == QLatin1String(".exe") || ext == QLatin1String(".com") || ext == QLatin1String(".bat") ||
            ext == QLatin1String(".pif") || ext == QLatin1String(".cmd") || (fileAttrib & FILE_ATTRIBUTE_DIRECTORY))
            ret |= QAbstractFileEngine::ExeOwnerPerm | QAbstractFileEngine::ExeGroupPerm |
            QAbstractFileEngine::ExeOtherPerm | QAbstractFileEngine::ExeUserPerm;
    }
    return ret;
}

/*!
    \reimp
*/
QAbstractFileEngine::FileFlags QFSFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    Q_D(const QFSFileEngine);
    QAbstractFileEngine::FileFlags ret = 0;
    // Force a stat, so that we're guaranteed to get up-to-date results
    if (type & QAbstractFileEngine::FileFlag(QAbstractFileEngine::Refresh)) {
        d->tried_stat = 0;
    }

    if (type & PermsMask) {
        ret |= d->getPermissions();
        // ### Workaround pascals ### above. Since we always set all properties to true
        // we need to disable read and exec access if the file does not exists
        if (d->doStat())
            ret |= ExistsFlag;
        else
            ret &= 0x2222;
    }
    if (type & TypesMask) {
        if (d->filePath.endsWith(QLatin1String(".lnk"))) {
            ret |= LinkType;
            QString l = readLink(d->filePath);
            if (!l.isEmpty()) {
                if (isDirPath(l, 0))
                    ret |= DirectoryType;
                else
                    ret |= FileType;
            }
        } else if (d->doStat()) {
            if (d->fileAttrib & FILE_ATTRIBUTE_DIRECTORY) {
                ret |= DirectoryType;
            } else {
                ret |= FileType;
            }
        }
    }
    if (type & FlagsMask) {
        if(d->doStat()) {
            ret |= QAbstractFileEngine::FileFlags(ExistsFlag | LocalDiskFlag);
            if (d->fileAttrib & FILE_ATTRIBUTE_HIDDEN)
                ret |= HiddenFlag;
            if (d->filePath == QLatin1String("/") || (d->filePath.at(0).isLetter() && d->filePath.mid(1,d->filePath.length()) == QLatin1String(":/"))
                || isUncRoot(d->filePath))
                ret |= RootFlag;
        }
    }
    return ret;
}

/*!
    \reimp
*/
QString QFSFileEngine::fileName(FileName file) const
{
    Q_D(const QFSFileEngine);
    if(file == BaseName) {
        int slash = d->filePath.lastIndexOf(QLatin1Char('/'));
        if(slash == -1) {
            int colon = d->filePath.lastIndexOf(QLatin1Char(':'));
            if(colon != -1)
                return d->filePath.mid(colon + 1);
            return d->filePath;
        }
        return d->filePath.mid(slash + 1);
    } else if(file == PathName) {
        if(!d->filePath.size())
            return d->filePath;

        int slash = d->filePath.lastIndexOf(QLatin1Char('/'));
        if(slash == -1) {
            if(d->filePath.length() >= 2 && d->filePath.at(1) == QLatin1Char(':'))
                return d->filePath.left(2);
            return QString::fromLatin1(".");
        } else {
            if(!slash)
                return QString::fromLatin1("/");
            if(slash == 2 && d->filePath.length() >= 2 && d->filePath.at(1) == QLatin1Char(':'))
                slash++;
            return d->filePath.left(slash);
        }
    } else if(file == AbsoluteName || file == AbsolutePathName) {
        QString ret;

        if (!isRelativePath()) {
            if (d->filePath.size() > 2 && d->filePath.at(1) == QLatin1Char(':')
                && d->filePath.at(2) != QLatin1Char('/') || // It's a drive-relative path, so Z:a.txt -> Z:\currentpath\a.txt
                d->filePath.startsWith(QLatin1Char('/'))    // It's a absolute path to the current drive, so \a.txt -> Z:\a.txt
                ) {
                ret = QDir::fromNativeSeparators(nativeAbsoluteFilePath(d->filePath));
            } else {
                ret = d->filePath;
            }
        } else {
            ret = QDir::cleanPath(QDir::currentPath() + QLatin1Char('/') + d->filePath);
        }

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
            if (slash < 0)
                return ret;
            else if (ret.at(0) != QLatin1Char('/') && slash == 2)
                return ret.left(3);      // include the slash
            else
                return ret.left(slash > 0 ? slash : 1);
        }
        return ret;
    } else if(file == CanonicalName || file == CanonicalPathName) {
        QString ret;
        if ((fileFlags(ExistsFlag) & ExistsFlag)) {
            QString abs;
            bool attach_basename = false;
            if (fileFlags(DirectoryType) & DirectoryType) {
                if (d->filePath.endsWith(QLatin1String(".lnk")))
                    abs = fileName(LinkName);
                else
                    abs = fileName(AbsoluteName);
            } else {
                if(file == CanonicalName)
                    attach_basename = true;
                abs = fileName(AbsolutePathName);
            }
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
            if (attach_basename) {
                if (!ret.endsWith(QLatin1Char('/')))
                    ret += QLatin1Char('/');
                ret += fileName(BaseName);
            }

            ret[0] = ret.at(0).toUpper(); // Force uppercase drive letters.
            return QDir::fromNativeSeparators(ret);
        }
        return ret;
    } else if(file == LinkName) {
        return QDir::fromNativeSeparators(d->getLink());
    }
    return d->filePath;
}

/*!
    \reimp
*/
bool QFSFileEngine::isRelativePath() const
{
    Q_D(const QFSFileEngine);
    return !(d->filePath.startsWith(QLatin1Char('/'))
        || (d->filePath.length() >= 2
        && ((d->filePath.at(0).isLetter() && d->filePath.at(1) == QLatin1Char(':'))
        || (d->filePath.at(0) == QLatin1Char('/') && d->filePath.at(1) == QLatin1Char('/')))));                // drive, e.g. a:
}

/*!
    \reimp
*/
uint QFSFileEngine::ownerId(FileOwner /*own*/) const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

/*!
    \reimp
*/
QString QFSFileEngine::owner(FileOwner own) const
{
#if !defined(QT_NO_LIBRARY)
    Q_D(const QFSFileEngine);
    if((qt_ntfs_permission_lookup > 0) && ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_NT)) {
	PSID pOwner = 0;
	PSECURITY_DESCRIPTOR pSD;
	QString name;
	QFSFileEnginePrivate::resolveLibs();

	if(ptrGetNamedSecurityInfoW && ptrLookupAccountSidW) {
	    if(ptrGetNamedSecurityInfoW((wchar_t*)d->filePath.utf16(), SE_FILE_OBJECT,
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
    return QString(QLatin1String(""));
}

/*!
    \reimp
*/
bool QFSFileEngine::setPermissions(uint perms)
{
    Q_D(QFSFileEngine);
    bool ret = false;
    int mode = 0;

    if (perms & QFile::ReadOwner || perms & QFile::ReadUser || perms & QFile::ReadGroup || perms & QFile::ReadOther)
        mode |= _S_IREAD;
    if (perms & QFile::WriteOwner || perms & QFile::WriteUser || perms & QFile::WriteGroup || perms & QFile::WriteOther)
        mode |= _S_IWRITE;

    if (mode == 0) // not supported
        return false;

   QT_WA({
        ret = ::_wchmod((TCHAR*)d->filePath.utf16(), mode) == 0;
   } , {
        ret = ::_chmod(d->filePath.toLocal8Bit(), mode) == 0;
   });
   return ret;
}

/*!
    \reimp
*/
bool QFSFileEngine::setSize(qint64 size)
{
    Q_D(QFSFileEngine);
    
    if (d->fileHandle != INVALID_HANDLE_VALUE || d->fd != -1) {
        // resize open file
        HANDLE fh = d->fileHandle;
        if (fh == INVALID_HANDLE_VALUE)
            fh = (HANDLE)_get_osfhandle(d->fd);
        if (fh == INVALID_HANDLE_VALUE)
            return false;
        qint64 currentPos = pos();

        if (seek(size) && SetEndOfFile(d->fileHandle)) {
            seek(qMin(currentPos, size));
            return true;
        }

        seek(currentPos);
        return false;
    }

    if (!d->nativeFilePath.isEmpty()) {
        // resize file on disk
        QFile file1(QString::fromLatin1(QFile::encodeName(d->filePath)));
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
                file1.remove();
                if (file2.rename(file1.fileName()))
                    return true;
                qWarning("QFSFileEngine::rename: Failed to rename %s: %s",
                    qPrintable(file2.fileName()), qPrintable(qt_error_string(errno)));
            }
        }
    }
    return false;
}


static inline QDateTime fileTimeToQDateTime(const FILETIME *time)
{
    QDateTime ret;
    FILETIME localtime;
    SYSTEMTIME lTime;
    FileTimeToLocalFileTime(time, &localtime);
    FileTimeToSystemTime(&localtime, &lTime);
    ret.setDate(QDate(lTime.wYear, lTime.wMonth, lTime.wDay));
    ret.setTime(QTime(lTime.wHour, lTime.wMinute, lTime.wSecond, lTime.wMilliseconds));
    return ret;
}

/*!
    \reimp
*/
QDateTime QFSFileEngine::fileTime(FileTime time) const
{
    Q_D(const QFSFileEngine);
    QDateTime ret;
    if (d->fd != -1) {
        HANDLE fh = (HANDLE)_get_osfhandle(d->fd);
        if (fh != INVALID_HANDLE_VALUE) {
            FILETIME creationTime, lastAccessTime, lastWriteTime;
            if (GetFileTime(fh, &creationTime, &lastAccessTime, &lastWriteTime)) {
                if(time == CreationTime)
                    ret = fileTimeToQDateTime(&creationTime);
                else if(time == ModificationTime)
                    ret = fileTimeToQDateTime(&lastWriteTime);
                else if(time == AccessTime)
                    ret = fileTimeToQDateTime(&lastAccessTime);
            }
        }
    } else {
        bool ok = false;
        WIN32_FILE_ATTRIBUTE_DATA attribData;
        QT_WA({
            ok = ::GetFileAttributesExW((TCHAR*)QFSFileEnginePrivate::longFileName(d->filePath).utf16(), GetFileExInfoStandard, &attribData);
        } , {
            ok = ::GetFileAttributesExA(QFSFileEnginePrivate::win95Name(QFileInfo(d->filePath).absoluteFilePath()), GetFileExInfoStandard, &attribData);
        });
        if (ok) {
            if(time == CreationTime)
                ret = fileTimeToQDateTime(&attribData.ftCreationTime);
            else if(time == ModificationTime)
                ret = fileTimeToQDateTime(&attribData.ftLastWriteTime);
            else if(time == AccessTime)
                ret = fileTimeToQDateTime(&attribData.ftLastAccessTime);
        }
    }
    return ret;
}
