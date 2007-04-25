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

#include "qplatformdefs.h"
#include "qabstractfileengine.h"
#include "private/qfsfileengine_p.h"
#ifndef QT_NO_REGEXP
# include "qregexp.h"
#endif
#include "qfile.h"
#include "qdir.h"
#include "qdatetime.h"
#include "qdebug.h"
#include "qvarlengtharray.h"

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#if !defined(QWS) && defined(Q_OS_MAC)
# include <private/qcore_mac_p.h>
#endif

/*!
    \internal

    Returns the stdlib open string corresponding to a QIODevice::OpenMode.
*/
static QByteArray openModeToFopenMode(QIODevice::OpenMode flags, const QString &fileName = QString())
{
    QByteArray mode;
    if ((flags & QIODevice::ReadOnly) && !(flags & QIODevice::Truncate)) {
        mode = "rb";
        if (flags & QIODevice::WriteOnly) {
            if (!fileName.isEmpty() &&QFile::exists(fileName))
                mode = "rb+";
            else
                mode = "wb+";
        }
    } else if (flags & QIODevice::WriteOnly) {
        mode = "wb";
        if (flags & QIODevice::ReadOnly)
            mode += "+";
    }
    if (flags & QIODevice::Append) {
        mode = "ab";
        if (flags & QIODevice::ReadOnly)
            mode += "+";
    }
    return mode;
}

/*!
    \internal

    Returns the stdio open flags corresponding to a QIODevice::OpenMode.
*/
static int openModeToOpenFlags(QIODevice::OpenMode mode)
{
    int oflags = QT_OPEN_RDONLY;
#ifdef QT_LARGEFILE_SUPPORT
    oflags |= QT_OPEN_LARGEFILE;
#endif

    if ((mode & QFile::ReadWrite) == QFile::ReadWrite) {
        oflags = QT_OPEN_RDWR | QT_OPEN_CREAT;
    } else if (mode & QFile::WriteOnly) {
        oflags = QT_OPEN_WRONLY | QT_OPEN_CREAT;
    }

    if (mode & QFile::Append) {
        oflags |= QT_OPEN_APPEND;
    } else if (mode & QFile::WriteOnly) {
        if ((mode & QFile::Truncate) || !(mode & QFile::ReadOnly))
            oflags |= QT_OPEN_TRUNC;
    }

    return oflags;
}

/*!
    \internal
*/
void QFSFileEnginePrivate::nativeInitFileName()
{
    nativeFilePath = QFile::encodeName(filePath);
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeOpen(QIODevice::OpenMode openMode)
{
    Q_Q(QFSFileEngine);

    if (openMode & QIODevice::Unbuffered) {
        int flags = openModeToOpenFlags(openMode);

        // Try to open the file in unbuffered mode.
        do {
            fd = QT_OPEN(nativeFilePath.constData(), flags, 0666);
        } while (fd == -1 && errno == EINTR);

        // On failure, return and report the error.
        if (fd == -1) {
            q->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                        qt_error_string(errno));
            return false;
        }

        // Seek to the end when in Append mode.
        if (flags & QFile::Append) {
            int ret;
            do {
                ret = QT_LSEEK(fd, 0, SEEK_END);
            } while (ret == -1 && errno == EINTR);

            if (ret == -1) {
                q->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                            qt_error_string(int(errno)));
                return false;
            }
        }

        fh = 0;
    } else {
        QByteArray fopenMode = openModeToFopenMode(openMode, filePath);

        // Try to open the file in buffered mode.
        do {
            fh = QT_FOPEN(nativeFilePath.constData(), fopenMode.constData());
        } while (!fh && errno == EINTR);

        // On failure, return and report the error.
        if (!fh) {
            q->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                        qt_error_string(int(errno)));
            return false;
        }

        // Seek to the end when in Append mode.
        if (openMode & QIODevice::Append) {
            int ret;
            do {
                ret = QT_FSEEK(fh, 0, SEEK_END);
            } while (ret == -1 && errno == EINTR);

            if (ret == -1) {
                q->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
                            qt_error_string(int(errno)));
                return false;
            }
        }

        fd = -1;
    }

    closeFileHandle = true;
    return true;
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeClose()
{
    return closeFdFh();
}

/*!
    \internal

*/
bool QFSFileEnginePrivate::nativeFlush()
{
    return fh ? flushFh() : fd != -1;
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativeRead(char *data, qint64 len)
{
    Q_Q(QFSFileEngine);

    if (fh && nativeIsSequential()) {
        size_t readBytes = 0;
        int oldFlags = fcntl(QT_FILENO(fh), F_GETFL);
        for (int i = 0; i < 2; ++i) {
            // Unix: Make the underlying file descriptor non-blocking
            int v = 1;
            if ((oldFlags & O_NONBLOCK) == 0)
                fcntl(QT_FILENO(fh), F_SETFL, oldFlags | O_NONBLOCK, &v, sizeof(v));

            // Cross platform stdlib read
            size_t read = 0;
            do {
                read = fread(data + readBytes, 1, size_t(len - readBytes), fh);
            } while (read == 0 && !feof(fh) && errno == EINTR);
            if (read > 0) {
                readBytes += read;
                break;
            } else {
                if (readBytes)
                    break;
                readBytes = read;
            }

            // Unix: Restore the blocking state of the underlying socket
            if ((oldFlags & O_NONBLOCK) == 0) {
                int v = 1;
                fcntl(QT_FILENO(fh), F_SETFL, oldFlags, &v, sizeof(v));
                if (readBytes == 0) {
                    int readByte = 0;
                    do {
                        readByte = fgetc(fh);
                    } while (readByte == -1 && errno == EINTR);
                    if (readByte != -1) {
                        *data = uchar(readByte);
                        readBytes += 1;
                    } else {
                        break;
                    }
                }
            }
        }
        // Unix: Restore the blocking state of the underlying socket
        if ((oldFlags & O_NONBLOCK) == 0) {
            int v = 1;
            fcntl(QT_FILENO(fh), F_SETFL, oldFlags, &v, sizeof(v));
        }
        if (readBytes == 0) {
            q->setError(QFile::ReadError, qt_error_string(int(errno)));
            return -1;
        }
        return readBytes;
    }

    return readFdFh(data, len);
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativeReadLine(char *data, qint64 maxlen)
{
    return readLineFdFh(data, maxlen);
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativeWrite(const char *data, qint64 len)
{
    return writeFdFh(data, len);
}

/*!
    \internal
*/
qint64 QFSFileEnginePrivate::nativePos() const
{
    return posFdFh();
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeSeek(qint64 pos)
{
    return seekFdFh(pos);
}

/*!
    \internal
*/
int QFSFileEnginePrivate::nativeHandle() const
{
    return fh ? fileno(fh) : fd;
}

/*!
    \internal
*/
bool QFSFileEnginePrivate::nativeIsSequential() const
{
    return isSequentialFdFh();
}

bool QFSFileEngine::remove()
{
    Q_D(QFSFileEngine);
    return unlink(d->nativeFilePath.constData()) == 0;
}

bool QFSFileEngine::copy(const QString &)
{
    // ### Add copy code for Unix here
    return false;
}

bool QFSFileEngine::rename(const QString &newName)
{
    Q_D(QFSFileEngine);
    return ::rename(d->nativeFilePath.constData(), QFile::encodeName(newName).constData()) == 0;
}

bool QFSFileEngine::link(const QString &newName)
{
    Q_D(QFSFileEngine);
    return ::symlink(d->nativeFilePath.constData(), QFile::encodeName(newName).constData()) == 0;
}

qint64 QFSFileEnginePrivate::nativeSize() const
{
    return sizeFdFh();
}

bool QFSFileEngine::mkdir(const QString &name, bool createParentDirectories) const
{
    QString dirName = name;
    if (createParentDirectories) {
        dirName = QDir::cleanPath(dirName);
        for(int oldslash = -1, slash=0; slash != -1; oldslash = slash) {
            slash = dirName.indexOf(QDir::separator(), oldslash+1);
            if (slash == -1) {
                if (oldslash == dirName.length())
                    break;
                slash = dirName.length();
            }
            if (slash) {
                QByteArray chunk = QFile::encodeName(dirName.left(slash));
                QT_STATBUF st;
                if (QT_STAT(chunk, &st) != -1) {
                    if ((st.st_mode & S_IFMT) != S_IFDIR)
                        return false;
                } else if (::mkdir(chunk, 0777) != 0) {
                        return false;
                }
            }
        }
        return true;
    }
#if defined(Q_OS_DARWIN)  // Mac X doesn't support trailing /'s
    if (dirName[dirName.length() - 1] == QLatin1Char('/'))
        dirName = dirName.left(dirName.length() - 1);
#endif
    return (::mkdir(QFile::encodeName(dirName), 0777) == 0);
}

bool QFSFileEngine::rmdir(const QString &name, bool recurseParentDirectories) const
{
    QString dirName = name;
    if (recurseParentDirectories) {
        dirName = QDir::cleanPath(dirName);
        for(int oldslash = 0, slash=dirName.length(); slash > 0; oldslash = slash) {
            QByteArray chunk = QFile::encodeName(dirName.left(slash));
            QT_STATBUF st;
            if (QT_STAT(chunk, &st) != -1) {
                if ((st.st_mode & S_IFMT) != S_IFDIR)
                    return false;
                if (::rmdir(chunk) != 0)
                    return oldslash != 0;
            } else {
                return false;
            }
            slash = dirName.lastIndexOf(QDir::separator(), oldslash-1);
        }
        return true;
    }
    return ::rmdir(QFile::encodeName(dirName)) == 0;
}

bool QFSFileEngine::caseSensitive() const
{
    return true;
}

bool QFSFileEngine::setCurrentPath(const QString &path)
{
    int r;
    r = ::chdir(QFile::encodeName(path));
    return r >= 0;
}

QString QFSFileEngine::currentPath(const QString &)
{
    QString result;
    QT_STATBUF st;
    if (QT_STAT(".", &st) == 0) {
#if defined(__GLIBC__) && !defined(PATH_MAX)
        char *currentName = ::get_current_dir_name();
        if (currentName) {
            result = QFile::decodeName(QByteArray(currentName));
            ::free(currentName);
        }
#else
        char currentName[PATH_MAX+1];
        if (::getcwd(currentName, PATH_MAX))
            result = QFile::decodeName(QByteArray(currentName));
#endif
#if defined(QT_DEBUG)
        if (result.isNull())
            qWarning("QDir::currentPath: getcwd() failed");
#endif
    } else {
#if defined(QT_DEBUG)
        qWarning("QDir::currentPath: stat(\".\") failed");
#endif
    }
    return result;
}

QString QFSFileEngine::homePath()
{
    QString home = QFile::decodeName(qgetenv("HOME"));
    if (home.isNull())
        home = rootPath();
    return home;
}

QString QFSFileEngine::rootPath()
{
    return QString::fromLatin1("/");
}

QString QFSFileEngine::tempPath()
{
    QString temp = QFile::decodeName(qgetenv("TMPDIR"));
    if (temp.isEmpty())
        temp = QString::fromLatin1("/tmp/");
    return temp;
}

QFileInfoList QFSFileEngine::drives()
{
    QFileInfoList ret;
    ret.append(rootPath());
    return ret;
}

bool QFSFileEnginePrivate::doStat() const
{
    if (tried_stat == 0) {
        QFSFileEnginePrivate *that = const_cast<QFSFileEnginePrivate*>(this);
        if (fh && nativeFilePath.isEmpty()) {
            // ### actually covers two cases: d->fh and when the file is not open
            that->could_stat = (QT_FSTAT(fileno(fh), &st) == 0);
        } else if (fd == -1) {
            // ### actually covers two cases: d->fh and when the file is not open
            that->could_stat = (QT_STAT(nativeFilePath.constData(), &st) == 0);
        } else {
            that->could_stat = (QT_FSTAT(fd, &st) == 0);
        }
	that->tried_stat = 1;
    }
    return could_stat;
}

bool QFSFileEnginePrivate::isSymlink() const
{
    if (need_lstat) {
        QFSFileEnginePrivate *that = const_cast<QFSFileEnginePrivate *>(this);
        that->need_lstat = false;
        QT_STATBUF st;          // don't clobber our main one
        that->is_link = (QT_LSTAT(nativeFilePath.constData(), &st) == 0) ? S_ISLNK(st.st_mode) : false;
    }
    return is_link;
}

#if !defined(QWS) && defined(Q_OS_MAC)
static bool _q_isMacHidden(const QString &path)
{
    OSErr err = noErr;

    FSRef fsRef;

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
        err = FSPathMakeRefWithOptions(reinterpret_cast<const UInt8 *>(QFile::encodeName(path).constData()),
                                        kFSPathMakeRefDoNotFollowLeafSymlink, &fsRef, 0);
    } else
#endif
    {
        QFileInfo fi(path);
        FSRef parentRef;
        err = FSPathMakeRef(reinterpret_cast<const UInt8 *>(fi.absoluteDir().canonicalPath().toUtf8().constData()),
                            &parentRef, 0);
        if (err == noErr) {
            QString fileName = fi.fileName();
            err = FSMakeFSRefUnicode(&parentRef, fileName.length(),
                                     reinterpret_cast<const UniChar *>(fileName.unicode()),
                                     kTextEncodingUnknown, &fsRef);
        }
    }
    if (err != noErr)
        return false;

    FSCatalogInfo catInfo;
    err = FSGetCatalogInfo(&fsRef, kFSCatInfoFinderInfo, &catInfo, NULL, NULL, NULL);
    if (err != noErr)
        return false;

    FileInfo * const fileInfo = reinterpret_cast<FileInfo*>(&catInfo.finderInfo);
    bool result = (fileInfo->finderFlags & kIsInvisible);
    return result;
}
#endif

/*!
    \reimp
*/
QAbstractFileEngine::FileFlags QFSFileEngine::fileFlags(FileFlags type) const
{
    Q_D(const QFSFileEngine);
    // Force a stat, so that we're guaranteed to get up-to-date results
    if (type & QAbstractFileEngine::FileFlag(QAbstractFileEngine::Refresh)) {
        d->tried_stat = 0;
        d->need_lstat = 1;
    }

    QAbstractFileEngine::FileFlags ret = 0;
    bool exists = d->doStat();
    if (!exists && !d->isSymlink())
        return ret;

    if (exists && (type & PermsMask)) {
        if (d->st.st_mode & S_IRUSR)
            ret |= ReadOwnerPerm;
        if (d->st.st_mode & S_IWUSR)
            ret |= WriteOwnerPerm;
        if (d->st.st_mode & S_IXUSR)
            ret |= ExeOwnerPerm;
        if (d->st.st_mode & S_IRUSR)
            ret |= ReadUserPerm;
        if (d->st.st_mode & S_IWUSR)
            ret |= WriteUserPerm;
        if (d->st.st_mode & S_IXUSR)
            ret |= ExeUserPerm;
        if (d->st.st_mode & S_IRGRP)
            ret |= ReadGroupPerm;
        if (d->st.st_mode & S_IWGRP)
            ret |= WriteGroupPerm;
        if (d->st.st_mode & S_IXGRP)
            ret |= ExeGroupPerm;
        if (d->st.st_mode & S_IROTH)
            ret |= ReadOtherPerm;
        if (d->st.st_mode & S_IWOTH)
            ret |= WriteOtherPerm;
        if (d->st.st_mode & S_IXOTH)
            ret |= ExeOtherPerm;
    }
    if (type & TypesMask) {
#if !defined(QWS) && defined(Q_OS_MAC)
        bool foundAlias = false;
        {
            FSRef fref;
            if (FSPathMakeRef((const UInt8 *)QFile::encodeName(QDir::cleanPath(d->filePath)).data(),
                             &fref, NULL) == noErr) {
                Boolean isAlias, isFolder;
                if (FSIsAliasFile(&fref, &isAlias, &isFolder) == noErr && isAlias) {
                    foundAlias = true;
                    ret |= LinkType;
                }
            }
        }
        if (!foundAlias)
#endif
        {
            if ((type & LinkType) && d->isSymlink())
                ret |= LinkType;
            if (exists && (d->st.st_mode & S_IFMT) == S_IFREG)
                ret |= FileType;
            else if (exists && (d->st.st_mode & S_IFMT) == S_IFDIR)
                ret |= DirectoryType;
#if !defined(QWS) && defined(Q_OS_MAC)
            if(ret & DirectoryType) {
                QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, QCFString(d->filePath),
                                                                      kCFURLPOSIXPathStyle, true);
                UInt32 type, creator;
                if(CFBundleGetPackageInfoInDirectory(url, &type, &creator))
                    ret |= BundleType;
            }
#endif
        }
    }
    if (type & FlagsMask) {
        ret |= LocalDiskFlag;
        if (exists)
            ret |= ExistsFlag;
        if (fileName(BaseName)[0] == QLatin1Char('.')
#if !defined(QWS) && defined(Q_OS_MAC)
            || _q_isMacHidden(d->filePath)
#endif
        )
            ret |= HiddenFlag;
        if (d->filePath == QLatin1String("/"))
            ret |= RootFlag;
    }
    return ret;
}

QString QFSFileEngine::fileName(FileName file) const
{
    Q_D(const QFSFileEngine);
    if (file == BundleName) {
#if !defined(QWS) && defined(Q_OS_MAC)
        QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0, QCFString(d->filePath),
                                                              kCFURLPOSIXPathStyle, true);
        QCFType<CFBundleRef> bundle = CFBundleCreate(0, url);
        if(bundle) {
            if(CFTypeRef name = CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleNameKey)) {
                if(CFGetTypeID(name) == CFStringGetTypeID())
                    return QCFString::toQString((CFStringRef)name);
            }
        }
#endif
        return QString();
    } else if (file == BaseName) {
        int slash = d->filePath.lastIndexOf(QLatin1Char('/'));
        if (slash != -1)
            return d->filePath.mid(slash + 1);
    } else if (file == PathName) {
        int slash = d->filePath.lastIndexOf(QLatin1Char('/'));
        if (slash == -1)
            return QLatin1String(".");
        else if (!slash)
            return QLatin1String("/");
        return d->filePath.left(slash);
    } else if (file == AbsoluteName || file == AbsolutePathName) {
        QString ret;
        if (d->filePath.isEmpty() || !d->filePath.startsWith(QLatin1Char('/')))
            ret = QDir::currentPath();
        if (!d->filePath.isEmpty() && d->filePath != QLatin1String(".")) {
            if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
                ret += QLatin1Char('/');
            ret += d->filePath;
        }
        if (ret == QLatin1String("/"))
            return ret;
        bool isDir = ret.endsWith(QLatin1Char('/'));
        ret = QDir::cleanPath(ret);
        if (isDir)
            ret += QLatin1String("/");
        if (file == AbsolutePathName) {
            int slash = ret.lastIndexOf(QLatin1Char('/'));
            if (slash == -1)
                return QDir::currentPath();
            else if (!slash)
                return QLatin1String("/");
            return ret.left(slash);
        }
        return ret;
    } else if (file == CanonicalName || file == CanonicalPathName) {
#if defined(__GLIBC__) && !defined(PATH_MAX)
      char *cur = ::get_current_dir_name();
      if (cur) {
	QString ret;
	char *tmp = ::canonicalize_file_name(d->nativeFilePath.constData());
	if (tmp) {
	  ret = QFile::decodeName(tmp);
	  ::free(tmp);
	}
	::chdir(cur); // always make sure we go back to the current dir
	::free(cur);
#else
        char cur[PATH_MAX+1];
        if (::getcwd(cur, PATH_MAX)) {
            QString ret;
#  ifndef Q_OS_INTEGRITY // INTEGRITY has no realpath
            char real[PATH_MAX+1];
            // need the cast for old solaris versions of realpath that doesn't take
            // a const char*.
            if (::realpath(d->nativeFilePath.constData(), real))
                ret = QFile::decodeName(QByteArray(real));
#  endif
            ::chdir(cur); // always make sure we go back to the current dir
#endif
            //check it
            QT_STATBUF st;
            if (QT_STAT(QFile::encodeName(ret), &st) != 0)
                ret = QString();
            if (!ret.isEmpty() && file == CanonicalPathName) {
                int slash = ret.lastIndexOf(QLatin1Char('/'));
                if (slash == -1)
                    return QDir::currentPath();
                else if (!slash)
                    return QLatin1String("/");
                return ret.left(slash);
            }
            return ret;
        }
        if (file == CanonicalPathName)
            return fileName(AbsolutePathName);
        return fileName(AbsoluteName);
    } else if (file == LinkName) {
        if (d->isSymlink()) {
#if defined(__GLIBC__) && !defined(PATH_MAX)
#define PATH_CHUNK_SIZE 256
            char *s = 0;
            int len = -1;
            int size = PATH_CHUNK_SIZE;

            while (1) {
                s = (char *) ::realloc(s, size);
                if (s == 0) {
                    len = -1;
                    break;
                }
                len = ::readlink(d->nativeFilePath.constData(), s, size);
                if (len < 0) {
                    ::free(s);
                    break;
                }
                if (len < size) {
                    break;
                }
                size *= 2;
            }
#else
            char s[PATH_MAX+1];
            int len = readlink(d->nativeFilePath.constData(), s, PATH_MAX);
#endif
            if (len > 0) {
                QString ret;
                if (S_ISDIR(d->st.st_mode) && s[0] != '/') {
                    QDir parent(d->filePath);
                    parent.cdUp();
                    ret = parent.path();
                    if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
                        ret += QLatin1Char('/');
                }
                s[len] = '\0';
                ret += QFile::decodeName(QByteArray(s));
#if defined(__GLIBC__) && !defined(PATH_MAX)
		::free(s);
#endif

                if (!ret.startsWith(QLatin1Char('/'))) {
                    if (d->filePath.startsWith(QLatin1Char('/'))) {
                        ret.prepend(d->filePath.left(d->filePath.lastIndexOf(QLatin1Char('/')))
                                    + QLatin1Char('/'));
                    } else {
                        ret.prepend(QDir::currentPath() + QLatin1Char('/'));
                    }
                }
                ret = QDir::cleanPath(ret);
                if (ret.size() > 1 && ret.endsWith(QLatin1Char('/')))
                    ret.chop(1);
                return ret;
            }
        }
#if !defined(QWS) && defined(Q_OS_MAC)
        {
            FSRef fref;
            if (FSPathMakeRef((const UInt8 *)QFile::encodeName(QDir::cleanPath(d->filePath)).data(), &fref, 0) == noErr) {
                Boolean isAlias, isFolder;
                if (FSResolveAliasFile(&fref, true, &isFolder, &isAlias) == noErr && isAlias) {
                    AliasHandle alias;
                    if (FSNewAlias(0, &fref, &alias) == noErr && alias) {
                        CFStringRef cfstr;
                        if (FSCopyAliasInfo(alias, 0, 0, &cfstr, 0, 0) == noErr)
                            return QCFString::toQString(cfstr);
                    }
                }
            }
        }
#endif
        return QString();
    }
    return d->filePath;
}

bool QFSFileEngine::isRelativePath() const
{
    Q_D(const QFSFileEngine);
    int len = d->filePath.length();
    if (len == 0)
        return true;
    return d->filePath[0] != QLatin1Char('/');
}

uint QFSFileEngine::ownerId(FileOwner own) const
{
    Q_D(const QFSFileEngine);
    static const uint nobodyID = (uint) -2;
    if (d->doStat()) {
        if (own == OwnerUser)
            return d->st.st_uid;
        else
            return d->st.st_gid;
    }
    return nobodyID;
}

QString QFSFileEngine::owner(FileOwner own) const
{
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
    QVarLengthArray<char, 1024> buf(sysconf(_SC_GETPW_R_SIZE_MAX));
#endif

    if (own == OwnerUser) {
        struct passwd *pw = 0;
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
        struct passwd entry;
        getpwuid_r(ownerId(own), &entry, buf.data(), buf.size(), &pw);
#else
        pw = getpwuid(ownerId(own));
#endif
        if (pw)
            return QFile::decodeName(QByteArray(pw->pw_name));
    } else if (own == OwnerGroup) {
        struct group *gr = 0;
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
        buf.resize(sysconf(_SC_GETGR_R_SIZE_MAX));
        struct group entry;
        getgrgid_r(ownerId(own), &entry, buf.data(), buf.size(), &gr);
#else
        gr = getgrgid(ownerId(own));
#endif
        if (gr)
            return QFile::decodeName(QByteArray(gr->gr_name));
    }
    return QString();
}

bool QFSFileEngine::setPermissions(uint perms)
{
    Q_D(QFSFileEngine);
    mode_t mode = 0;
    if (perms & ReadOwnerPerm)
        mode |= S_IRUSR;
    if (perms & WriteOwnerPerm)
        mode |= S_IWUSR;
    if (perms & ExeOwnerPerm)
        mode |= S_IXUSR;
    if (perms & ReadUserPerm)
        mode |= S_IRUSR;
    if (perms & WriteUserPerm)
        mode |= S_IWUSR;
    if (perms & ExeUserPerm)
        mode |= S_IXUSR;
    if (perms & ReadGroupPerm)
        mode |= S_IRGRP;
    if (perms & WriteGroupPerm)
        mode |= S_IWGRP;
    if (perms & ExeGroupPerm)
        mode |= S_IXGRP;
    if (perms & ReadOtherPerm)
        mode |= S_IROTH;
    if (perms & WriteOtherPerm)
        mode |= S_IWOTH;
    if (perms & ExeOtherPerm)
        mode |= S_IXOTH;
    if (d->fd != -1)
        return !fchmod(d->fd, mode);
    return !::chmod(d->nativeFilePath.constData(), mode);
}

bool QFSFileEngine::setSize(qint64 size)
{
    Q_D(QFSFileEngine);
    if (d->fd != -1)
        return !QT_FTRUNCATE(d->fd, size);
    return !QT_TRUNCATE(d->nativeFilePath.constData(), size);
}

QDateTime QFSFileEngine::fileTime(FileTime time) const
{
    Q_D(const QFSFileEngine);
    QDateTime ret;
    if (d->doStat()) {
        if (time == CreationTime)
            ret.setTime_t(d->st.st_ctime ? d->st.st_ctime : d->st.st_mtime);
        else if (time == ModificationTime)
            ret.setTime_t(d->st.st_mtime);
        else if (time == AccessTime)
            ret.setTime_t(d->st.st_atime);
    }
    return ret;
}
