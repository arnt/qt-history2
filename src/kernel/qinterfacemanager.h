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
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QApplicationInterface;

#if 1 // internal class to provide signal functionality to QPlugInManager
class Q_EXPORT QInterfaceManagerSignalEmitter : public QObject
{
    Q_OBJECT
public:
    QInterfaceManagerSignalEmitter() : QObject() {}

signals:
    void featureAdded( const QString& );
    void featureRemoved( const QString& );
};
#endif

template<class Type>
class Q_EXPORT QInterfaceManager : public QInterfaceManagerSignalEmitter, public Type
{
public:
    QInterfaceManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
	 QApplicationInterface* appIface = 0, QPlugIn::LibraryPolicy pol = QPlugIn::Default )
	: appInterface( appIface ), defPol( pol )
    {
	// Every library is unloaded on destruction of the manager
	libDict.setAutoDelete( TRUE );
	plugDict.setAutoDelete( FALSE );
	if ( !path.isEmpty() )
	    addPlugInPath( path, filter );
    }

    ~QInterfaceManager()
    {
	QDictIterator<Type> it (plugDict);
	while( it.current() ) {
	    emit featureRemoved( it.currentKey() );
	    ++it;
	}
    }

    void addPlugInPath( const QString& path, const QString& filter = "*.dll; *.so" )
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
	    QStringList iFaces = plugin->interfaceList();
	    for ( QStringList::Iterator i = iFaces.begin(); i != iFaces.end(); ++i ) {
		QUnknownInterface *iFace = plugin->queryInterface( *i );
		if ( iFace && iFace->interfaceID() == interfaceID() ) {
		    Type* typedIFace = (Type*)iFace;
		    QStringList al = typedIFace->featureList();
		    for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ ) {
			useful = TRUE;
			if ( !plugDict[*a] ) {
			    plugDict.insert( *a, typedIFace );
			    emit featureAdded( *a );
			}
#ifdef CHECK_RANGE
			else
			    qWarning("%s: Feature %s already defined!", plugin->library().latin1(), (*a).latin1() );
#endif
		    }
		} else {
		    delete iFace;
		}
	    }
	}

	if ( useful ) {
	    libDict.replace( plugin->library(), plugin );
	    return plugin;
	} else {
//	    delete plugin;
	    return 0;
	}	
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
		signalEmitter->emit featureRemoved( *a );
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

    Type *operator[](const QString& feature) const
    {
	if ( feature.isEmpty() )
	    return 0;
	return (Type*)plugDict[feature];
    }

    QPlugIn* plugIn( const QString& fileName )
    {
	if ( fileName.isEmpty() )
	    return 0;
	return libDict[fileName];
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
	Type* iface = 0;
	if ( !!feat )
	    iface = queryInterface( feat );
	QDictIterator<Type> it( libDict );

	while ( it.current() ) {
	    if ( it.current() == iface && ! it.current()->loaded() )
		it.current()->load();
	    else if ( it.current() != iface && it.current()->loaded() )
		it.current()->unload();
	    ++it;
	}

	return plugin != 0;
    }

    bool unloadFeature( const QString& feat )
    {
	Type* iface = queryInterface( feat );
	if ( !iface )
	    return FALSE;
	if ( iface->loaded() )
	    return iface->unload();
	return TRUE;
    }

private:
    QDict<Type> plugDict;	    // Dict to match requested feature with plugin
    QDict<QPlugIn> libDict;	    // Dict to match library file with plugin

    QPlugIn::LibraryPolicy defPol;
    QApplicationInterface* appInterface;
};

#endif

#endif //QINTERFACEMANAGER_H
