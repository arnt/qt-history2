/****************************************************************************
** $Id: //depot/qt/main/src/tools/qlibrary.cpp#15 $
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

#include "qcom.h"
#ifndef QT_NO_COMPONENT
#include "qlibrary.h"

#ifndef QT_H
#include "qwindowdefs.h"
#include "qfile.h"
#ifndef QT_LITE_COMPONENT
#include "qobject.h"
#include "qtimer.h"
#endif
#endif // QT_H

//#define QT_DEBUG_COMPONENT 1


// KAI C++ has at the moment problems with unloading the Qt plugins. So don't
// unload them as a workaround for now.
#if defined(Q_CC_KAI)
#define QT_NO_LIBRARY_UNLOAD
#endif

#if defined(Q_WS_WIN) && !defined(QT_DLL)
#define QT_NO_LIBRARY_UNLOAD
#endif

/*
  Private helper class that saves the platform dependent handle
  and does the unload magic using a QTimer.
*/
#ifndef QT_LITE_COMPONENT
class QLibraryPrivate : public QObject
{
    Q_OBJECT
public:
    QLibraryPrivate( QLibrary *lib )
	: QObject( 0, lib->library().latin1() ), pHnd( 0 ), libIface( 0 ), unloadTimer( 0 ), library( lib )
    {}

    ~QLibraryPrivate()
    {
	if ( libIface )
	    libIface->release();
	killTimer();
    }

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

    QLibraryInterface *libIface;

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
	if ( library->policy() == QLibrary::Manual || !pHnd || !libIface )
	    return;

	if ( !libIface->canUnload() )
	    return;

#if QT_DEBUG_COMPONENT == 1
	if ( library->unload() )
	    qDebug( "%s has been automatically unloaded", library->library().latin1() );
#else
	library->unload();
#endif
    }

private:
    QTimer *unloadTimer;
    QLibrary *library;
};

#include "qlibrary.moc"
#else
class QLibraryPrivate
{
public:
    QLibraryPrivate( QLibrary *lib )
	: pHnd( 0 ), libIface( 0 ), library( lib )
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
    QLibraryInterface *libIface;

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

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();
    if ( filename.find( ".dll" ) == -1 )
	filename += ".dll";

#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	pHnd = LoadLibraryW( (TCHAR*)qt_winTchar( filename, TRUE) );
    else
#endif
	pHnd = LoadLibraryA(QFile::encodeName( filename ).data());
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qSystemWarning( QString("Failed to load library %1!").arg( filename ) );
#endif

    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
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

void* QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return 0;

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

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    shl_t handle = new shl_t;
    QString filename = library->library();
    if ( filename.find( ".so" ) == -1 )
	filename = QString( "lib%1.so" ).arg( filename );

    *handle = shl_load( filename.latin1(), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0 );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !handle )
	qDebug( "Failed to load library %s!", filename.latin1() );
#endif
    pHnd = (void*)handle;
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
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

void* QLibraryPrivate::resolveSymbol( const char* symbol )
{
    if ( !pHnd )
	return 0;

    void* address = 0;
    if ( shl_findsym( (shl_t*)pHnd, symbol, TYPE_UNDEFINED, address ) < 0 ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	qDebug( "Couldn't resolve symbol \"%s\"", symbol );
#endif
	return 0;
    }
    return address;
}

#elif defined(Q_OS_MACX)

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

// Mac
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

    pHnd = (void *)NSLinkModule(img, filename, NSLINKMODULE_OPTION_PRIVATE);
    if ( pHnd ) {
	glibs_loaded->insert( filename, pHnd ); //insert it in the loaded hash
	return pHnd;
    }
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


#else
// Something else, assuming POSIX
#include <dlfcn.h>

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();
    if ( filename.find( ".so" ) == -1 )
	filename = QString( "lib%1.so" ).arg( filename );

    pHnd = dlopen( filename.latin1() , RTLD_LAZY );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qWarning( dlerror() );
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
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

void* QLibraryPrivate::resolveSymbol( const char* f )
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
  \preliminary
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

  Note that \a filename must not include the (platform specific) file extension.

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

void QLibrary::createInstanceInternal()
{
    if ( libfile.isEmpty() )
	return;

    if ( !d->pHnd ) {
	Q_ASSERT( entry == 0 );
	d->loadLibrary();
    }

    if ( d->pHnd && !entry ) {
#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
	qDebug( "%s has been loaded.", libfile.latin1() );
#endif
#ifndef QT_LITE_COMPONENT
	typedef void(*UCMInitProc)( QApplication *theApp );
	UCMInitProc ucmInitProc;
	ucmInitProc = (UCMInitProc) resolve( "ucm_initialize" );
	if ( ucmInitProc )
	    ucmInitProc( qApp );
#endif
	typedef QUnknownInterface* (*UCMInstanceProc)();
	UCMInstanceProc ucmInstanceProc;
	ucmInstanceProc = (UCMInstanceProc) resolve( "ucm_instantiate" );
	entry = ucmInstanceProc ? ucmInstanceProc() : 0;
	if ( entry ) {
	    entry->queryInterface( IID_QLibrary, (QUnknownInterface**)&d->libIface);
	    if ( d->libIface ) {
		if ( !d->libIface->init() ) {
#if defined(QT_DEBUG_COMPONENT)
		    qDebug( "%s: QLibraryInterface::init() failed.", libfile.latin1() );
#endif
		    unload();
		    return;
		}

		d->killTimer();
		if ( libPol != Manual )
		    d->startTimer();
	    }
	} else {
#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONTENT == 2
	    qDebug( "%s: No interface implemented.", libfile.latin1() );
#endif
	    unload();
	}
    }
}

/*!
  Returns the address of the exported symbol \a symb. The library gets
  loaded if necessary. The function returns NULL if the symbol could
  not be resolved, or if loading the library failed.

  \code
  typedef int (*addProc)( int, int );

  addProc add = (addProc) library->resolve( "add" );
  if ( add )
      return add( 5, 8 );
  else
      return 5 + 8;
  \endcode

  \sa queryInterface
*/
void *QLibrary::resolve( const char* symb )
{
    if ( !d->pHnd )
	d->loadLibrary();
    if ( !d->pHnd )
	return 0;

    void *address = d->resolveSymbol( symb );
    if ( !address ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	// resolveSymbol() might give a warning; so let that warning look so fatal
	qWarning( QString("Trying to resolve symbol \"_%1\" instead").arg( symb ) );
#endif
	address = d->resolveSymbol( QString( "_" ) + symb );
    }
    return address;
}

/*!
  Returns whether the library is loaded.

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

  \sa queryInterface, resolve
*/
bool QLibrary::unload( bool force )
{
    if ( !d->pHnd )
	return TRUE;

    if ( entry ) {
	if ( d->libIface ) {
	    d->libIface->cleanup();

	    bool can = d->libIface->canUnload();
	    can = ( d->libIface->release() <= 1 ) && can;
	    // the "entry" member must be the last reference to the component
	    if ( can || force ) {
		d->libIface = 0;
	    } else {
		d->libIface->addRef();
		return FALSE;
	    }
	}

	if ( entry->release() ) {
#if defined(QT_DEBUG_COMPONENT) || defined(QT_CHECK_RANGE)
	    qDebug( "%s is still in use!", libfile.latin1() );
#endif
	    if ( force ) {
		delete entry;
	    } else {
		entry->addRef();
		return FALSE;
	    }
	}
	d->killTimer();

	entry = 0;
    }

// ### this is a hack to solve problems with plugin unloading und KAI C++
// (other compilers may have the same problem)
#if defined(QT_NO_LIBRARY_UNLOAD)
    if ( TRUE ) {
#else
    if ( !d->freeLibrary() ) {
#endif
#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
	qDebug( "%s could not be unloaded.", libfile.latin1() );
#endif
	return FALSE;
    }

#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
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
QRESULT QLibrary::queryInterface( const QUuid& request, QUnknownInterface** iface )
{
    if ( !entry )
	createInstanceInternal();

    if( entry )
	entry->queryInterface( request, iface );
    //### return value
}

#endif // QT_NO_COMPONENT
