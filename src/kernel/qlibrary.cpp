/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlibrary.cpp#1 $
**
** Implementation of QLibrary class
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

#include "qcomponentinterface.h"
#ifndef QT_NO_COMPONENT
#include "qlibrary.h"
//#define QT_DEBUG_COMPONENT

#ifndef QT_H
#include "qstring.h" // char*->QString conversion
#endif // QT_H

#ifdef Q_OS_WIN32
// Windows
#include "qt_windows.h"
#include "qapplication_p.h"

extern void qSystemWarning( const QString& messsage );

static HINSTANCE qt_load_library( const QString& lib )
{
    HINSTANCE handle;
    if ( qt_winver & Qt::WV_NT_based )
	handle = LoadLibraryW( (TCHAR*)qt_winTchar(lib, TRUE) );
    else
	handle = LoadLibraryA( (const char*)lib.local8Bit() );
#if defined(QT_DEBUG)
    if ( !handle )
	qSystemWarning( "Failed to load library!" );
#endif

    return handle;
}

static bool qt_free_library( HINSTANCE handle )
{
    bool ok = FreeLibrary( handle );
#if defined(QT_DEBUG)
    if ( !ok )
	qSystemWarning( "Failed to unload library!" );
#endif

    return ok;
}

static void* qt_resolve_symbol( HINSTANCE handle, const char* f )
{
    void* address = GetProcAddress( handle, f );
#if defined(QT_DEBUG)
    if ( !address )
	qSystemWarning( QString("Couldn't resolve symbol \"%1\"").arg( f ) );
#endif

    return address;
}

#elif defined(Q_OS_HPUX)
// for HP-UX < 11.x and 32 bit
#include <dl.h>

static void* qt_load_library( const QString& lib )
{
    shl_load( lib, BIND_IMMEDIATE | BIND_NONFATAL | DYNAMIC_PATH, 0 );
}

static bool qt_free_library( void* handle )
{
    return shl_unload( handle );
}

static void* qt_resolve_symbol( const QString& symbol, void* handle )
{
    void* address;
    if ( !shl_findsym( symbol, handle, TYPE_UNDFINED, address ) )
	return 0;
    return address;
}

#elif defined(Q_OS_MACX)

#define DO_MAC_LIBRARY

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

// Mac
static void* qt_load_library( const QString &file )
{
#ifdef DO_MAC_LIBRARY
    NSObjectFileImage img;
    if( NSCreateObjectFileImageFromFile(file, &img)  != NSObjectFileImageSuccess )
	return NULL;
    return NSLinkModule(img, file, TRUE);
#else
    return NULL;
#endif
}

static bool qt_free_library( void *handle )
{
#ifdef DO_MAC_LIBRARY
    NSUnLinkModule(handle, FALSE);
    return TRUE;
#else
    return FALSE;
#endif
}

static void* qt_resolve_symbol( void *, const char *symbol)
{
#ifdef DO_MAC_LIBRARY
    QCString symn2;
    symn2.sprintf("_%s", symbol);
    return NSAddressOfSymbol(NSLookupAndBindSymbol(symn2));
#else
    return NULL;
#endif
}

#else
// Something else, assuming POSIX
#include <dlfcn.h>

static void* qt_load_library( const QString& lib )
{
    void* handle = dlopen( lib, RTLD_LAZY );
#if defined(QT_DEBUG)
    if ( !handle )
	qWarning( dlerror() );
#endif
    return handle;
}

static bool qt_free_library( void* handle )
{
    int ok = dlclose( handle );
#if defined(QT_DEBUG)
    const char* error = dlerror();
    if ( error )
	qWarning( error );
#endif
    return ok == 0;
}

static void* qt_resolve_symbol( void* handle, const char* f )
{
    void* address = dlsym( handle, f );
#if defined(QT_DEBUG)
    const char* error = dlerror();
    if ( error )
	qWarning( error );
#endif
    return address;
}

#endif

/*!
  \class QCleanupHandler qcleanuphandler.h

  \brief Provides a save class for memory cleanup.
*/

/*!
  \fn QCleanupHandler::~QCleanupHandler()

  This destructor will delete all handled objects.
*/

/*!
  \fn void QCleanupHandler::add( Type* object )

  Adds \a object to the list that will be destroyed upon
  destruction of the cleanup handler itself.
*/

/*!
  \fn void QCleanupHandler::remove( Type* object )

  Removes \a object from this handler.
*/

/*!
  \fn bool QCleanupHandler::isEmpty() const

  Return TRUE if there are any undeleted objects this handler
  has to care about.
*/



/*!
  \class QLibrary qlibrary.h

  \brief This class provides a wrapper for library loading and unloading.
  \ingroup component
*/

/*!
  \enum QLibrary::Policy

  This enum type is used to set and read the plugin's library
  policy.
  Defined values are:
  <ul>
  <li> \c Delayed - The library get's loaded as soon as needed
  <li> \c Immediately - The library is loaded immediately
  <li> \c Manual - The library has to be loaded and unloaded manually
  </ul>
*/

/*!
  Creates a QLibrary object for the shared library \a filename.
  The library get's loaded if \a pol is Immediately.

  \sa setPolicy(), load()
*/
QLibrary::QLibrary( const QString& filename, Policy pol )
    : pHnd( 0 ), libfile( filename ), libPol( pol ), info( 0 )
{
    if ( pol == Immediately )
	load();
}

/*!
  Deletes the QLibrary object.

  When the library policy is not Manual, the library will be unloaded.

  \sa setPolicy(), unload()
*/
QLibrary::~QLibrary()
{
    if ( libPol != Manual )
	unload();
}

/*!
  Loads the shared library and initializes the connection to the component.
  Returns a pointer to the QUnknownInterface provided by the component if the 
  library was loaded successfully, otherwise returns null.

  This function gets called automatically if the policy is not Manual.
  Otherwise you have to make sure that the library has been loaded before usage.

  \sa setPolicy(), unload()
*/
QUnknownInterface* QLibrary::load()
{
    if ( libfile.isEmpty() )
	return 0;

    if ( !pHnd )
	pHnd = qt_load_library( libfile );

    if ( pHnd && !info ) {
#if defined(QT_DEBUG_COMPONENT)
	qDebug( "%s has been loaded.", libfile.latin1() );
#endif
	typedef QUnknownInterface* (*QtLoadInfoProc)();
	QtLoadInfoProc infoProc;
	infoProc = (QtLoadInfoProc) qt_resolve_symbol( pHnd, "qt_load_interface" );
#if defined(QT_DEBUG_COMPONENT)
	if ( !infoProc )
	    qDebug( "Symbol \"qt_load_interface\" not found." );
#endif
	info = infoProc ? infoProc() : 0;
#if defined(QT_DEBUG_COMPONENT)
	if ( !info )
	    qDebug( "No interface implemented." );
#endif
    }
#if defined(QT_DEBUG_COMPONENT)
    else {
	qDebug( "%s could not be loaded.", libfile.latin1() );
    }
#endif

    return info;
}

/*!
  Returns TRUE if the library is loaded.

  \sa load
*/
bool QLibrary::isLoaded() const
{
    return info != 0;
}

/*!
  Releases the component and unloads the library when successful.
  Returns TRUE if the library could be unloaded, otherwise FALSE.

  This function gets called automatically in the destructor if 
  the policy is not Manual.

  \warning
  If \a force is set to TRUE, the library gets unloaded
  at any cost, which is in most cases a segmentation fault,
  so you should know what you're doing!

  \sa load
*/
bool QLibrary::unload( bool force )
{
    if ( pHnd ) {
	if ( info ) {
	    if ( info->release() ) {
#if defined(QT_DEBUG_COMPONENT) || defined(QT_CHECK_RANGE)
		qDebug( "%s is still in use!", libfile.latin1() );
#endif
		if ( force )
		    delete info;
		else
		    return FALSE;
	    }
	    info = 0;
	}
	if ( !qt_free_library( pHnd ) )
#if defined(QT_DEBUG_COMPONENT)
	{
	    qDebug( "%s could not be unloaded.", libfile.latin1() );
#endif
	    return FALSE;
#if defined(QT_DEBUG_COMPONENT)
	} else {
	    qDebug( "%s has been unloaded.", libfile.latin1() );
	}
#endif
    }
    pHnd = 0;
    return TRUE;
}

/*!
  Sets the current policy to \a pol.
  The library is loaded if \a pol is set to Immediately.

  \sa LibraryPolicy
*/
void QLibrary::setPolicy( Policy pol )
{
    libPol = pol;

    if ( libPol == Immediately && !info )
	load();
}

/*!
  Returns the current policy.

  \sa setPolicy
*/
QLibrary::Policy QLibrary::policy() const
{
    return libPol;
}

/*!
  Returns the filename of the shared library this QLibrary object handles.
*/
QString QLibrary::library() const
{
    return libfile;
}

/*!
  Forwards the query to the component and returns the result.
  If the current policy is not Manual, load() gets called if necessary.

  \sa QUnknownInterface::queryInterface
*/
QUnknownInterface* QLibrary::queryInterface( const QGuid& request )
{
    if ( !info ) {
	if ( libPol != Manual )
	    load();
	else {
#if defined(QT_CHECK_NULL)
	    qWarning( "Tried to use library %s without loading!", libfile.latin1() );
#endif
	    return 0;
	}
    }

    QUnknownInterface * iface = 0;
    if( info )
	iface = info->queryInterface( request );

    return iface;
}

#endif // QT_NO_COMPONENT
