/****************************************************************************
**
** Implementation of QLibraryPrivate class for Win32.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qmap.h>
#include <private/qlibrary_p.h>

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#ifndef QT_H
#include "qfile.h"
#endif // QT_H

#ifndef QT_NO_LIBRARY

struct LibInstance {
    LibInstance() { instance = 0; refCount = 0; }
    HINSTANCE instance;
    int refCount;
};

static QMap<QString, LibInstance*> *map = 0;
/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/

#include "qt_windows.h"

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect map creation/access
    QMutexLocker locker( qt_global_mutexpool ?
			 qt_global_mutexpool->get( &map ) : 0 );
#endif // QT_THREAD_SUPPORT

    if ( !map )
	map = new QMap<QString, LibInstance*>;

    QString filename = library->library();
    if ( map->find(filename) != map->end() ) {
	LibInstance *lib = (*map)[filename];
	lib->refCount++;
	pHnd = lib->instance;
    }
    else {
	QT_WA( {
	    pHnd = LoadLibraryW( (TCHAR*)filename.ucs2() );
	} , {
	    pHnd = LoadLibraryA(QFile::encodeName( filename ).data());
	} );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	if ( !pHnd )
	    qSystemWarning("Failed to load library %s", QFile::encodeName(filename).data());
#endif
	if ( pHnd ) {
	    LibInstance *lib = new LibInstance;
	    lib->instance = pHnd;
	    lib->refCount++;
	    map->insert( filename, lib );
	}
    }
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect map access
    QMutexLocker locker( qt_global_mutexpool ?
			 qt_global_mutexpool->get( &map ) : 0 );
#endif // QT_THREAD_SUPPORT

    bool ok = FALSE;
    QMap<QString, LibInstance*>::iterator it;
    for ( it = map->begin(); it != map->end(); ++it ) {
	LibInstance *lib = *it;
	if ( lib->instance == pHnd ) {
	    lib->refCount--;
	    if ( lib->refCount == 0 ) {
		ok = FreeLibrary( pHnd );
		if ( ok ) {
		    map->remove( it );
		    if ( map->count() == 0 ) {
			delete map;
			map = 0;
		    }
		}
		delete lib;
	    } else
		ok = TRUE;
	    break;
	}
    }
    if ( ok )
	pHnd = 0;
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    else
	qSystemWarning( "Failed to unload library" );
#endif
    return ok;
}

void* QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return 0;

#ifdef Q_OS_TEMP
    void* address = (void*)GetProcAddress( pHnd, (const wchar_t*)QString(f).ucs2() );
#else
    void* address = (void*)GetProcAddress( pHnd, f );
#endif
#if defined(QT_DEBUG_COMPONENT)
    if ( !address )
	qSystemWarning("Couldn't resolve symbol \"%s\"", f);
#endif

    return address;
}

#endif //QT_NO_LIBRARY
