#ifndef QPLUGINMANAGER_H
#define QPLUGINMANAGER_H

#include <qdict.h>
#include <qdir.h>
#include <qapplication.h>

typedef QDict<QPlugIn> QPlugInDict;

template<class Type>
class QPlugInManager : protected QObject
{
public:
    QPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy pol = QPlugIn::Default )
	: QObject( qApp, path ), defPol( pol )
    {
	// Every library is unloaded on destruction of the manager
	libDict.setAutoDelete( TRUE );
	plugDict.setAutoDelete( FALSE );
	if ( !path.isEmpty() )
	    addPlugInPath( path );
    }

    virtual ~QPlugInManager()
    {
    }

    virtual void addPlugInPath( const QString& path, const QString& filter = "*.dll; *.so" )
    {
	QStringList plugins = QDir(path).entryList( filter );

	for ( uint p = 0; p < plugins.count(); p++ ) {
	    QString lib = path + "/" + plugins[p];
	    addLibrary( lib );
	}
    }

    bool removeLibrary( const QString& file )
    {
	Type* plugin = (Type*)(libDict[ file ]);
	if ( !plugin )
	    return FALSE;

	QStringList al = ((QPlugIn*)plugin)->featureList();
	for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ )
	    plugDict.remove( *a );

	if ( !libDict.remove( file ) )
	    return FALSE;


	return TRUE;
    }

    Type* addLibrary( const QString& file )
    {
	if ( libDict[file] )
	    return 0;

	Type* plugin = new Type( file, defPol );

	bool useful = FALSE;
	QStringList al = ((QPlugIn*)plugin)->featureList();
	for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ ) {
	    useful = TRUE;
#ifdef CHECK_RANGE
	    if ( plugDict[*a] )
		qWarning("%s: Action %s already defined!", plugin->library().latin1(), (*a).latin1() );
	    else
#endif
		plugDict.insert( *a, plugin );
	}

	if ( useful ) {
#ifdef CHECK_RANGE
	    if ( libDict[plugin->library()] )
		qWarning( "QPlugInManager: Can't manage library twice! (%s)", plugin->library().latin1() );
#endif
	    libDict.replace( plugin->library(), plugin );
	} else {
	    delete plugin;
	    return 0;
	}

	return plugin;
    }

    void setDefaultPolicy( QPlugIn::LibraryPolicy pol )
    {
	defPol = pol;
    }

    QPlugIn::LibraryPolicy defaultPolicy() const
    {
	return defPol;
    }

    QPlugIn *plugIn( const QString &className )
    {
	return plugDict[className];
    }

    QPlugIn* plugInFromFile( const QString& fileName )
    {
	return libDict[fileName];
    }

    QList<QPlugIn> plugInList()
    {
	QList<QPlugIn> list;
	QDictIterator<QPlugIn> it( libDict );

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

	QDictIterator<QPlugIn> it( libDict );
	while ( it.current() ) {
	    list << it.currentKey();
	    ++it;
	}

	return list;
    }

protected:
    QPlugInDict plugDict;	    // Dict to match requested interface with plugin
    QPlugInDict libDict;	    // Dict to match library file with plugin

private:
    QPlugIn::LibraryPolicy defPol;
};

#endif //QPLUGINMANAGER_H