/****************************************************************************
** $Id$
**
** Implementation of QLibraryPrivate class
**
** Created : 2000-01-01
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
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

#include "private/qlibrary_p.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/
#if defined(Q_OS_MACX)

#include "qt_mac.h"
#include "qdict.h"

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
    if ( pHnd )
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect glibs_loaded creation/access
    QMutexLocker locker( qt_global_mutexpool->get( &glibs_loaded ) );
#endif // QT_THREAD_SUPPORT

#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    static bool first = TRUE;
    if(first) { //deal with errors
	first = FALSE;
	NSLinkEditErrorHandlers hdl;
	hdl.undefined = qt_mac_library_undefined;
	hdl.multiple = qt_mac_library_multiple;
	hdl.linkEdit = qt_mac_library_error;
	NSInstallLinkEditErrorHandlers(&hdl);
    }
#endif

    QString filename = library->library();
    if(!glibs_loaded) {
	glibs_loaded = new QDict<glibs_ref>();
    } else if(glibs_ref *i = glibs_loaded->find( filename )) {
	i->count++;
	pHnd = i->handle;
	return TRUE;
    }
    NSObjectFileImage img;
    if( NSCreateObjectFileImageFromFile( filename, &img)  != NSObjectFileImageSuccess )
	return FALSE;
    if((pHnd = (void *)NSLinkModule(img, filename,
				    NSLINKMODULE_OPTION_PRIVATE|NSLINKMODULE_OPTION_RETURN_ON_ERROR))) {
	glibs_ref *i = new glibs_ref;
	i->handle = pHnd;
	i->count = 1;
	i->name = filename;
	glibs_loaded->insert( filename, i ); //insert it in the loaded hash
	return TRUE;
    }
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    qDebug( "Failed to load library %s!", filename.latin1() );
#endif
    pHnd = NULL;
    return FALSE;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect glibs_loaded access
    QMutexLocker locker( qt_global_mutexpool->get( &glibs_loaded ) );
#endif // QT_THREAD_SUPPORT

    if(glibs_loaded) {
	for(QDictIterator<glibs_ref> it(*glibs_loaded); it.current(); ++it) {
	    if( it.current()->handle == pHnd && !(--it.current()->count)) {
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

void* QLibraryPrivate::resolveSymbol( const char *symbol )
{
    if ( !pHnd )
	return 0;
    void *ret = NULL;
    QCString symn2;
    symn2.sprintf("_%s", symbol);
    ret = NSAddressOfSymbol(NSLookupSymbolInModule(pHnd, symn2));
#if defined(QT_DEBUG_COMPONENT)
    if(!ret)
	qDebug( "Couldn't resolve symbol \"%s\"", symbol );
#endif
    return ret;
}

#else

bool QLibraryPrivate::loadLibrary()
{
    return FALSE;
}

bool QLibraryPrivate::freeLibrary()
{
    return FALSE;
}

void* QLibraryPrivate::resolveSymbol( const char *symbol )
{
    return 0;
}

#endif // Q_OS_MAC9
