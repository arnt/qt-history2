#ifndef QPLUGINMANAGER_H
#define QPLUGINMANAGER_H

#include <qdict.h>
#include <qdir.h>
#include <qapplication.h>

typedef QDict<QPlugIn> QPlugInDict;

template<class Type>
class QPlugInManager : protected QAbstractPlugInManager
{
public:
    QPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy pol = QPlugIn::Default )
	: defPol( pol )
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
/*	if ( plugin && plugin->removeFromManager( plugDict ) && libDict.remove( file ) )
		return TRUE;*/
	if ( plugin && removePlugIn( plugin ) && libDict.remove( file ) )
		return TRUE;
	return FALSE;
    }

    Type* addLibrary( const QString& file )
    {
	if ( libDict[file] )
	    return 0;

	Type* plugin = new Type( file, defPol );

//	bool result = plugin->addToManager( plugDict );
	bool result = addPlugIn( plugin );

	if ( result ) {
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