#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

// Get Qt defines/settings
#include "qglobal.h"


#include <unistd.h>
#include <sys/types.h>
// We are hot - unistd.h should have turned on all the specific
// APIs we requested


#ifdef QT_THREAD_SUPPORT
#  include <pthread.h>
#endif


#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stropts.h>
#include <time.h>

#include <netinet/in.h>

#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>


#define QT_STATBUF		struct stat
#define QT_STATBUF4TSTAT	struct stat
#define QT_STAT			::stat
#define QT_FSTAT		::fstat
#define QT_STAT_REG		S_IFREG
#define QT_STAT_DIR		S_IFDIR
#define QT_STAT_MASK		S_IFMT
#define QT_STAT_LNK		S_IFLNK
#define QT_FILENO		fileno
#define QT_OPEN			::open
#define QT_CLOSE		::close
#define QT_LSEEK		::lseek
#define QT_READ			::read
#define QT_WRITE		::write
#define QT_ACCESS		::access
#define QT_GETCWD		::getcwd
#define QT_CHDIR		::chdir
#define QT_MKDIR		::mkdir
#define QT_RMDIR		::rmdir
#define QT_OPEN_RDONLY		O_RDONLY
#define QT_OPEN_WRONLY		O_WRONLY
#define QT_OPEN_RDWR		O_RDWR
#define QT_OPEN_CREAT		O_CREAT
#define QT_OPEN_TRUNC		O_TRUNC
#define QT_OPEN_APPEND		O_APPEND

#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

// on Reliant 5.45 and if __LP64__ is not defined:
// - socket functions use 'size_t'
// - if _SOCKLEN_T is defined 'socklen_t' is typedef'ed to 'size_t' 
#if defined(_XOPEN_SOURCE_EXTENDED) || (_XOPEN_SOURCE-0 >= 500)
#  define QT_SOCKLEN_T socklen_t
#else
#  define QT_SOCKLEN_T size_t
#endif

#define QT_NREAD	I_NREAD

inline int qt_socket_accept(int s, struct sockaddr *addr, QT_SOCKLEN_T *addrlen)
{ return ::accept(s, addr, addrlen); }

inline int qt_socket_listen(int s, int backlog)
{ return ::listen(s, backlog); }


#endif // QPLATFORMDEFS_H
