#ifndef QPLUGIN_P_H
#define QPLUGIN_P_H

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
    qSystemWarning( "Failed to load library!" );
#endif

    return handle;
}

bool qt_free_library( HINSTANCE handle )
{
    bool ok = FreeLibrary( handle );
#ifdef CHECK_RANGE
    qSystemWarning( "Failed to unload library!" );
#endif

    return ok;
}

void* qt_resolve_symbol( HINSTANCE handle, const char* f )
{
    void* address = GetProcAddress( handle, f );
#ifdef CHECK_RANGE
    if ( !address )
	qSystemWarning( "Couldn't resolve symbol" );
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
    char* error = dlerror();
    if ( error )
	qWarning( error );
#endif
    return ok != 0;
}

void* qt_resolve_symbol( void* handle, const char* f )
{
    void* address = dlsym( handle, f );
#ifdef CHECK_RANGE
    char* error = dlerror();
    if ( error )
	qWarning( error );
#endif
    return address;
}
#endif

#endif //QPLUGIN_P_H
