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
#include "mach-o/dyld.h"
#undef bool
#undef TRUE
#define TRUE OLD_T
#undef FALSE
#define FALSE OLD_F
#endif

static QDict<void> *glibs_loaded = 0;

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();
    if ( filename.find( ".dylib" ) == -1 )
	filename += ".dylib";

    if(!glibs_loaded)
	glibs_loaded = new QDict<void>();
    else if( ( pHnd = glibs_loaded->find( filename ) ))
	return TRUE;

#ifdef DO_MAC_LIBRARY
    NSObjectFileImage img;
    if( NSCreateObjectFileImageFromFile( filename, &img)  != NSObjectFileImageSuccess )
	return FALSE;

    if((pHnd = (void *)NSLinkModule(img, filename, NSLINKMODULE_OPTION_PRIVATE)))
	glibs_loaded->insert( filename, pHnd ); //insert it in the loaded hash
    return TRUE;
#else
    return FALSE;
#endif
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

    if(glibs_loaded) {
	for(QDictIterator<void> it(*glibs_loaded); it.current(); ++it) {
	    if( it.current() == pHnd) {
		glibs_loaded->remove(it.currentKey());
		break;
	    }
	}
    }
#ifdef DO_MAC_LIBRARY
    NSUnLinkModule(pHnd, FALSE);
    pHnd = 0;
    return TRUE;
#else
    return FALSE;
#endif
}

void* QLibraryPrivate::resolveSymbol( const char *symbol )
{
    if ( !pHnd )
	return 0;

#ifdef DO_MAC_LIBRARY
    QCString symn2;
    symn2.sprintf("_%s", symbol);
    return NSAddressOfSymbol(NSLookupSymbolInModule(pHnd, symn2));
#else
    return 0;
#endif
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
