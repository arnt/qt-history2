#include "qplugin.h"
#include <qdir.h>
#include <qdict.h>
#ifdef _WS_WIN_
#include <qt_windows.h>
#endif

class QPlugIn
{
    friend class QPlugInManager;
public:
    QPlugIn( HINSTANCE pHnd );
    ~QPlugIn();

private:
    HINSTANCE pHnd;

    typedef QWidget* (*CREATEWIDGETPROC)(const QString&, QWidget* = 0, const char* = 0, Qt::WFlags = 0 );
    typedef const char* (*ENUMERATEWIDGETSPROC)();

    typedef QWidget* (*PROCESSFILEPROC)( QFile* f, bool& ok );
    typedef const char* (*ENUMERATEFILETYPESPROC)();

    typedef QAction* (*CREATEACTIONPROC)(const QString&, QObject* = 0 );
    typedef const char* (*ENUMERATEACTIONSPROC)();

    CREATEWIDGETPROC createWidget;
    ENUMERATEWIDGETSPROC enumerateWidgets;

    PROCESSFILEPROC processFile;
    ENUMERATEFILETYPESPROC enumerateFileTypes;

    CREATEACTIONPROC createAction;
    ENUMERATEACTIONSPROC enumerateActions;
};

QPlugIn::QPlugIn( HINSTANCE handle )
{
    pHnd = handle;

    createWidget = (CREATEWIDGETPROC) GetProcAddress( pHnd, "createWidget" );
    enumerateWidgets = (ENUMERATEWIDGETSPROC) GetProcAddress( pHnd, "enumerateWidgets" );

    processFile = (PROCESSFILEPROC) GetProcAddress( pHnd, "processFile" );
    enumerateFileTypes = (ENUMERATEFILETYPESPROC) GetProcAddress( pHnd, "enumerateFileTypes" );
    
    createAction = (CREATEACTIONPROC) GetProcAddress( pHnd, "createAction" );
    enumerateActions = (ENUMERATEACTIONSPROC) GetProcAddress( pHnd, "enumerateActions" );
}

QPlugIn::~QPlugIn()
{
    FreeLibrary( pHnd );
}

static QDict<QPlugIn> pHnds;
static QDict<QPlugIn> pLibs;

/*! \class QPlugInManager qplugin.h
    \brief Implementation of QWidgetFactory and QActionFactory for plugin support.
*/

/*!
  Creates an empty plugin manager.
*/
QPlugInManager::QPlugInManager()
{
    init();
}

/*!
  Creates a plugin manager.
  The manager looks up and loads all shared libraries in \a path.

  \sa addPlugInPath(), addPlugIn()
*/
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

/*!
  Loads the shared library \a file and registers all provided
  widgets and actions.
*/
bool QPlugInManager::addPlugIn( const QString &file )
{
    HINSTANCE dllHandle = LoadLibrary( file );
    bool useful = FALSE;
    if ( dllHandle ) {
	QPlugIn* plugin = new QPlugIn( dllHandle );
	if ( pLibs[file] )
	    return FALSE;

	if ( plugin->enumerateWidgets && plugin->createWidget ) {
	    QStringList wl = QStringList::split( QRegExp("\n"), plugin->enumerateWidgets() );
	    for ( uint w = 0; w < wl.count(); w++ ) {
		if ( pHnds["WIDGET_"+wl[w]] )
		    qWarning("%s: Widget %s already defined!", file.latin1(), wl[w].latin1() );
		else
		    pHnds.insert( "WIDGET_"+wl[w], plugin );
	    }
	    useful = TRUE;
	}
	if ( plugin->enumerateActions && plugin->createAction ) {
	    QStringList al = QStringList::split( QRegExp("\n"), plugin->enumerateActions() );
	    for ( uint a = 0; a < al.count(); a++ ) {
		if ( pHnds["ACTION_"+al[a]] )
		    qWarning("%s: Action %s already defined!", file.latin1(), al[a].latin1() );
		else
		    pHnds.insert( "ACTION_"+al[a], plugin );
	    }
	    useful = TRUE;
	}
	if ( plugin->enumerateFileTypes && plugin->processFile ) {
	    QStringList fl = QStringList::split( QRegExp("\n"), plugin->enumerateFileTypes() );
	    for ( uint f = 0; f < fl.count(); f++ ) {
		if ( pHnds["FILE_"+fl[f]] )
		    qWarning("%s: File %s already supported!", file.latin1(), fl[f].latin1() );
		else
		    pHnds.insert( "FILE_"+fl[f].lower(), plugin );
	    }
	    useful = TRUE;
	}
	if ( useful )
	    pLibs.insert( file, plugin );
	else
	    delete plugin;	    
    } 
    return useful;
}

/*!
  Tries to add all shared libraries in \a path.
*/
void QPlugInManager::addPlugInPath( const QString& path )
{
    QStringList plugins = QDir(path).entryList("*.dll *.so");

    for ( uint p = 0; p < plugins.count(); p++ ) {
	QString lib = path + "/" + plugins[p];
	addPlugIn( lib );
    }
}

/*! \reimp
*/
QWidget* QPlugInManager::newWidget( const QString& classname, QWidget* parent, const char* name, Qt::WFlags f )
{
    QPlugIn* plugin = pHnds[ "WIDGET_"+classname ];
    if ( plugin )
	return plugin->createWidget( classname, parent, name, f );
    return 0;
}

/*!
  Returns a list of all widget classes supported by loaded
  plugins.
*/
QStringList QPlugInManager::enumerateWidgets()
{
    QStringList list;
    QDictIterator<QPlugIn> it( pLibs );

    while( it.current() ) {
	QPlugIn* plugin = it.current();
	if ( plugin->enumerateWidgets ) {
	    QStringList widgets = QStringList::split( QRegExp("\n"), plugin->enumerateWidgets() );
	    for ( uint w = 0; w < widgets.count(); w++ )
		list << widgets[w];
	}
	++it;
    }

    return list;
}

/*! \reimp
*/

QStringList QPlugInManager::enumerateFileTypes()
{
    QStringList list;
    QDictIterator<QPlugIn> it( pLibs );

    while ( it.current() ) {
	QPlugIn* plugin = it.current();
	if ( plugin->enumerateFileTypes ) {
	    QStringList ft = QStringList::split( QRegExp("\n"), it.current()->enumerateFileTypes() );
	    for ( uint f = 0; f < ft.count(); f++ )
		list << ft[f].lower();
	}
	++it;
    }

    return list;
}

/*! \reimp
*/
QWidget* QPlugInManager::processFile( QFile* file, bool &ok )
{
    QString filename = file->name();
    QString fileext = "";
    int extpos = filename.findRev('.');
    if ( extpos != -1 )
	fileext = filename.right( filename.length() - extpos );

    QPlugIn* plugin = pHnds[ "FILE_"+fileext ];
    if ( plugin )
	return plugin->processFile( file, ok );

    return 0;
}

/*! \reimp
*/
QAction* QPlugInManager::newAction( const QString& actionname, QObject* parent )
{
    QPlugIn* plugin = pHnds[ "ACTION_"+actionname ];
    if ( plugin )
	return plugin->createAction( actionname, parent );
    return 0;
}

/*!
  Returns a list of all action names supported by loaded
  plugins.
*/
QStringList QPlugInManager::enumerateActions()
{
    QStringList list;
    QDictIterator<QPlugIn> it( pHnds );

    while( it.current() ) {
	QPlugIn* plugin = it.current();
	if ( plugin->enumerateActions ) {
    	    QStringList actions = QStringList::split( QRegExp("\n"), plugin->enumerateActions() );
	    for ( uint a = 0; a < actions.count(); a++ )
		list << actions[a];
	}
	++it;
    }

    return list;
}

