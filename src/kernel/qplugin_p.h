#ifndef QPLUGIN_P_H
#define QPLUGIN_P_H

#ifdef _OS_WIN32_
// Windows
#   include <qt_windows.h>
#   define QT_LOAD_LIBRARY( x ) LoadLibrary( x )
#   define QT_FREE_LIBRARY( x ) FreeLibrary( x )
#   define QT_RESOLVE_SYMBOL( x ) GetProcAddress( x )
#elif defined(_OS_LINUX_) ||defined (_OS_FREEBSD_) || defined(_OS_OPENBSD_) || defined(_OS_NETBSD_)
// Linux
// FreeBSD and OpenBSD
#   include <dlfcn.h>
#   define QT_LOAD_LIBRARY( x ) dlopen( x )
#   define QT_FREE_LIBRARY( x ) dlclose( x )
#   define QT_RESOLVE_SYMBOL( x ) dlsym( x )
#elif defined(_OS_OSF_)
// Digital UNIX
#   error "Tell qt-bugs@trolltech.com what's the header file!"
#elif defined(_OS_AIX_)
// AIX
#   error "Tell qt-bugs@trolltech.com what's the header file!"
#elif defined(_OS_HPUX_)
// HP/UX
#   error "Tell qt-bugs@trolltech.com what's the header file!"
#elif defined(_OS_SOLARIS_) || defined(_OS_SUN_)
// Solaris
#   error "Tell qt-bugs@trolltech.com what's the header file!"
#elif defined(_OS_IRIX_)
// Irix
#   error "Tell qt-bugs@trolltech.com what's the header file!"
#elif defined(_OS_MAC_)
#else
#   error "Plugin support has not been implemented for this OS - talk to qt-bugs@trolltech.com"
#endif

#endif //QPLUGIN_P_H
