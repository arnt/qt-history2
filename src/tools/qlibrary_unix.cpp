/****************************************************************************
**
** Implementation of QLibraryPrivate class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "private/qlibrary_p.h"

#ifndef QT_NO_LIBRARY

#if defined(QT_AOUT_UNDERSCORE)
#include <string.h>
#endif

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/

#if defined(Q_OS_DARWIN)
#include "qt_mac.h"
# define ENUM_DYLD_BOOL
 enum DYLD_BOOL { DYLD_FALSE, DYLD_TRUE };
 extern "C" {
# include "mach-o/dyld.h"
 }
#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT
#include "qdict.h"
#include "qdir.h"
#include "qstringlist.h"

struct glibs_ref {
    QString name;
    int count;
    void *handle;
};
static QDict<glibs_ref> *glibs_loaded = 0;

#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
static NSModule qt_mac_library_multiple(NSSymbol sym, NSModule o, NSModule)
{
    qDebug("multiple definition %s", NSNameOfSymbol(sym));
    return o;
}

static void qt_mac_library_undefined(const char *symbolname)
{
    qDebug("qlibrary_mac.cpp: undefined symbol (%s)", symbolname);
    exit(666);
}

static void qt_mac_library_error(NSLinkEditErrors err, int line, const char *fileName,
				 const char *error)
{
    qDebug("qlibrary_mac.cpp: %d: %d: %s (%s)", err, line, fileName, error);
}
#endif

bool QLibraryPrivate::loadLibrary()
{
    if(pHnd)
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect glibs_loaded creation/access
    QMutexLocker locker(qt_global_mutexpool ? qt_global_mutexpool->get(&glibs_loaded) : 0);
#endif // QT_THREAD_SUPPORT

#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    static bool first = TRUE;
    if(first) { //deal with errors
	first = FALSE;
	NSLinkEditErrorHandlers hdl;
	memset(&hdl, 0, sizeof(hdl));
	hdl.undefined = qt_mac_library_undefined;
	hdl.multiple = qt_mac_library_multiple;
	hdl.linkEdit = qt_mac_library_error;
	NSInstallLinkEditErrorHandlers(&hdl);
    }
#endif

    /* Not pretty, but it looks to me like this has to be done
       if I want to look in the regular loading places (like dyld isn't
       used to find the plugin). So instead I'll just hardcode it so
       it matches dyld(1) paths */
    QStringList places(""); //just look for the filename first..
    if(const char *fallback = getenv("DYLD_FALLBACK_LIBRARY_PATH")) {
	QStringList lst = QStringList::split(':', fallback, TRUE);
	for(QStringList::Iterator it = lst.begin(); it != lst.end(); it++) {
	    QString d = (*it);
	    if(d.isEmpty())
		d = QDir::currentDirPath();
	    if(!d.endsWith(QString(QChar(QDir::separator()))))
		d += QDir::separator();
	    places << d << "qt_plugins/" << d;
	}
    } else {
	places << QDir::homeDirPath() << "/lib/qt_plugins/" << QDir::homeDirPath() << "/lib/"
	       << "/usr/local/lib/qt_plugins/" << "/usr/local/lib/"
	       << "/lib/qt_plugins/" << "/lib/"
	       << "/usr/lib/qt_plugins/" << "/usr/lib/";
    }
    if(const char *dyld_path = getenv("DYLD_LIBRARY_PATH")) {
	QStringList lst = QStringList::split(':', dyld_path, TRUE);
	for(QStringList::Iterator it = lst.begin(); it != lst.end(); it++) {
	    QString d = (*it);
	    if(d.isEmpty())
		d = QDir::currentDirPath();
	    if(!d.endsWith(QString(QChar(QDir::separator()))))
		d += QDir::separator();
	    places << d;
	}
    }

    QString filename;
    NSObjectFileImage img;
    for(QStringList::Iterator it = places.begin(); it != places.end(); it++) {
	QString tmp = (*it);
	if(!tmp.isEmpty() && !tmp.endsWith(QString(QChar(QDir::separator()))))
		tmp += QDir::separator();
	tmp += library->library();
	if(!glibs_loaded) {
	    glibs_loaded = new QDict<glibs_ref>();
	} else if(glibs_ref *i = glibs_loaded->find(tmp)) {
	    i->count++;
	    pHnd = i->handle;
	    return TRUE;
	}
	if(NSCreateObjectFileImageFromFile(tmp, &img) == NSObjectFileImageSuccess) {
	    filename = tmp;
	    break;
	}
    }
    if(filename.isEmpty()) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	qDebug("Could not find %s in '%s'", library->library().latin1(), places.join("::").latin1());
#endif
	return FALSE;
    }

    if((pHnd = (void *)NSLinkModule(img, filename,
				    NSLINKMODULE_OPTION_BINDNOW|NSLINKMODULE_OPTION_PRIVATE|
				    NSLINKMODULE_OPTION_RETURN_ON_ERROR))) {
	glibs_ref *i = new glibs_ref;
	i->handle = pHnd;
	i->count = 1;
	i->name = filename;
	glibs_loaded->insert(filename, i); //insert it in the loaded hash
	return TRUE;
    }
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    NSLinkEditErrors lee;
    int err;
    const char *f, *msg;
    NSLinkEditError(&lee, &err, &f, &msg);
    qDebug("Failed to load library '%s' (%d:%d):\n  %s",  filename.latin1(), lee, err, msg);
#endif
    pHnd = NULL;
    return FALSE;
}

bool QLibraryPrivate::freeLibrary()
{
    if(!pHnd)
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect glibs_loaded access
    QMutexLocker locker(qt_global_mutexpool ? qt_global_mutexpool->get(&glibs_loaded) : 0);
#endif // QT_THREAD_SUPPORT

    if(glibs_loaded) {
	for(QDictIterator<glibs_ref> it(*glibs_loaded); it.current(); ++it) {
	    if(it.current()->handle == pHnd && !(--it.current()->count)) {
		glibs_loaded->remove(it.currentKey());

		NSUnLinkModule(pHnd,
			       NSUNLINKMODULE_OPTION_KEEP_MEMORY_MAPPED|
			       NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES);

		break;
	    }
	}
    }
    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol(const char *symbol)
{
    if(!pHnd)
	return 0;
    void *ret = NULL;
    QString symn2;
    symn2.sprintf("_%s", symbol);
    ret = NSAddressOfSymbol(NSLookupSymbolInModule(pHnd, symn2.latin1()));
#if defined(QT_DEBUG_COMPONENT)
    if(!ret)
	qDebug("Couldn't resolve symbol \"%s\"", symbol);
#endif
    return ret;
}

#elif defined(QT_HPUX_LD) // for HP-UX < 11.x and 32 bit

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();

    pHnd = (void*)shl_load( filename.latin1(), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0 );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qWarning( "%s: failed to load library!", filename.latin1() );
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

    if ( shl_unload( (shl_t)pHnd ) ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	QString filename = library->library();
	qWarning( "%s: Failed to unload library!", filename.latin1() );
#endif
	return FALSE;
    }
    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol( const char* symbol )
{
    if ( !pHnd )
	return 0;

    void* address = 0;
    if ( shl_findsym( (shl_t*)&pHnd, symbol, TYPE_UNDEFINED, &address ) < 0 ) {
#if defined(QT_DEBUG_COMPONENT)
	QString filename = library->library();
	qWarning( "%s: couldn't resolve symbol \"%s\"", filename.latin1(), symbol );
#endif
    }
    return address;
}

#else // POSIX
#include <dlfcn.h>

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();

    pHnd = dlopen( filename.latin1(), RTLD_LAZY );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qWarning( "%s", dlerror() );
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

    if ( dlclose( pHnd ) ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	qWarning( "%s", dlerror() );
#endif
	return FALSE;
    }

    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol( const char* symbol )
{
    if ( !pHnd )
	return 0;

#if defined(QT_AOUT_UNDERSCORE)
    // older a.out systems add an underscore in front of symbols
    char* undrscr_symbol = new char[strlen(symbol)+2];
    undrscr_symbol[0] = '_';
    strcpy(undrscr_symbol+1, symbol);
    void* address = dlsym( pHnd, undrscr_symbol );
    delete [] undrscr_symbol;
#else
    void* address = dlsym( pHnd, symbol );
#endif
#if defined(QT_DEBUG_COMPONENT)
    const char* error = dlerror();
    if ( error )
	qWarning( "%s", error );
#endif
    return address;
}

#endif // POSIX

#endif
