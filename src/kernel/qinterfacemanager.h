/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpluginmanager.h#1 $
**
** Definition of QPlugInManager class
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

#ifndef QINTERFACEMANAGER_H
#define QINTERFACEMANAGER_H

#ifndef QT_H
#include "qplugin.h"
#include "qdict.h"
#include "qdir.h"
#include "qmap.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QApplicationInterface;

template<class Type>
class Q_EXPORT QInterfaceManager : public Type
{
public:
    QInterfaceManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
	 QApplicationInterface* appIface = 0, QPlugIn::LibraryPolicy pol = QPlugIn::Default )
	: defPol( pol ), appInterface( appIface )
    {
	// Every library is unloaded on destruction of the manager
	libDict.setAutoDelete( TRUE );
	if ( !path.isEmpty() )
	    addLibraryPath( path, filter );
    }

    void addLibraryPath( const QString& path, const QString& filter = "*.dll; *.so" )
    {
	if ( !QDir( path ).exists( ".", TRUE ) )
	    return;

	QStringList plugins = QDir(path).entryList( filter );

	for ( uint p = 0; p < plugins.count(); p++ ) {
	    QString lib = path + "/" + plugins[p];
	    addLibrary( lib );
	}
    }

    QPlugIn* addLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return 0;

	if ( libDict[file] )
	    return 0;

	QPlugIn* plugin = new QPlugIn( file, appInterface, defPol );
	bool useful = FALSE;

	if ( plugin->load() ) {
	    Type* iFace = (Type*)plugin->queryInterface( interfaceID() );
	    if ( iFace ) {
		QStringList fl = iFace->featureList();
		for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ ) {
		    useful = TRUE;
#ifdef CHECK_RANGE
		    if ( !plugDict[*f] )
			plugDict.replace( *f, plugin );
		    else
			qWarning("%s: Feature %s already defined!", plugin->library().latin1(), (*f).latin1() );
#else
		    plugDict.replace( *f, plugin );
#endif
		}
		iFace->release();
	    }
	}

	if ( useful ) {
	    libDict.replace( plugin->library(), plugin );
	    return plugin;
	} else {
	    delete plugin;
	    return 0;
	}	
    }

    bool removeLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return FALSE;

	QPlugIn* plugin = libDict[ file ];
	if ( !plugin )
	    return FALSE;

	Type *iFace = (Type*)plugin->queryInterface( interfaceID() );
	if ( iFace && iFace->interfaceID() == interfaceID() ) {
	    QStringList fl = iFace->featureList();
	    for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ ) {
		plugDict.remove( *f );
	    }
	    if ( !iFace->release() )
		return FALSE;
	}
	bool unloaded = plugin->unload();

	if ( !libDict.remove( file ) ) {
	    delete plugin;
	    return FALSE;
	}

	return unloaded;
    }

    void setDefaultPolicy( QPlugIn::LibraryPolicy pol )
    {
	defPol = pol;
    }

    QPlugIn::LibraryPolicy defaultPolicy() const
    {
	return defPol;
    }

    Type *queryInterface(const QString& feature) const
    {
	if ( feature.isEmpty() )
	    return 0;
	QPlugIn* plugin = plugDict[feature];
	if ( !plugin )
	    return 0;
	return (Type*)plugin->queryInterface( interfaceID() );
    }

    QPlugIn* plugIn( const QString& feature ) const
    {
	if ( feature.isEmpty() )
	    return 0;
	return plugDict[feature];
    }

    QStringList featureList() const
    {
	QStringList list;
	QDictIterator<QPlugIn> it( plugDict );

	while( it.current() ) {
	    list << it.currentKey();
	    ++it;
	}

	return list;
    }

    QPlugIn* plugInFromFile( const QString& fileName ) const
    {
	if ( fileName.isEmpty() )
	    return 0;
	return libDict[fileName];
    }

    QStringList libraryList() const
    {
	QStringList list;
	QDictIterator<QPlugIn> it( libDict );

	while ( it.current() ) {
	    list << it.currentKey();
	    ++it;
	}

	return list;
    }

    bool selectFeature( const QString& feature )
    {
	QPlugIn* plugin = 0;
	if ( !!feature )
	    plugin = plugDict[feature];

	QDictIterator<QPlugIn> it( libDict );
	while ( it.current() ) {
	    if ( it.current() == plugin && !it.current()->loaded() )
		it.current()->load();
	    else if ( it.current() != plugin && it.current()->loaded() )
		it.current()->unload();
	    ++it;
	}

	return plugin != 0;
    }

    bool unloadFeature( const QString& feature )
    {
	QPlugIn *plugin = 0;
	if ( feature.isEmpty() || !(plugin = plugDict[feature]) )
	    return FALSE;

	if ( plugin->loaded() )
	    return plugin->unload();
	return TRUE;
    }

private:
    QDict<QPlugIn> plugDict;	    // Dict to match feature with plugin
    QDict<QPlugIn> libDict;	    // Dict to match library file with plugin

    QPlugIn::LibraryPolicy defPol;
    QApplicationInterface* appInterface;
};

#endif

#endif //QINTERFACEMANAGER_H
