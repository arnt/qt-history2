/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinf.cpp#1 $
**
** Implementation of QFile class
**
** Author  : Haavard Nord
** Created : 950628
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfileinf.h"
#include "qdatetm.h"
#include "qdir.h"

#if !defined(_OS_MAC_)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <errno.h>
#if defined(UNIX)
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#endif
#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
#include <io.h>
#endif
#include <limits.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qfileinf.cpp#1 $";
#endif


 // The following macros were copied from qfile.cpp

#undef STATBUF
#undef STAT
#undef STAT_REG
#undef STAT_DIR
#undef STAT_LNK
#undef OPEN
#undef CLOSE
#undef LSEEK
#undef READ
#undef WRITE
#undef OPEN_RDONLY
#undef OPEN_WRONLY
#undef OPEN_CREAT
#undef OPEN_TRUNC
#undef OPEN_APPEND
#undef OPEN_TEXT

#if defined(_CC_MSC_) || defined(_CC_SYM_)
#define STATBUF	 _stat				// non-ANSI defs
#define STAT	 _stat
#define STAT_REG _S_IFREG
#define STAT_DIR _S_IFDIR
#if defined(_S_IFLNK)
#define STAT_LNK _S_IFLNK
#endif
#define OPEN	::_open
#define CLOSE	::_close
#define LSEEK	_lseek
#define READ	_read
#define WRITE	_write
#define OPEN_RDONLY	_O_RDONLY
#define OPEN_WRONLY	_O_WRONLY
#define OPEN_RDWR	_O_RDWR
#define OPEN_CREAT	_O_CREAT
#define OPEN_TRUNC	_O_TRUNC
#define OPEN_APPEND	_O_APPEND
#define OPEN_TEXT	_O_TEXT

#else						// all other systems

#define STATBUF	 stat
#define STAT	 ::stat
#define STAT_REG S_IFREG
#define STAT_DIR S_IFDIR
#if defined(S_IFLNK)
#define STAT_LNK S_IFLNK
#endif
#define OPEN	::open
#define CLOSE	::close
#define LSEEK	::lseek
#define READ	::read
#define WRITE	::write
#define OPEN_RDONLY	O_RDONLY
#define OPEN_WRONLY	O_WRONLY
#define OPEN_RDWR	O_RDWR
#define OPEN_CREAT	O_CREAT
#define OPEN_TRUNC	O_TRUNC
#define OPEN_APPEND	O_APPEND
#if defined(O_TEXT)
#define OPEN_TEXT	O_TEXT
#endif
#endif

static bool getStat( const char *fn, struct STATBUF *st )
{
#if defined(_OS_MAC_)
    return FALSE;
#else
    if ( !fn )				// no filename specified
	return FALSE;
    return STAT( fn, st )==0 ? TRUE : FALSE;
#endif
}


/*! \class QFileInfo qfileinf.h

  \brief The QFile class provides system-independent file information and
  related functions.

  \ingroup tools

  This class is not yet documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */


QFileInfo::QFileInfo()
{
}

QFileInfo::QFileInfo( const  QFileInfo &fi )
{
    f = fi.f;
}

QFileInfo::QFileInfo( const  QFile &file )
{
    f = file;
}

QFileInfo::QFileInfo( const  QDir &d, const char *fileName )
{
    f.setFileName( d, fileName );
}

QFileInfo::QFileInfo( const char *fullPathFileName )
{
    f.setFileName( fullPathFileName );
}

QFileInfo::~QFileInfo()
{
}

void QFileInfo::setFile( const char *fullPathFileName )
{
    f.setFileName( fullPathFileName );
}

void QFileInfo::setFile( const QDir &d, const char *fileName )
{
    f.setFileName( d, fileName );
}

const char *QFileInfo::fullPathFileName() const
{
    return f.fileName();
}

bool QFileInfo::exists() const
{
    return f.exists();
}

bool QFileInfo::isReadable() const
{
    if ( f.fileName() )
        return ( access( f.fileName(), R_OK ) == 0 );
    else
        return FALSE;
}

bool QFileInfo::isWritable() const
{
    if ( f.fileName() )
        return ( access( f.fileName(), W_OK ) == 0 );
    else
        return FALSE;
}

bool QFileInfo::isExecutable() const
{
    if ( f.fileName() )
        return ( access( f.fileName(), X_OK ) == 0 );
    else
        return FALSE;

}

bool QFileInfo::isFile() const
{
    return f.isFile();
}

bool QFileInfo::isDir() const
{
    return f.isDir();
}

bool QFileInfo::isSymLink() const
{
    return f.isSymLink();
}

const char *QFileInfo::owner() const
{
#if defined(UNIX)
    const passwd *tmp = getpwuid( ownerId() );
    return tmp ? tmp->pw_name : 0;
#else
    return 0;
#endif
}

// Advanced programming in the UNIX environment, page 146:

static const uint nobodyID = 65534;   

uint QFileInfo:: ownerId() const
{
#if defined(UNIX)
    struct STATBUF st;
    if ( getStat( f.fileName(), &st ) )
        return st.st_uid;
#endif
    return nobodyID;
}

const char *QFileInfo::group() const
{
#if defined(UNIX)
    struct group *gr = getgrgid( groupId() );
    return gr ? gr->gr_name : 0;
#else
    return 0;
#endif
}

uint QFileInfo:: groupId() const
{
#if defined(UNIX)
    struct STATBUF st;
    if ( getStat( f.fileName(), &st ) )
        return st.st_gid;
#endif
    return nobodyID;
}

bool QFileInfo::permission( int permissionSpec ) const
{
#if defined(UNIX)
    struct STATBUF st;
    if ( getStat( f.fileName(), &st ) ) {
        ulong mask = 0;
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
           return ( st.st_mode & mask) == mask;
        } else {
#if defined(CHECK_NULL)
           warning("QFileInfo::permission: permissionSpec is 0.");
#endif
           return TRUE;
	}
    } else {
        return FALSE;
    }
#else
    return TRUE;
#endif
}

long QFileInfo::size() const
{
    struct STATBUF st;
    if ( getStat( f.fileName(), &st ) )
        return st.st_size;
    else
        return -1;
}

QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;
    struct STATBUF st;

    if ( getStat( f.fileName(), &st ) )
        dt.setTime_t( st.st_mtime );
    return dt;
}

QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;
    struct STATBUF st;

    if ( getStat( f.fileName(), &st ) )
        dt.setTime_t( st.st_atime );
    return dt;
}




