/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qinterfacemanager.h#1 $
**
** Definition of QInterfaceManager class
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
#include "qlibrary.h"
#include "qcom.h"
#include "qdict.h"
#include "qmap.h"
#include "qdir.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

#define QT_DEBUG_COMPONENT

template<class Type>
class Q_EXPORT QInterfaceManager
{
public:
    QInterfaceManager( const QUuid& id, const QString& path = QString::null, QLibrary::Policy pol = QLibrary::Delayed, bool cs = TRUE )
	: interfaceId( id ), plugDict( 17, cs ), defPol( pol ), casesens( cs )
    {
	// Every QLibrary object is destroyed on destruction of the manager
	libDict.setAutoDelete( TRUE );
	if ( !path.isEmpty() )
	    addLibraryPath( path );
    }

    void addLibraryPath( const QString& path )
    {
	if ( !QDir( path ).exists( ".", TRUE ) )
	    return;
	
#if defined(Q_OS_WIN32)
	QString filter = "dll";
#elif defined(Q_OS_UNIX)
	QString filter = "so";
#elif defined(Q_OS_MACX)
	QString filter = "dylib";
#endif
	QStringList plugins = QDir(path).entryList( "*." + filter );
	for ( QStringList::Iterator p = plugins.begin(); p != plugins.end(); ++p ) {
	    QString lib = path + "/" + *p;
	    lib = lib.left( lib.length() - filter.length() - 1 );
	    libList.append( lib );

	    if ( defPol == QLibrary::Immediately ) {
		if ( !addLibrary( lib ) )
		    libList.remove( lib );
	    }
	}
    }

    QLibrary* addLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return 0;

	QLibrary *plugin = 0;
	if ( ( plugin = libDict[file] ) )
	    return plugin;

	// Create a library object, and try to get the desired interface
	plugin = new QLibrary( file, defPol );

	bool useful = FALSE;

	Type* iFace = 0;
	plugin->queryInterface( interfaceId, (QUnknownInterface**) &iFace );
	if ( iFace ) {
	    QFeatureListInterface *fliFace = 0;
	    QComponentInterface *cpiFace = 0;
	    iFace->queryInterface( IID_QFeatureListInterface, (QUnknownInterface**)&fliFace );
	    if ( !fliFace )
		plugin->queryInterface( IID_QFeatureListInterface, (QUnknownInterface**)&fliFace );
	    if ( !fliFace ) {
		iFace->queryInterface( IID_QComponentInterface, (QUnknownInterface**)&cpiFace );
		if ( !cpiFace )
		    plugin->queryInterface( IID_QComponentInterface, (QUnknownInterface**)&cpiFace );
	    }
	    QStringList fl;
	    if ( fliFace )
		// Map all found features to the library
		fl = fliFace->featureList();
	    else if ( cpiFace )
		fl << cpiFace->name();

	    for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ ) {
		useful = TRUE;
#ifdef QT_DEBUG_COMPONENT
		qDebug("Adding feature %s", (*f).latin1() );
#endif
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
	    QComponentInterface *cpiFace = 0;
	    fliFace = (QFeatureListInterface*)iFace->queryInterface( IID_QFeatureListInterface, (QUnknownInterface**)&iFace );
	    if ( !fliFace )
		plugin->queryInterface( IID_QFeatureListInterface, (QUnknownInterface**)&fliFace );
	    if ( !fliFace ) {
		iFace->queryInterface( IID_QComponentInterface, (QUnknownInterface**)&cpiFace );
		if ( !cpiFace )
		    plugin->queryInterface( IID_QComponentInterface, (QUnknownInterface**)&cpiFace );
	    }
	    QStringList fl;
	    if ( fliFace )
		fl = iFace->featureList();
	    else if ( cpiFace )
		fl << cpiFace->name();

	    for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ )
		plugDict.remove( *f );

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

    void setDefaultPolicy( QLibrary::Policy pol )
    {
	defPol = pol;
    }

    QLibrary::Policy defaultPolicy() const
    {
	return defPol;
    }

    QLibrary* library( const QString& feature ) const
    {
	if ( feature.isEmpty() )
	    return 0;

	// We already have a QLibrary object for this feature
	QLibrary *library = 0;
	if ( ( library = plugDict[feature] ) )
	    return library;

	// Find the filename that matches the feature request best
	QMap<int, QStringList> map;
	QStringList::ConstIterator it = libList.begin();
	int best = 0;
	int worst = 15;
	while ( it != libList.end() ) {
	    QString lib = *it;
	    lib = lib.right( lib.length() - lib.findRev( "/" ) - 1 );
	    lib = lib.left( lib.findRev( "." ) );
	    int s = feature.similarityWith( lib );
	    if ( s < worst )
		worst = s;
	    if ( s > best )
		best = s;
	    map[s].append( *it );
	    ++it;
	}

	// Start with the best match to get the library object
	QInterfaceManager<Type> *that = (QInterfaceManager<Type>*)this;
	for ( int s = best; s >= worst; --s ) {
	    QStringList group = map[s];
	    QStringList::Iterator git = group.begin();
	    while ( git != group.end() ) {
		QString lib = *git;
		++git;
		if ( that->addLibrary( lib ) && ( library = plugDict[feature] ) )
		    return library;
	    }
	}

	return 0;
    }

    QRESULT queryInterface(const QString& feature, QUnknownInterface** iface) const
    {
	QLibrary* plugin = 0;
	plugin = library( feature );

	if ( plugin )
	    plugin->queryInterface( interfaceId, iface );
    }

    QStringList featureList() const
    {
	// Make sure that all libraries have been loaded once.
	QInterfaceManager<Type> *that = (QInterfaceManager<Type>*)this;
	QStringList::ConstIterator it = libList.begin();
	while ( it != libList.end() ) {
	    QString lib = *it;
	    ++it;
	    that->addLibrary( lib );
	}

	QStringList list;
	QDictIterator<QLibrary> pit( plugDict );
	while( pit.current() ) {
	    list << pit.currentKey();
	    ++pit;
	}

	return list;
    }

private:
    QUuid interfaceId;
    QDict<QLibrary> plugDict;	    // Dict to match feature with library
    QDict<QLibrary> libDict;	    // Dict to match library file with library
    QStringList libList;

    QLibrary::Policy defPol;
    uint casesens : 1;
};

#endif //QT_NO_COMPONENT

#endif //QINTERFACEMANAGER_H
