/****************************************************************************
** $Id: $
**
** Implementation of QLibraryPrivate class
**
** Created : 2000-01-01
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QT_NO_COMPONENT

#include "private/qlibrary_p.h"

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/
#if defined(Q_OS_MACX)

#define DO_MAC_LIBRARY
#include "qdict.h"

#ifdef DO_MAC_LIBRARY
//is this gross or what!?! God I love the preprocessor..
#define OLD_T TRUE
#define OLD_F FALSE
#undef TRUE
#define TRUE DYLD_TRUE
#undef FALSE
#define FALSE DYLD_FALSE
#define ENUM_DYLD_BOOL
enum DYLD_BOOL { DYLD_TRUE=1, DYLD_FALSE=0 };
extern "C" {
#include "mach-o/dyld.h"
}
#undef bool
#undef TRUE
#define TRUE OLD_T
#undef FALSE
#define FALSE OLD_F
#endif

#ifdef DO_MAC_LIBRARY
struct glibs_ref {
    QString name;
    int count;
    void *handle;
};
static QDict<glibs_ref> *glibs_loaded = 0;
#endif

#ifdef DO_MAC_LIBRARY
static NSModule qt_mac_library_multiple(NSSymbol sym, NSModule o, NSModule)
{
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    qDebug("multiple definition %s", NSNameOfSymbol(sym));
#endif
    return o;
}

static void qt_mac_library_undefined(const char *symbolname)
{
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    qDebug("qlibrary_mac.cpp: undefined symbol (%s)", symbolname);
#endif
}

static void qt_mac_library_error(NSLinkEditErrors err, int line, const char *fileName, 
				 const char *error)
{
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    qDebug("qlibrary_mac.cpp: %d: %d: %s (%s)", err, line, fileName, error);
#endif
}
#endif

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

#ifdef DO_MAC_LIBRARY
    static bool first = TRUE;
    if(first) { //deal with errors
	first = FALSE;
	NSLinkEditErrorHandlers hdl;
	hdl.undefined = qt_mac_library_undefined;
	hdl.multiple = qt_mac_library_multiple;
	hdl.linkEdit = qt_mac_library_error;
	NSInstallLinkEditErrorHandlers(&hdl);
    }

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
#endif
    pHnd = NULL;
    return FALSE;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

#ifdef DO_MAC_LIBRARY
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
#endif
    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol( const char *symbol )
{
    if ( !pHnd )
	return 0;
    void *ret = NULL;
#ifdef DO_MAC_LIBRARY
    QCString symn2;
    symn2.sprintf("_%s", symbol);
    ret = NSAddressOfSymbol(NSLookupSymbolInModule(pHnd, symn2));
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if(!ret)
	qDebug( "Couldn't resolve symbol \"%s\"", symbol );
#endif
#endif
    return ret;
}

#elif defined(Q_OS_MAC9)

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

#endif // QT_NO_COMPONENT
