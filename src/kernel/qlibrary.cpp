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
#ifndef QT_NO_PLUGIN
#include "qlibrary.h"
#include "qlibrary_p.h"

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
  <li> \c Default - The library get's loaded as soon as needed
  <li> \c OptimizeSpeed - The library is loaded immediately
  <li> \c Manual - The library has to be loaded and unloaded manually
  </ul>
*/

/*!
  Creates a QLibrary object for the shared library \a filename, propagating \a appIface
  to all interfaces created within this library.
  The library get's loaded immediately if \a pol is OptimizeSpeed.

  \sa setPolicy(), load()
*/
QLibrary::QLibrary( const QString& filename, QApplicationInterface* appIface, Policy pol )
    : pHnd( 0 ), libfile( filename ), libPol( pol ), appInterface( appIface ), info( 0 )
{
    if ( pol == OptimizeSpeed )
	load();
}

/*!
  Deletes the QLibrary object.

  When the library policy is not Manual, the library will be unloaded.

  \sa unload()
*/
QLibrary::~QLibrary()
{
    if ( libPol != Manual )
	unload();
}

/*!
  Loads the shared library and initializes the connection to the QComponentInterface.
  Returns a pointer to the QComponentInterface if the library was loaded successfully,
  otherwise returns null.

  This function gets called automatically if the policy is not Manual.
  Otherwise you have to make sure that the library has been loaded before usage.

  \sa setPolicy()
*/
QComponentInterface* QLibrary::load()
{
    if ( libfile.isEmpty() )
	return 0;

    if ( !pHnd )
	pHnd = qt_load_library( libfile );

    if ( pHnd && !info ) {
	typedef QComponentInterface* (*QtLoadInfoProc)();
	QtLoadInfoProc infoProc;
	infoProc = (QtLoadInfoProc) qt_resolve_symbol( pHnd, "qt_load_interface" );

	info = infoProc ? infoProc() : 0;

	if ( info ) {
	    info->appInterface = appInterface;
	    if ( !info->addRef() ) {
		delete info;
		info = 0;
	    }
	}
    }

    return info;
}

/*!
  Returns TRUE if the library is loaded.
*/
bool QLibrary::isLoaded() const
{
    return info != 0;
}

/*!
  Releases the QComponentInterface and unloads the library when successful.
  Returns TRUE if the library could be unloaded, otherwise FALSE.

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
	    if ( !info->release() && !force )
		return FALSE;

	    delete info;
	    info = 0;
	}
	if ( !qt_free_library( pHnd ) )
	    return FALSE;
    }
    pHnd = 0;
    return TRUE;
}

/*!
  Sets the current policy to \a pol.
  If \a pol is set to OptimizeSpeed, the library gets load immediately.

  \sa LibraryPolicy
*/
void QLibrary::setPolicy( Policy pol )
{
    libPol = pol;

    if ( libPol == OptimizeSpeed )
	load();
}

/*!
  Returns the current policy.
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
  Forwards the query to the QComponentInterface and returns the result.
  If the current policy is not Manual, load() gets called if necessary.

  \sa QUnknownInterface::queryInterface
*/
QUnknownInterface* QLibrary::queryInterface( const QString &request, bool recursive, bool regexp )
{
    if ( !info ) {
	if ( libPol != Manual )
	    load();
	else {
#if defined(DEBUG)
	    qWarning( "Tried to use library %s without loading!", libfile.latin1() );
#endif
	    return 0;
	}
    }

    QUnknownInterface *iface = info->queryInterface( request, recursive, regexp );

    return iface;
}

#endif // QT_NO_PLUGIN
