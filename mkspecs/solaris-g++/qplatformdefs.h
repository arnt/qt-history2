#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

// Get Qt defines/settings
#include "qglobal.h"


// Set any POSIX/XOPEN defines at the top of this file to turn on
// specific APIs

#include <unistd.h>
#include <sys/types.h>
// We are hot - unistd.h should have turned on all the specific
// APIs we requested


#ifdef QT_THREAD_SUPPORT
#include <thread.h>
#endif // QT_THREAD_SUPPORT


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

#if !defined(_XOPEN_UNIX)
// Function usleep() is in C library but not in header files on Solaris 2.5.1.
// Not really a surprise, usleep() is specified by XPG4v2 and XPG4v2 is only
// supported by Solaris 2.6 and better.
// So we are trying to detect Solaris 2.5.1 using macro _XOPEN_UNIX which is
// not defined by <unistd.h> when XPG4v2 is not supported.
typedef unsigned int useconds_t;
extern "C" int usleep(useconds_t);
#endif

// On Solaris 7 and better, even though XPG4v2 is specified, sockets use
// socklen_t not size_t because size_t breaks 64-bit platforms and Solaris 7
// is a 64-bit platform. In the end both socklen_t and size_t are correct
// because socklen_t is typedef'ed to size_t for compatibility in 32-bit mode.
// On Solaris 2.6, XPG4v2 is specified, sockets use size_t.
// On Solaris 2.5.1, XPG4v2 is not specified, sockets use int.
#if defined(_XOPEN_UNIX)
// Solaris 2.6 and better!
#define QT_SOCKLEN_T size_t
#else
// Solaris 2.5.1.
#define QT_SOCKLEN_T int
#endif

#define QT_NREAD	I_NREAD

inline int qt_socket_accept(int s, struct sockaddr *addr, QT_SOCKLEN_T *addrlen)
{ return ::accept(s, addr, addrlen); }

inline int qt_socket_listen(int s, int backlog)
{ return ::listen(s, backlog); }


#endif // QPLATFORMDEFS_H
