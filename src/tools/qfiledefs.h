/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfiledefs.h#2 $
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
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#if !defined(_OS_MAC_)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <errno.h>
#if defined(UNIX)
#include <unistd.h>
#endif
#if defined(_OS_MSDOS_) || defined(_OS_WIN32_) || defined(_OS_OS2_)
#include <io.h>
#endif
#include <limits.h>

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
#undef ACCESS
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
#define LSEEK	::_lseek
#define READ	::_read
#define WRITE	::_write
#define ACCESS	::_access
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
#define ACCESS	::access
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
