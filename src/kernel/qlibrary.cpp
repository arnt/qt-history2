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
#define QT_DEBUG_COMPONENT 1

#ifndef QT_H
#include "qwindowdefs.h"
#include "qfile.h"
#ifndef QT_LITE_COMPONENT
#include "qobject.h"
#include "qtimer.h"
#endif
#endif // QT_H

/*
  Private helper class that saves the platform dependent handle
  and does the unload magic using a QTimer.
*/
#ifndef QT_LITE_COMPONENT
class QLibrary::QLibraryPrivate : private QObject
{
    Q_OBJECT
public:
    QLibraryPrivate( QLibrary *lib ) 
	: QObject( 0, lib->library().latin1() ), pHnd( 0 ), unloadTimer( 0 ), library( lib )
    {}

    void startTimer()
    {
	unloadTimer = new QTimer( this );
	connect( unloadTimer, SIGNAL( timeout() ), this, SLOT( tryUnload() ) );
	unloadTimer->start( 5000, FALSE );
    }

    void killTimer()
    {
	delete unloadTimer;
	unloadTimer = 0;
    }

#ifdef Q_WS_WIN
    HINSTANCE pHnd;
#else
    void *pHnd;
#endif

    bool loadLibrary();
    bool freeLibrary();
    void *resolveSymbol( const char * );

public slots:
    /*
      Only components that implement the QLibraryInterface can 
      be unloaded automatically.
    */
    void tryUnload() 
    {
	if ( library->policy() == Manual )
	    return;

	QLibraryInterface *lIface = (QLibraryInterface*)library->queryInterface( IID_QLibraryInterface );
	if ( !lIface )
	    return;

	if ( !lIface->canUnload() ) {
	    lIface->release();
	    return;
	}

	lIface->release();

    #if QT_DEBUG_COMPONENT == 1
	if ( library->unload() )
	    qDebug( "%s has been automatically unloaded", library->library().latin1() );
    #else
	library()->unload();
    #endif
    }

private:
    QTimer *unloadTimer;
    QLibrary *library;
};

#include "qlibrary.moc"
#else
class QLibrary::QLibraryPrivate
{
public:
    QLibraryPrivate( QLibrary *lib )
	: pHnd( 0 ), library( lib )
    {}

    void startTimer()
    {
    }

    void killTimer()
    {
    }

#ifdef Q_WS_WIN
    HINSTANCE pHnd;
#else
    void *pHnd;
#endif

    bool loadLibrary();
    bool freeLibrary();
    void *resolveSymbol( const char * );

private:
    QLibrary *library;
};
#endif

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/
#ifdef Q_OS_WIN32
// Windows
#include "qt_windows.h"

bool QLibrary::QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	pHnd = LoadLibraryW( (TCHAR*)qt_winTchar(library->library(), TRUE) );
    else
#endif
	pHnd = LoadLibraryA(QFile::encodeName(library->library()).data());
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qSystemWarning( QString("Failed to load library %1!").arg( library->library() ) );
#endif

    return pHnd != 0;
}

bool QLibrary::QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;
    bool ok = FreeLibrary( pHnd );
    if ( ok )
	pHnd = 0;
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    else
	qSystemWarning( "Failed to unload library!" );
#endif

    return ok;
}

void* QLibrary::QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return NULL;

    void* address = GetProcAddress( pHnd, f );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !address )
	qSystemWarning( QString("Couldn't resolve symbol \"%1\"").arg( f ) );
#endif

    return address;
}

#elif defined(Q_OS_HPUX)
// for HP-UX < 11.x and 32 bit
#include <dl.h>

bool QLibrary::QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    shl_t handle = new shl_t;
    *handle = shl_load( library->library().latin1(), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0 );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !handle )
	qDebug( "Failed to load library %1!", library->library().latin1() );
#endif
    pHnd = (void*)handle;
    return pHnd != 0;
}

bool QLibrary::QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

    shl_t handle = *((shl_t*)pHnd);
    if ( !shl_unload( pHnd ) {
	delete handle;
	pHnd = 0;
	return TRUE;
    }
    return FALSE;
}

void* QLibrary::QLibraryPrivate::resolveSymbol( const char* symbol )
{
    if ( !pHnd )
	return NULL;

    void* address;
    if ( shl_findsym( (shl_t*)pHnd, symbol, TYPE_UNDEFINED, address ) < 0 ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	qDebug( "Couldn't resolve symbol \"%1\"", symbol );
#endif
	return 0;
    }
    return address;
}

#elif defined(Q_OS_MACX)

#define DO_MAC_LIBRARY
#include <qdict.h>

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

static QDict<void> *glibs_loaded = NULL;

// Mac
bool QLibrary::QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    if(!glibs_loaded)
	glibs_loaded = new QDict<void>();
    else if( ( pHnd = glibs_loaded->find(library->library()) ))
	return TRUE;

#ifdef DO_MAC_LIBRARY
    NSObjectFileImage img;
    if( NSCreateObjectFileImageFromFile(library->library(), &img)  != NSObjectFileImageSuccess )
	return FALSE;

    pHnd = (void *)NSLinkModule(img, library->library(), NSLINKMODULE_OPTION_PRIVATE);
    if ( pHnd ) {
	glibs_loaded->insert( library->library(), pHnd ); //insert it in the loaded hash
	return pHnd;
    }
#else
    return FALSE;
#endif
}

bool QLibrary::QLibraryPrivate::freeLibrary()
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

void* QLibrary::QLibraryPrivate::resolveSymbol( const char *symbol )
{
    if ( !pHnd )
	return NULL;

#ifdef DO_MAC_LIBRARY
    QCString symn2;
    symn2.sprintf("_%s", symbol);
    return NSAddressOfSymbol(NSLookupSymbolInModule(pHnd, symn2));
#else
    return NULL;
#endif
}

#elif defined(Q_OS_MAC9)

bool QLibrary::QLibraryPrivate::loadLibrary()
{
    return FALSE;
}

bool QLibrary::QLibraryPrivate::freeLibrary()
{
    return FALSE;
}

void* QLibrary::QLibraryPrivate::resolveSymbol( const char *symbol )
{
    return NULL;
}


#else
// Something else, assuming POSIX
#include <dlfcn.h>

bool QLibrary::QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    pHnd = dlopen( library->library(), RTLD_LAZY );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qWarning( dlerror() );
#endif
    return pHnd != 0;
}

bool QLibrary::QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;

    int ec = dlclose( pHnd );
    if ( !ec )
	pHnd = 0;
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    else {
	const char* error = dlerror();
	if ( error )
	    qWarning( error );
    }
#endif
    return pHnd == 0;
}

void* QLibrary::QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return 0;

    void* address = dlsym( pHnd, f );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    const char* error = dlerror();
    if ( error )
	qWarning( error );
#endif
    return address;
}

#endif


/*!
  \class QLibrary qlibrary.h

  \brief The QLibrary class provides a wrapper for handling shared libraries.
  \ingroup componentmodel
*/

/*!
  \enum QLibrary::Policy

  This enum type defines the various policies a QLibrary can have with respect to
  loading and unloading the shared library.

  The \e policy can be:

  \value Delayed  The library get's loaded as soon as needed
  \value Immediately  The library is loaded immediately
  \value Manual  The library has to be unloaded manually
*/

/*!
  Creates a QLibrary object for the shared library \a filename.
  The library get's loaded if \a pol is Immediately.

  \sa setPolicy(), unload()
*/
QLibrary::QLibrary( const QString& filename, Policy pol )
    : libfile( filename ), libPol( pol ), entry( 0 )
{
    d = new QLibraryPrivate( this );
    if ( pol == Immediately )
	d->loadLibrary();
}

/*!
  Deletes the QLibrary object. 
  The library will be unloaded if the policy is not Manual.

  \sa unload(), setPolicy()
*/
QLibrary::~QLibrary()
{
    if ( libPol == Manual || !unload() ) {
	if ( entry ) {
	    entry->release();
	    entry = 0;
	}
    }
    delete d;
}

/*!
  Loads the shared library and initializes the connection to the component server.
  Returns a pointer to the QUnknownInterface provided by the component if the 
  library was loaded successfully, otherwise unloades the library again and returns 
  null.
  If the component implements the QLibraryInterface, the init() function will
  be called and the loading canceled if this function returns FALSE. 
  If the policy is not Manual, the system will call the canUnload() function of 
  this interface and try to unload the library in regular intervalls.

  This function gets called automatically by queryInterface.

  \sa setPolicy(), unload(), resolve
*/
QUnknownInterface* QLibrary::createInstance()
{
    if ( libfile.isEmpty() )
	return 0;

    if ( !d->pHnd )
	d->loadLibrary();

    if ( d->pHnd && !entry ) {
#if QT_DEBUG_COMPONENT == 2
	qDebug( "%s has been loaded.", libfile.latin1() );
#endif
	typedef QUnknownInterface* (*QtLoadInfoProc)();
	QtLoadInfoProc infoProc;
	infoProc = (QtLoadInfoProc) d->resolveSymbol( "qt_load_interface" );
	if ( !infoProc )
	    infoProc = (QtLoadInfoProc) d->resolveSymbol( "_qt_load_interface" );
#if QT_DEBUG_COMPONENT == 2
	if ( !infoProc )
	    qDebug( "%s: Symbol \"qt_load_interface\" not found.", libfile.latin1() );
#endif
	entry = infoProc ? infoProc() : 0;
	if ( entry ) {
	    QLibraryInterface *piface = (QLibraryInterface*)entry->queryInterface( IID_QLibraryInterface );
	    if ( piface ) {
		bool ok = piface->init();
		piface->release();
		if ( !ok ) {
		    entry->release();
		    entry = 0;
#if defined(QT_DEBUG_COMPONENT)
		    qDebug( "%s: QLibraryInterface::init() failed.", libfile.latin1() );
#endif
		    unload();
		    return 0;
		}

		d->killTimer();
		if ( libPol != Manual )
		    d->startTimer();
	    }
	} else {
#if QT_DEBUG_COMPONTENT == 2
	    qDebug( "%s: No interface implemented.", libfile.latin1() );
#endif
	    unload();
	}
    }
#if QT_DEBUG_COMPONENT == 2
    else {
	qDebug( "%s could not be loaded.", libfile.latin1() );
    }
#endif

    return entry;
}

/*!
  Returns the address of the exported symbol \a symb. The library gets
  loaded if necessary. The function returns NULL if the symbol could
  not be resolved or the library not be loaded.

  \code
  typedef int (*addProc)( int, int );

  addProc add = (addProc) library->resolve( "add" );
  if ( add )
      return add( 5, 8 );
  else
      return 5 + 8;
  \endcode

  \sa createInstance, queryInterface
*/
void *QLibrary::resolve( const char* symb )
{
    if ( !d->pHnd )
	d->loadLibrary();
    if ( !d->pHnd )
	return 0;

    void *address = d->resolveSymbol( symb );
    if ( !address )
	address = d->resolveSymbol( QString( "_" ) + symb );
    return address;
}

/*!
  Returns TRUE if the library is loaded.

  \sa unload
*/
bool QLibrary::isLoaded() const
{
    return d->pHnd != 0;
}

/*!
  Releases the component and unloads the library when successful.
  Returns TRUE if the library could be unloaded, otherwise FALSE.
  If the component implements the QLibraryInterface, the cleanup()
  function of this interface will be called. The unloading will be
  cancelled if the subsequent call to canUnload() returns FALSE.

  This function gets called automatically in the destructor if 
  the policy is not Manual.

  \warning
  If \a force is set to TRUE, the library gets unloaded at any cost, 
  which is in most cases a segmentation fault, so you should know what 
  you're doing!

  \sa createInstance, queryInterface, resolve
*/
bool QLibrary::unload( bool force )
{
    if ( !d->pHnd )
	return TRUE;

    if ( entry ) {
	QLibraryInterface *piface = (QLibraryInterface*)entry->queryInterface( IID_QLibraryInterface );
	if ( piface ) {
	    piface->cleanup();

	    bool can = piface->canUnload();
	    piface->release();
	    if ( !can ) {
#if QT_DEBUG_COMPONENT == 2
		qDebug( "%s refuses to be unloaded!", libfile.latin1() );
#endif
		if ( !force )
		    return FALSE;
	    }
	}

	if ( entry->release() ) {
#if defined(QT_DEBUG_COMPONENT) || defined(QT_CHECK_RANGE)
	    qDebug( "%s is still in use!", libfile.latin1() );
#endif
	    if ( force )
		delete entry;
	    else {
		entry->addRef();
		return FALSE;
	    }
	}
	d->killTimer();

	entry = 0;
    }

    if ( !d->freeLibrary() )
    {
#if QT_DEBUG_COMPONENT == 2
	qDebug( "%s could not be unloaded.", libfile.latin1() );
#endif
	return FALSE;
    }

#if QT_DEBUG_COMPONENT == 2
    qDebug( "%s has been unloaded.", libfile.latin1() );
#endif

    d->pHnd = 0;
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

    if ( libPol == Immediately && !d->pHnd )
	d->loadLibrary();
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
  The library gets loaded if necessary.

  \sa QUnknownInterface::queryInterface
*/
QUnknownInterface* QLibrary::queryInterface( const QUuid& request )
{
    if ( !entry )
	createInstance();

    QUnknownInterface * iface = 0;
    if( entry )
	iface = entry->queryInterface( request );

    return iface;
}

#endif // QT_NO_COMPONENT
