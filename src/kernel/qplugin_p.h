/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qplugin_p.h#1 $
**
** Definition of some Qt private functions.
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

#ifndef QPLUGIN_P_H
#define QPLUGIN_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qplugin.cpp.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//


#ifndef QT_H
#include "qstring.h" // char*->QString conversion
#endif // QT_H

#ifdef _OS_WIN32_
// Windows
#   include "qt_windows.h"
#   include "qapplication_p.h"

extern void qSystemWarning( const QString& messsage );

HINSTANCE qt_load_library( const QString& lib )
{
    HINSTANCE handle;
    if ( qt_winver & Qt::WV_NT_based )
	handle = LoadLibraryW( (TCHAR*)qt_winTchar(lib, TRUE) );
    else
	handle = LoadLibraryA( (const char*)lib.local8Bit() );
#ifdef CHECK_RANGE
    if ( !handle )
	qSystemWarning( "Failed to load library!" );
#endif

    return handle;
}

bool qt_free_library( HINSTANCE handle )
{
    bool ok = FreeLibrary( handle );
#ifdef CHECK_RANGE
    if ( !ok )
	qSystemWarning( "Failed to unload library!" );
#endif

    return ok;
}

void* qt_resolve_symbol( HINSTANCE handle, const char* f )
{
    void* address = GetProcAddress( handle, f );
#ifdef CHECK_RANGE
    if ( !address )
	qSystemWarning( QString("Couldn't resolve symbol \"%1\"").arg( f ) );
#endif

    return address;
}

#elif defined(_OS_HPUX_)
// for HP-UX < 11.x and 32 bit
#   include <dl.h>

void* qt_load_library( const QString& lib )
{
    shl_load( lib, BIND_IMMEDIATE | BIND_NONFATAL | DYNAMIC_PATH, 0 );
}

bool qt_free_library( void* handle )
{
    return shl_unload( handle );
}

void* qt_resolve_symbol( const QString& symbol, void* handle )
{
    void* address;
    if ( !shl_findsym( symbol, handle, TYPE_UNDFINED, address ) )
	return 0;
    return address;
}

#elif defined(_OS_MAC_)
// Mac
void* qt_load_library( const QString& lib )
{
    qWarning( "Tell vohi@trolltech.com what dl-loader implementation to use!" );
    return 0;
}

bool qt_free_library( void* handle )
{
    return FALSE;
}

void* qt_resolve_symbol( void* handle, const char* f )
{
    return 0;
}

#else
// Something else, assuming POSIX
#   include <dlfcn.h>

void* qt_load_library( const QString& lib )
{
    void* handle = dlopen( lib, RTLD_LAZY );
#ifdef CHECK_RANGE
    if ( !handle )
	qWarning( dlerror() );
#endif
    return handle;
}

bool qt_free_library( void* handle )
{
    int ok = dlclose( handle );
#ifdef CHECK_RANGE
    const char* error = dlerror();
    if ( error )
	qWarning( error );
#endif
    return ok != 0;
}

void* qt_resolve_symbol( void* handle, const char* f )
{
    void* address = dlsym( handle, f );
#ifdef CHECK_RANGE
    const char* error = dlerror();
    if ( error )
	qWarning( error );
#endif
    return address;
}
#endif

#endif //QPLUGIN_P_H
