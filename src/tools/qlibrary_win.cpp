/****************************************************************************
** $Id$
**
** Implementation of QLibraryPrivate class for Win32
**
** Created : 2000-01-01
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
    QMutexLocker locker( qt_global_mutexpool->get( &map ) );
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
	    qSystemWarning( QString("Failed to load library %1!").arg( filename ) );
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
    QMutexLocker locker( qt_global_mutexpool->get( &map ) );
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
	    } else
		ok = TRUE;
	    break;
	}
    }
    if ( ok )
	pHnd = 0;
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    else
	qSystemWarning( "Failed to unload library!" );
#endif
    return ok;
}

void* QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return 0;

#ifdef Q_OS_TEMP
    void* address = GetProcAddress( pHnd, f.ucs2() );
#else
    void* address = GetProcAddress( pHnd, f );
#endif
#if defined(QT_DEBUG_COMPONENT)
    if ( !address )
	qSystemWarning( QString("Couldn't resolve symbol \"%1\"").arg( f ) );
#endif

    return address;
}

#endif //QT_NO_LIBRARY