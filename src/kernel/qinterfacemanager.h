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
#include "qdict.h"
#include "qdir.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

template<class Type>
class Q_EXPORT QInterfaceManager
{
public:
    QInterfaceManager( const QString& id, const QString& path = QString::null, const QString& filter = "*.dll; *.so", QLibrary::Policy pol = QLibrary::Default )
	: interfaceId( id ), defPol( pol )
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

	for ( QStringList::Iterator p = plugins.begin(); p != plugins.end(); ++p ) {
	    QString lib = path + "/" + *p;
	    addLibrary( lib );
	}
    }

    QLibrary* addLibrary( const QString& file )
    {
	if ( file.isEmpty() )
	    return 0;

	if ( libDict[file] )
	    return 0;

	QLibrary* plugin = new QLibrary( file, defPol );
	bool useful = FALSE;

	Type* iFace = (Type*)plugin->queryInterface( interfaceId );
	if ( iFace ) {
	    QStringList fl = iFace->featureList();
	    for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ ) {
		useful = TRUE;
#ifdef QT_CHECK_RANGE
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
	if ( defPol != QLibrary::OptimizeSpeed )
	    plugin->unload();

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

	QLibrary* plugin = libDict[ file ];
	if ( !plugin )
	    return FALSE;

	Type *iFace = (Type*)plugin->queryInterface( interfaceId );
	if ( iFace ) {
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

    void setDefaultPolicy( QLibrary::Policy pol )
    {
	defPol = pol;
    }

    QLibrary::Policy defaultPolicy() const
    {
	return defPol;
    }

    Type *queryInterface(const QString& feature) const
    {
	if ( feature.isEmpty() )
	    return 0;
	QLibrary* plugin = plugDict[feature];
	if ( !plugin )
	    return 0;
	return (Type*)plugin->queryInterface( interfaceId );
    }

    QLibrary* plugIn( const QString& feature ) const
    {
	if ( feature.isEmpty() )
	    return 0;
	return plugDict[feature];
    }

    QStringList featureList() const
    {
	QStringList list;
	QDictIterator<QLibrary> it( plugDict );

	while( it.current() ) {
	    list << it.currentKey();
	    ++it;
	}

	return list;
    }

    QLibrary* plugInFromFile( const QString& fileName ) const
    {
	if ( fileName.isEmpty() )
	    return 0;
	return libDict[fileName];
    }

    QStringList libraryList() const
    {
	QStringList list;
	QDictIterator<QLibrary> it( libDict );

	while ( it.current() ) {
	    list << it.currentKey();
	    ++it;
	}

	return list;
    }

private:
    QString interfaceId;
    QDict<QLibrary> plugDict;	    // Dict to match feature with library
    QDict<QLibrary> libDict;	    // Dict to match library file with library

    QLibrary::Policy defPol;
};

#endif //QT_NO_COMPONENT

#endif //QINTERFACEMANAGER_H
