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
  \fn bool QCleanUpHandler::clean()

  Return TRUE if there are any undeleted objects this handler
  has to care about.
  This may be useful for plugins that do not want to be unloaded
  until all memory allocated in the library's scope has been freed.
*/



/*!
  \class QPlugIn qplugin.h

  \brief This class provides a wrapper for library loading and unloading.
  \ingroup plugin
*/

/*!
  \enum QPlugIn::LibraryPolicy

  This enum type is used to set and read the plugin's library
  policy.
  Defined values are:
  <ul>
  <li> \c Default - The library get's loaded on first need and never unloaded
  <li> \c OptimizeSpeed - The library is loaded as soon as possible at the cost of memory
  <li> \c Manual - The library has to be loaded and unloaded manually
  </ul>
*/

/*!
  Creates a QPlugIn object for the shared library \a filename.

  The library get's loaded immediately if \a pol is OptimizeSpeed,
  as soon as necessary if \a pol is Default, or not automatically
  if \a pol is Manual.

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

  When the library policy is not Manual, the plugin will try to unload the library.

  \sa unload()
*/
QPlugIn::~QPlugIn()
{
    if ( libPol != Manual )
	unload();
}

/*!
  Loads the shared library and initializes the connection to the QPlugInInterface.
  Returns TRUE if the library was loaded successfully, otherwise returns FALSE.

  This function gets called automatically if the policy is not Manual. 
  Otherwise you have to make sure that the library has been loaded before usage.

  \sa setPolicy()
*/
bool QPlugIn::load()
{
    if ( libfile.isEmpty() )
	return FALSE;

    if ( !pHnd ) {
	pHnd = qt_load_library( libfile );

	if ( !pHnd )
	    return FALSE;
    }

    if ( pHnd )
	return info ? TRUE : loadInterface();
    return FALSE;
}

/*!
  Releases the QPlugInInterface and unloads the library when successful.

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
#ifdef CHECK_RANGE
	else
	    qWarning( "Tried to use library %s without loading!", libfile.latin1() );
#endif
	return FALSE;
    }
    return TRUE;
}

/*!
  Loads the interface of the shared library and calls the initialize function.
  Returns TRUE if successful, or FALSE if the interface could not be loaded.

  \sa QApplicationInterface
*/
bool QPlugIn::loadInterface()
{
    if ( !pHnd ) {
#if defined(CHECK_RANGE)
	qWarning("QPlugIn::loadInterface(): Failed to load library - no handle!");
#endif
	return FALSE;
    }

    typedef QPlugInInterface* (*QtLoadInfoProc)();
    QtLoadInfoProc infoProc;
    infoProc = (QtLoadInfoProc) qt_resolve_symbol( pHnd, "qt_load_interface" );

    if ( !infoProc )
	return FALSE;
    info = infoProc();
    
    if ( info ) {
	info->appInterface = appInterface;
	return info->ref();
    }

    return FALSE;
}

/*!
  Returns TRUE if the library is loaded.
*/
bool QPlugIn::loaded() const
{
    return pHnd != 0;
}

/*!
  Sets the current policy to \a pol.
  Forces the library to load if \a pol is set to
  OptimizeSpeed.

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
QUnknownInterface* QPlugIn::queryInterface( const QString &request )
{
    if ( !use() )
	return 0;

    QUnknownInterface *iface = info->queryInterface( request );
   
    if ( !iface )
	return 0;

    return iface;
}

#endif // QT_NO_PLUGIN
