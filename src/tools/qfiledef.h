/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfiledef.h#14 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Common macros and system include files for QFile, QFileInfo and QDir.
** This file is included by qfile.cpp, qfileinf.cpp and qdir.cpp
**
** Author  : Haavard Nord
** Created : 930812
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#if !defined(_OS_MAC_)
# include <sys/types.h>
# include <sys/stat.h>
#endif
#include <fcntl.h>
#include <errno.h>
#if defined(UNIX)
# include <dirent.h>
# include <unistd.h>
#endif
#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
# include <io.h>
# include <dos.h>
# include <direct.h>
# define _OS_FATFS_
#endif
#include <limits.h>


#if !defined(PATH_MAX)
#if defined( MAXPATHLEN )
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif


#undef STATBUF
#undef STAT
#undef STAT_REG
#undef STAT_DIR
#undef STAT_LNK
#undef STAT_MASK
#undef OPEN
#undef CLOSE
#undef LSEEK
#undef READ
#undef WRITE
#undef ACCESS
#undef GETCWD
#undef MKDIR
#undef RMDIR
#undef OPEN_RDONLY
#undef OPEN_WRONLY
#undef OPEN_CREAT
#undef OPEN_TRUNC
#undef OPEN_APPEND
#undef OPEN_TEXT


#if defined(_CC_MSVC_) || defined(_CC_SYM_)

# define STATBUF	struct _stat		// non-ANSI defs
# define STAT		_stat
# define STAT_REG	_S_IFREG
# define STAT_DIR	_S_IFDIR
# define STAT_MASK	_S_IFMT
# if defined(_S_IFLNK)
#  define STAT_LNK	_S_IFLNK
# endif
# define OPEN		::_open
# define CLOSE		::_close
# define LSEEK		::_lseek
# define READ		::_read
# define WRITE		::_write
# define ACCESS		::_access
# define GETCWD		::_getcwd
# define MKDIR		::_mkdir
# define RMDIR		::_rmdir
# define OPEN_RDONLY	_O_RDONLY
# define OPEN_WRONLY	_O_WRONLY
# define OPEN_RDWR	_O_RDWR
# define OPEN_CREAT	_O_CREAT
# define OPEN_TRUNC	_O_TRUNC
# define OPEN_APPEND	_O_APPEND
# if defined(O_TEXT)
#  define OPEN_TEXT	_O_TEXT
# endif

#else						// all other systems

# define STATBUF	struct stat
# if defined(UNIX)
#  define STAT		::stat
# else
#  define STAT		::stat
# endif
# define STAT_REG	S_IFREG
# define STAT_DIR	S_IFDIR
# define STAT_MASK	S_IFMT
# if defined(S_IFLNK)
#  define STAT_LNK	S_IFLNK
# endif
# define OPEN		::open
# define CLOSE		::close
# define LSEEK		::lseek
# define READ		::read
# define WRITE		::write
# define ACCESS		::access
# define GETCWD		::getcwd
# define MKDIR		::mkdir
# define RMDIR		::rmdir
# define OPEN_RDONLY	O_RDONLY
# define OPEN_WRONLY	O_WRONLY
# define OPEN_RDWR	O_RDWR
# define OPEN_CREAT	O_CREAT
# define OPEN_TRUNC	O_TRUNC
# define OPEN_APPEND	O_APPEND
# if defined(O_TEXT)
#  define OPEN_TEXT	O_TEXT
# endif
#endif


#if defined(_OS_FATFS_)
# define F_OK	0
# define X_OK	1
# define W_OK	2
# define R_OK	4
#endif
