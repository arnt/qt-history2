#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qwidgetfactory.h"
#include "qactionfactory.h"
#include "qwindowdefs.h"
#include "qdict.h"
#include <qdir.h>

class QAction;
class QPlugIn;

typedef QDict<QPlugIn> QPlugInDict;

class QPlugIn
{
public:
    enum LibraryPolicy
    { DefaultPolicy,
      OptimizeSpeed,
      ManualPolicy
    };

    QPlugIn( const QString& filename, LibraryPolicy = DefaultPolicy );
    virtual ~QPlugIn();

    virtual bool addToManager( QPlugInDict& dict ) = 0;			// add yourself to manager's dict
    virtual bool removeFromManager( QPlugInDict& dict ) = 0;		// remove yourself from manager's dict
    virtual bool load();
    void unload();

    void setPolicy( LibraryPolicy pol );
    LibraryPolicy policy() const;

    QString library() const;

    const char* infoString();

protected:
    void use();
    void* getSymbolAddress( const QString& );

private:
    typedef const char* (*StringProc)();
    StringProc infoStringPtr;

private:
#ifdef _WS_WIN_
    HINSTANCE pHnd;
#else
    void* pHnd;
#endif
    QString libfile;
    LibraryPolicy libPol;
};

template<class Type>
class QPlugInManager
{
public:
    QPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy pol = QPlugIn::DefaultPolicy )
    : defPol( pol )
    {
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
	    addPlugIn( lib );
	}
    }

    bool removePlugIn( const QString& file )
    {
	if ( libDict.remove( file ) &&  
	     plugin->removeFromManager( plugDict ) )
		return TRUE;
	return FALSE;
    }

    bool addPlugIn( const QString& file )
    {
	if ( libDict[file] )
	    return TRUE;

	Type* plugin = new Type( file, defPol );

	bool result = plugin->addToManager( plugDict );
    
	if ( result ) {
#ifdef CHECK_RANGE
	    if ( libDict[plugin->library()] )
		qWarning("QPlugInManager: Tried to insert %s twice!", plugin->library().latin1() );
#endif
	    libDict.replace( plugin->library(), plugin );
	} else {
	    delete plugin;
	}

	return result;
    }

    QPlugIn* plugIn( const QString& library )
    {
	return libDict[library];
    }

    void setDefaultPolicy( QPlugIn::LibraryPolicy )
    {
	defPol = pol;
    }

    QPlugIn::LibraryPolicy defaultPolicy() const
    {
	return defPol;
    }

    QList<QPlugIn> plugInList() {
	QList<QPlugIn> list;
	QDictIterator<QPlugIn> it( libDict );

	while ( it.current() ) {
#ifdef CHECK_RANGE
	    if ( list.containsRef( it.current() ) )
		qWarning("QPlugInManager: Library %s twice in dictionary!", it.current()->library().latin1() );
#endif
	    list.append( it.current() );
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

#endif // QPLUGIN_H

