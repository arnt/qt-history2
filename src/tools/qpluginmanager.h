/****************************************************************************
** $Id: $
**
** Definition of QPluginManager class
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

#ifndef QPLUGINMANAGER_H
#define QPLUGINMANAGER_H

#ifndef QT_H
#include "qgpluginmanager.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

template<class Type>
class Q_EXPORT QPluginManager : public QGPluginManager
{
public:
    QPluginManager( const QUuid& id, const QString& path = QString::null, QLibrary::Policy pol = QLibrary::Delayed, bool cs = TRUE )
	: QGPluginManager( id, pol, cs )
    {
	if ( !path.isEmpty() )
	    addLibraryPath( path );
    }

    QLibrary* addLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return 0;

	QLibrary *plugin = libDict[file];
	if ( plugin )
	    return plugin;

	// Create a library object, and try to get the desired interface
	plugin = new QLibrary( file, defPol );

	bool useful = FALSE;

	Type* iFace = 0;
	plugin->queryInterface( interfaceId, (QUnknownInterface**) &iFace );
	if ( iFace ) {
	    QFeatureListInterface *fliFace = 0;
	    QComponentInformationInterface *cpiFace = 0;
	    iFace->queryInterface( IID_QFeatureList, (QUnknownInterface**)&fliFace );
	    if ( !fliFace )
		plugin->queryInterface( IID_QFeatureList, (QUnknownInterface**)&fliFace );
	    if ( !fliFace ) {
		iFace->queryInterface( IID_QComponentInformation, (QUnknownInterface**)&cpiFace );
		if ( !cpiFace )
		    plugin->queryInterface( IID_QComponentInformation, (QUnknownInterface**)&cpiFace );
	    }
	    QStringList fl;
	    if ( fliFace )
		// Map all found features to the library
		fl = fliFace->featureList();
	    else if ( cpiFace )
		fl << cpiFace->name();

	    for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ ) {
		useful = TRUE;
#ifdef QT_CHECK_RANGE
		QLibrary *old = 0;
		if ( !(old = plugDict[*f]) )
		    plugDict.replace( *f, plugin );
		else
		    qWarning("%s: Feature %s already defined in %s!", plugin->library().latin1(), (*f).latin1(), old->library().latin1() );
#else
		plugDict.replace( *f, plugin );
#endif
	    }
	    if ( fliFace )
		fliFace->release();
	    if ( cpiFace )
		cpiFace->release();
	    iFace->release();
	}
	if ( defPol != QLibrary::Immediately )
	    plugin->unload();

	if ( useful ) {
	    libDict.replace( file, plugin );
	    if ( !libList.contains( file ) )
		libList.append( file );
	    return plugin;
	} else {
	    delete plugin;
	    libList.remove( file );
	    return 0;
	}
    }

    bool removeLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return FALSE;
	
	libList.remove( file );
	
	QLibrary* plugin = libDict[ file ];
	if ( !plugin )
	    return FALSE;

	// Unregister all features of this plugin
	Type *iFace = 0;
	plugin->queryInterface( interfaceId, (QUnknownInterface**)&iFace );
	if ( iFace ) {
	    QFeatureListInterface *fliFace = 0;
	    QComponentInformationInterface *cpiFace = 0;
	    iFace->queryInterface( IID_QFeatureList, (QUnknownInterface**)&iFace );
	    if ( !fliFace )
		plugin->queryInterface( IID_QFeatureList, (QUnknownInterface**)&fliFace );
	    if ( !fliFace ) {
		iFace->queryInterface( IID_QComponentInformation, (QUnknownInterface**)&cpiFace );
		if ( !cpiFace )
		    plugin->queryInterface( IID_QComponentInformation, (QUnknownInterface**)&cpiFace );
	    }
	    QStringList fl;
	    if ( fliFace )
		fl = fliFace->featureList();
	    else if ( cpiFace )
		fl << cpiFace->name();

	    for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ ) {
		plugDict.remove( *f );
	    }

	    if ( fliFace )
		fliFace->release();
	    if ( cpiFace )
		cpiFace->release();
	    iFace->release();
	}
	bool unloaded = plugin->unload();
	// This deletes the QLibrary object!
	libDict.remove( file );

	return unloaded;
    }

    QRESULT queryInterface(const QString& feature, Type** iface) const
    {
	QLibrary* plugin = 0;
	plugin = library( feature );

	return plugin ? plugin->queryInterface( interfaceId, (QUnknownInterface**)iface ) : QE_NOINTERFACE;
    }

#ifdef Q_QDOC
#error "The Symbol Q_QDOC is reserved for documentation purposes."
    void addLibraryPath( const QString& path );
    void setDefaultPolicy( QLibrary::Policy pol );
    QLibrary::Policy defaultPolicy() const;
    QLibrary* library( const QString& feature ) const;
    QStringList featureList() const;
#endif
};

#endif //QT_NO_COMPONENT

#endif //QPLUGINMANAGER_H
