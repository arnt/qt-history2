#ifndef QPLUGIN_P_H
#define QPLUGIN_P_H

#ifdef _OS_WIN32_
// Windows
#   include <qt_windows.h>
#   include "qapplication_p.h"

HINSTANCE qt_load_library( const QString& lib )
{
    if ( qt_winver & Qt::WV_NT_based )
	return LoadLibraryW( (TCHAR*)qt_winTchar(lib, TRUE) );
    return LoadLibraryA( (const char*)lib.local8Bit() );
}

bool qt_free_library( HINSTANCE handle )
{
    return FreeLibrary( handle );
}

void* qt_resolve_symbol( HINSTANCE handle, const char* f )
{
    return GetProcAddress( handle, f );
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
    return shl_unload( x );
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
    return dlopen( lib );
}

bool qt_free_library( void* handle )
{
    return dlclose( handle );
}

void* qt_resolve_symbol( void* handle, const char* f )
{
    return dlsym( h, f );
}
#endif

#endif //QPLUGIN_P_H
