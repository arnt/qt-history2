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

#ifndef QPLUGINMANAGER_H
#define QPLUGINMANAGER_H

#ifndef QT_H
#include "qplugin.h"
#include "qdict.h"
#include "qdir.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QApplicationInterface;

#if 1 // internal class to provide signal functionality to QPlugInManager
class Q_EXPORT QPlugInManagerSignalEmitter : public QObject
{
    Q_OBJECT
public:
    QPlugInManagerSignalEmitter() : QObject() {}

    void emitFeatureAdded( const QString& feature ) { emit featureAdded( feature ); }
    void emitFeatureRemoved( const QString& feature ) { emit featureRemoved( feature ); }

signals:
    void featureAdded( const QString& );
    void featureRemoved( const QString& );
};
#endif

template<class Type>
class Q_EXPORT QPlugInManager
{
public:
    QPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
	 QApplicationInterface* appIface = 0, QPlugIn::LibraryPolicy pol = QPlugIn::Default )
	: appInterface( appIface ), defPol( pol )
    {
	signalEmitter = new QPlugInManagerSignalEmitter;
	// Every library is unloaded on destruction of the manager
	libDict.setAutoDelete( TRUE );
	plugDict.setAutoDelete( FALSE );
	if ( !path.isEmpty() )
	    addPlugInPath( path, filter );
    }

    virtual ~QPlugInManager()
    {
	QDictIterator<Type> it (plugDict);
	while( it.current() ) {
	    signalEmitter->emitFeatureRemoved( it.currentKey() );
	    ++it;
	}
	delete signalEmitter;
    }

    bool connect( const char* signal, QObject* receiver, const char* slot )
    {
	return QObject::connect( signalEmitter, signal, receiver, slot );
    }

    virtual void addPlugInPath( const QString& path, const QString& filter = "*.dll; *.so" )
    {
	if ( !QDir( path ).exists( ".", TRUE ) )
	    return;

	QStringList plugins = QDir(path).entryList( filter );

	for ( uint p = 0; p < plugins.count(); p++ ) {
	    QString lib = path + "/" + plugins[p];
	    addLibrary( lib );
	}
    }

    Type* addLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return 0;

	if ( libDict[file] )
	    return 0;

	Type* plugin = new Type( file, appInterface, defPol );
	bool useful = FALSE;

	if ( plugin->load() ) {
	    QStringList al = ((QPlugIn*)plugin)->featureList();
	    for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ ) {
		useful = TRUE;
		if ( !plugDict[*a] ) {
		    plugDict.insert( *a, plugin );
		    signalEmitter->emitFeatureAdded( *a );
		}
#ifdef CHECK_RANGE
		else
		    qWarning("%s: Feature %s already defined!", plugin->library().latin1(), (*a).latin1() );
#endif
	    }
	}

	if ( useful ) {
	    libDict.replace( plugin->library(), plugin );
	} else {
	    delete plugin;
	    return 0;
	}

	return plugin;
    }

    bool removeLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return FALSE;

	Type* plugin = libDict[ file ];
	if ( !plugin )
	    return FALSE;

	{
	    QStringList al = ((QPlugIn*)plugin)->featureList();
	    for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ ) {
		plugDict.remove( *a );
		signalEmitter->emitFeatureRemoved( *a );
	    }
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

    void setDefaultFunction( const char* fn )
    {
	defFunction = fn;
    }

    const char* defaultFunction() const
    {
	return defFunction;
    }

    Type *plugIn( const QString &feature )
    {
	if ( feature.isEmpty() )
	    return 0;
	return (Type*)plugDict[feature];
    }

    Type* plugInFromFile( const QString& fileName )
    {
	if ( fileName.isEmpty() )
	    return 0;
	return libDict[fileName];
    }

    QList<Type> plugInList()
    {
	QList<Type> list;
	QDictIterator<Type> it( libDict );

	while ( it.current() ) {
#ifdef CHECK_RANGE
	    if ( list.containsRef( it.current() ) )
		qWarning("QPlugInManager: Library %s added twice!", it.current()->library().latin1() );
#endif
	    list.append( it.current() );
	    ++it;
	}
	return list;
    }

    QStringList libraryList()
    {
	QStringList list;

	QDictIterator<Type> it( libDict );
	while ( it.current() ) {
	    list << it.currentKey();
	    ++it;
	}

	return list;
    }

    QStringList featureList()
    {
	QStringList list;
	QDictIterator<Type> it (plugDict);

	while( it.current() ) {
	    list << it.currentKey();
	    ++it;
	}

	return list;
    }

    bool selectFeature( const QString& feat )
    {
	Type* plugin = 0;
	if ( !feat.isNull() )
	    plugin = plugIn( feat );
	QDictIterator<Type> it( libDict );

	while ( it.current() ) {
	    if ( it.current() == plugin && ! it.current()->loaded() )
		it.current()->load();
	    else if ( it.current() != plugin && it.current()->loaded() )
		it.current()->unload();
	    ++it;
	}

	return plugin != 0;
    }

    bool unloadFeature( const QString& feat )
    {
	Type* plugin = plugIn( feat );
	if ( !plugin )
	    return FALSE;
	if ( plugin->loaded() )
	    return plugin->unload();
	return TRUE;
    }

private:
    QPlugInManagerSignalEmitter* signalEmitter;
    QApplicationInterface* appInterface;
    QDict<Type> plugDict;	    // Dict to match requested feature with plugin
    QDict<Type> libDict;	    // Dict to match library file with plugin

    QPlugIn::LibraryPolicy defPol;
};

#endif

#endif //QPLUGINMANAGER_H
