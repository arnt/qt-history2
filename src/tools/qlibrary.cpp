/****************************************************************************
** $Id: $
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

#include "qapplication.h"

#ifndef QT_NO_COMPONENT
#include "private/qlibrary_p.h"
#ifndef QT_H
#include "qwindowdefs.h"
#include "qfile.h"
#endif // QT_H

// KAI C++ has at the moment problems with unloading the Qt plugins. So don't
// unload them as a workaround for now.
#if defined(Q_CC_KAI) || defined(Q_OS_MAC)
#define QT_NO_LIBRARY_UNLOAD
#endif

#if defined(Q_WS_WIN) && !defined(QT_MAKEDLL)
#define QT_NO_LIBRARY_UNLOAD
#endif

/*!
  \class QLibrary qlibrary.h

  \brief The QLibrary class provides a wrapper for handling shared libraries.
  \ingroup componentmodel

  An instance of a QLibrary object can handle a single shared library and provide
  access to the functionality in the library in a platform independent way. If the
  library is a component server, QLibrary provides access to the exported component
  and can directly query this component for interfaces.

  QLibrary makes sure that the shared library is loaded and stays in memory for the
  time it is being used, and can also unload the library upon destruction and thus
  release unused resources.

  A typical use of QLibrary is to resolve an exported symbol in a shared object, and
  to e.g. call the function that this symbol represents. This is called "explicit
  linking" in contrast to "implicit linking", which is done by the link step in the
  build process when linking an binary against a library.

  The following code snipplet will load a library, resolve the symbol "mysymbol",
  and call the function if everything succeeded. If anything went wrong, e.g. the
  library file is not existing, or the symbol is not defined, the function pointer
  will become a null pointer. Upon destruction of the QLibrary object the library
  will be unloaded again, making all referenced to memory allocated in the library
  invalid.

  \code
  typedef void (*MyPrototype)();
  MyPrototype myFunction;

  QLibrary myLib( "mylib" );
  myFunction = (MyProtoype)myLib.resolve( "mysymbol" );
  if ( myFunction ) {
    myFunction();
  }
  \endcode

  Using this mechanism is only safe when the correct type of the function in the
  shared object is guaranteed - a wrong type might crash the application or leave
  it in an unstable state. Keeping a reference to the function pointer is also
  dangerous, as the library might be unloaded.

  A safe and thus commonly used architecture for explicit linking of symbols is
  to use \link component.html interfaces and components \endlink. QLibrary supports
  the component model in Qt with the queryInterface function.

  \code
  MySpecialInterface *iface = 0;

  QLibrary myLib( "mylib" );
  if ( myLib.queryInterface( IID_MySpecial, (QUnknownInterface**)&iface ) == QS_OK ) {
      iface->myFunction();
      iface->release();
  }
  \endcode

  The shared object "mylib", which now acts as a component server, is guaranteed to
  provide a pointer to the correct interface type, so that it becomes safe to use
  unknown libraries in an application.
*/

/*!
  \enum QLibrary::Policy

  This enum type defines the various policies a QLibrary can have with respect to
  loading and unloading the shared library.

  The \e policy can be:

  \value Delayed  The library get's loaded as soon as needed and unloaded in the destructor
  \value Immediately  The library is loaded immediately and unloaded in the destructor
  \value Manual  Like delayed, and library has to be unloaded manually
*/

/*!
  Creates a QLibrary object for the shared library \a filename.
  The library get's loaded if \a pol is Immediately.

  Note that \a filename does not need to include the (platform specific)
  file extension, so calling

  \code
  QLibrary lib( "mylib" );
  \endcode

  would be equivalent to

  \code
  QLibrary lib( "mylib.dll" );
  \endcode

  on Windows. But \e "mylib.dll" will obviously not work on other platforms.

  \sa setPolicy(), unload()
*/
QLibrary::QLibrary( const QString& filename, Policy pol )
    : libfile( filename ), libPol( pol ), entry( 0 )
{
    d = new QLibraryPrivate( this );
    if ( pol == Immediately )
	load();
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
	load();
    }

    if ( d->pHnd && !entry ) {
#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
	qDebug( "%s has been loaded.", library().latin1() );
#endif
#ifndef QT_LITE_COMPONENT
#  ifdef Q_CC_BOR
	typedef void __stdcall (*UCMInitProc)(QApplication*);
#  else
	typedef void (*UCMInitProc)(QApplication*);
#  endif
	UCMInitProc ucmInitProc;
	ucmInitProc = (UCMInitProc) resolve( "ucm_initialize" );
	if ( ucmInitProc )
	    ucmInitProc( qApp );
#endif
#ifdef Q_CC_BOR
	typedef QUnknownInterface* __stdcall (*UCMInstanceProc)();
#else
	typedef QUnknownInterface* (*UCMInstanceProc)();
#endif
	UCMInstanceProc ucmInstanceProc;
	ucmInstanceProc = (UCMInstanceProc) resolve( "ucm_instantiate" );
	entry = ucmInstanceProc ? ucmInstanceProc() : 0;
	if ( entry ) {
	    entry->queryInterface( IID_QLibrary, (QUnknownInterface**)&d->libIface);
	    if ( d->libIface ) {
		if ( !d->libIface->init() ) {
#if defined(QT_DEBUG_COMPONENT)
		    qDebug( "%s: QLibraryInterface::init() failed.", library().latin1() );
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
	    qDebug( "%s: No interface implemented.", library().latin1() );
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
	load();
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
    \overload

  Loads the library \a filename and returns the address of the exported symbol \a symb.
  Note that like for the constructor, \a filename does not need to include the (platform specific)
  file extension. The library staying loaded until the process exits.

  The function returns a null pointer if the symbol could not be resolved or if loading
  the library failed.
*/
void *QLibrary::resolve( const QString &filename, const char *symb )
{
    QLibrary lib( filename, Manual );
    return lib.resolve( symb );
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
  Loads the library.
*/
bool QLibrary::load()
{
    return d->loadLibrary();
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
	    qDebug( "%s is still in use!", library().latin1() );
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
#if !defined(QT_NO_LIBRARY_UNLOAD)
    if ( !d->freeLibrary() ) {
#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
	qDebug( "%s could not be unloaded.", library().latin1() );
#endif
	return FALSE;
#else
	return TRUE;
#endif
#if !defined(QT_NO_LIBRARY_UNLOAD)
    }

#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
    qDebug( "%s has been unloaded.", library().latin1() );
#endif

    d->pHnd = 0;
    return TRUE;
#endif
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
  Returns the filename of the shared library this QLibrary object handles,
  including the platform specific file extension.

  \code
  QLibrary lib( "mylib" );
  QString str = lib.library();
  \endcode

  will set \e str to "mylib.dll" on Windows, and "libmylib.so" on Linux.
*/
QString QLibrary::library() const
{
    if ( libfile.isEmpty() )
	return libfile;

    QString filename = libfile;
#if defined(Q_WS_WIN)
    if ( filename.find( ".dll" ) == -1 )
	filename += ".dll";
#elif defined(Q_OS_MACX)
    if ( filename.find( ".dylib" ) == -1 )
	filename += ".dylib";
#else
    if ( filename.find( ".so" ) == -1 )
	filename = QString( "lib%1.so" ).arg( filename );
#endif

    return filename;
}

/*!
  Forwards the query to the component and returns the result. \a request and \a iface
  are propagated to the component's queryInterface implementation.

  The library gets loaded if necessary.

  \sa QUnknownInterface::queryInterface
*/
QRESULT QLibrary::queryInterface( const QUuid& request, QUnknownInterface** iface )
{
    if ( !entry ) {
	createInstanceInternal();
    }

    return entry ? entry->queryInterface( request, iface ) : QE_NOINTERFACE;
}

#endif // QT_NO_COMPONENT
