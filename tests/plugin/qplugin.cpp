#include "qplugin.h"
#include <qdir.h>

#ifdef _WS_WIN_
#include <qt_windows.h>
#endif


QPlugInManager::QPlugInManager()
{
    init();
}

QPlugInManager::QPlugInManager( const QString &path )
{
    init();
    addPlugInPath( path );
}

void QPlugInManager::init()
{
    pLibs.setAutoDelete( TRUE );
    pHnds.setAutoDelete( FALSE );
}

QPlugIn::QPlugIn( HINSTANCE handle )
{
    pHnd = handle;

    createWidget = (CREATEWIDGETPROC) GetProcAddress( pHnd, "createWidget" );
    enumerateWidgets = (ENUMERATEWIDGETSPROC) GetProcAddress( pHnd, "enumerateWidgets" );

    createAction = (CREATEACTIONPROC) GetProcAddress( pHnd, "createAction" );
    enumerateActions = (ENUMERATEACTIONSPROC) GetProcAddress( pHnd, "enumerateActions" );
}

QPlugIn::~QPlugIn()
{
    FreeLibrary( pHnd );
}

bool QPlugInManager::addPlugIn( const QString &fullname )
{
    HINSTANCE dllHandle = LoadLibrary( fullname );
    if ( dllHandle ) {
	QPlugIn* plugin = new QPlugIn( dllHandle );
	pLibs.insert( fullname, plugin );
	if ( plugin->enumerateWidgets && plugin->createWidget ) {
	    QStringList wl = QStringList::split( '\n', plugin->enumerateWidgets() );
	    for ( uint w = 0; w < wl.count(); w++ ) {
		if ( pHnds["WIDGET_"+wl[w]] )
		    qWarning("%s: Widget %s already defined!", fullname.latin1(), wl[w].latin1() );
		else
		    pHnds.insert( "WIDGET_"+wl[w], plugin );
	    }
	}
	if ( plugin->enumerateActions && plugin->createAction ) {
	    QStringList al = QStringList::split( '\n', plugin->enumerateActions() );
	    for ( uint a = 0; a < al.count(); a++ ) {
		if ( pHnds["ACTION_"+al[a]] )
		    qWarning("%s: Action %s already defined!", fullname.latin1(), al[a].latin1() );
		else
		    pHnds.insert( "ACTION_"+al[a], plugin );
	    }
	}
	return TRUE;
    } 
    return FALSE;
}

void QPlugInManager::addPlugInPath( const QString& path )
{
    QStringList plugins = QDir(path).entryList("*.dll *.so");

    for ( uint p = 0; p < plugins.count(); p++ ) {
	QString lib = path + "/" + plugins[p];
	addPlugIn( lib );
    }
}

QWidget* QPlugInManager::newWidget( const QString& classname, QWidget* parent, const char* name, Qt::WFlags f )
{
    QPlugIn* plugin = pHnds[ "WIDGET_"+classname ];
    if ( plugin )
	return plugin->createWidget( classname, parent, name, f );
    else
	return 0;
}

QStringList QPlugInManager::enumerateWidgets()
{
    QStringList list;
    QDictIterator<QPlugIn> it( pLibs );

    while( it.current() ) {
	QStringList widgets = QStringList::split( '\n', it.current()->enumerateWidgets() );
	for ( uint w = 0; w < widgets.count(); w++ )
	    list << widgets[w];
	++it;
    }

    return list;
}

QAction* QPlugInManager::newAction( const QString& actionname, QObject* parent )
{
    QPlugIn* plugin = pHnds[ "ACTION_"+actionname ];
    if ( plugin )
	return plugin->createAction( actionname, parent );
    else
	return 0;
}

QStringList QPlugInManager::enumerateActions()
{
    QStringList list;
    QDictIterator<QPlugIn> it( pHnds );

    while( it.current() ) {
	QStringList actions = QStringList::split( '\n', it.current()->enumerateActions() );
	for ( uint a = 0; a < actions.count(); a++ )
	    list << actions[a];
	++it;
    }

    return list;
}

