/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qplugin.cpp#1 $
**
** Implementation of QPlugIn class
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
#include "qplugin.h"
#include "qplugin_p.h"

/*!
  \class QCleanUpHandler qcleanuphandler.h

  \brief Provides a save class for memory cleanup.
*/

/*!
  \fn QCleanUpHandler::~QCleanUpHandler()

  This destructor will delete all handled objects.
*/

/*!
  \fn void QCleanUpHandler::addCleanUp( Type* object )

  Adds an object to the list that will be destroyed upon
  destruction of the cleanup handler itself.
*/

/*!
  \fn bool QCleanUpHandler::isClean()

  Return TRUE if there are any undeleted objects this handler
  has to care about.
  This may be useful for plugins that do not want to be unloaded
  until all memory allocated in the library's scope has been freed.
*/



/*!
  \class QPlugIn qplugin.h

  \brief This class provides a wrapper for library loading and unloading.
  \ingroup component
*/

/*!
  \enum QPlugIn::LibraryPolicy

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
  Creates a QPlugIn object for the shared library \a filename, propagating \a appIface
  to all interfaces created within this library.
  The library get's loaded immediately if \a pol is OptimizeSpeed.

  \sa setPolicy(), load()
*/
QPlugIn::QPlugIn( const QString& filename, QApplicationInterface* appIface, LibraryPolicy pol )
    : info( 0 ), pHnd( 0 ), libfile( filename ), libPol( pol ), appInterface( appIface )
{
    if ( pol == OptimizeSpeed )
	load();
}

/*!
  Deletes the QPlugIn object.

  When the library policy is not Manual, the library will be unloaded.

  \sa unload()
*/
QPlugIn::~QPlugIn()
{
    if ( libPol != Manual )
	unload();
}

/*!
  Loads the shared library and initializes the connection to the QPlugInInterface.
  Returns a pointer to the QPlugInInterface if the library was loaded successfully,
  otherwise returns null.

  This function gets called automatically if the policy is not Manual.
  Otherwise you have to make sure that the library has been loaded before usage.

  \sa setPolicy()
*/
QPlugInInterface* QPlugIn::load()
{
    if ( libfile.isEmpty() )
	return 0;

    if ( !pHnd ) {
	pHnd = qt_load_library( libfile );

	if ( !pHnd )
	    return 0;
    }

    if ( pHnd )
	return info ? info : loadInterface();
    return 0;
}

/*!
  Returns TRUE if the library is loaded.
*/
bool QPlugIn::loaded() const
{
    return pHnd != 0;
}

/*!
  Releases the QPlugInInterface and unloads the library when successful.
  Returns TRUE if the library could be unloaded, otherwise FALSE.

  \warning
  If \a force is set to TRUE, the library gets unloaded
  at any cost, which is in most cases a segmentation fault,
  so you should know what you're doing!

  \sa load
*/
bool QPlugIn::unload( bool force )
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


bool QPlugIn::use()
{
    if ( !pHnd || !info ) {
	if ( libPol != Manual )
	    return load();
#ifdef QT_CHECK_RANGE
	else
	    qWarning( "Tried to use library %s without loading!", libfile.latin1() );
#endif
	return FALSE;
    }
    return TRUE;
}

QPlugInInterface* QPlugIn::loadInterface()
{
    if ( !pHnd ) {
#if defined(QT_CHECK_RANGE)
	qWarning("QPlugIn::loadInterface(): Failed to load library - no handle!");
#endif
	return 0;
    }

    typedef QPlugInInterface* (*QtLoadInfoProc)();
    QtLoadInfoProc infoProc;
    infoProc = (QtLoadInfoProc) qt_resolve_symbol( pHnd, "qt_load_interface" );

    if ( !infoProc )
	return 0;
    info = infoProc();

    if ( info ) {
	info->appInterface = appInterface;
	if ( !info->ref() ) {
	    delete info;
	    info = 0;
	}
	return info;
    }

    return 0;
}

/*!
  Sets the current policy to \a pol.
  If \a pol is set to OptimizeSpeed, the library gets load immediately.

  \sa LibraryPolicy
*/
void QPlugIn::setPolicy( LibraryPolicy pol )
{
    libPol = pol;

    if ( libPol == OptimizeSpeed )
	load();
}

/*!
  Returns the current policy.
*/
QPlugIn::LibraryPolicy QPlugIn::policy() const
{
    return libPol;
}

/*!
  Returns the filename of the shared library this QPlugIn object handles.
*/
QString QPlugIn::library() const
{
    return libfile;
}

/*!
  Forwards the query to the QPlugInInterface and returns the result.
*/
QUnknownInterface* QPlugIn::queryInterface( const QString &request, bool rec )
{
    if ( !use() )
	return 0;

    QUnknownInterface *iface = info->queryInterface( request, rec );

    return iface;
}

#endif // QT_NO_PLUGIN
