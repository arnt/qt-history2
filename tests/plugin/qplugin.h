#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qwidgetfactory.h"
#include "qactionfactory.h"
#include "qwindowdefs.h"
#include "qdict.h"
#include <qdir.h>

#ifndef HINSTANCE
#define HINSTANCE void*
#endif

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

    virtual bool addToManager( QPlugInDict& dict ) = 0;
    virtual bool load();
    void unload();

    void setPolicy( LibraryPolicy pol );
    LibraryPolicy policy() const;

    QString library() const;

    virtual const char* infoString();

protected:
    void use();
    void* getSymbolAddress( const QString& );

    typedef const char* (*STRINGPROC)();
    STRINGPROC infoStringPtr;

private:
    HINSTANCE pHnd;
    QString libfile;

    LibraryPolicy libPol;
};

template<class Type>
class QPlugInManager //: public QWidgetFactory, public QActionFactory
{
public:
    QPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy pol = QPlugIn::DefaultPolicy )
    : defPol( pol )
    {
	pLibs.setAutoDelete( TRUE );
	pHnds.setAutoDelete( FALSE );
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

    bool addPlugIn( const QString& file )
    {
	if ( pLibs[file] )
	    return TRUE;

	Type* plugin = new Type( file, defPol );

	bool result = plugin->addToManager( pHnds );
    
	if ( result )
	    pLibs.insert( plugin->library(), plugin );
	else
	    delete plugin;

	return result;
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
	QDictIterator<QPlugIn> it( pLibs );

	while ( it.current() ) {
	    list.append( it.current() );
	    ++it;
	}
	return list;
    }

protected:
    QPlugInDict pHnds;

private:
    QPlugInDict pLibs;
    QPlugIn::LibraryPolicy defPol;
};

#endif // QPLUGIN_H

