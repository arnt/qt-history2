#include "qdefaultplugin.h"

/*! 
  \class QDefaultPlugIn qdefaultplugin.h
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QDefaultPlugIn::QDefaultPlugIn( const QString& file, LibraryPolicy pol )
: QPlugIn( file, pol )
{
}

/*! \reimp
*/
bool QDefaultPlugIn::load()
{
    if ( !QPlugIn::load() )
	return FALSE;

    createWidgetPtr = (CREATEWIDGETPROC) getSymbolAddress( "createWidget" );
    enumerateWidgetsPtr = (STRINGPROC) getSymbolAddress( "enumerateWidgets" );

    processFilePtr = (PROCESSFILEPROC) getSymbolAddress( "processFile" );
    enumerateFileTypesPtr = (STRINGPROC) getSymbolAddress( "enumerateFileTypes" );

    createActionPtr = (CREATEACTIONPROC) getSymbolAddress( "createAction" );
    enumerateActionsPtr = (STRINGPROC) getSymbolAddress( "enumerateActions" );

    return TRUE;
}

/*! \reimp
*/
bool QDefaultPlugIn::addToManager( QPlugInDict& dict )
{
    bool useful = FALSE;

    QStringList wl = QStringList::split( QRegExp("[;\\s]"), enumerateWidgets() );
    for ( QStringList::Iterator w = wl.begin(); w != wl.end(); w++ ) {
	useful = TRUE;
#ifdef CHECK_RANGE
	if ( dict[*w] )
	    qWarning("%s: Widget %s already defined!", library().latin1(), (*w).latin1() );
	else
#endif
	    dict.insert( *w, this );
    }

    QStringList al = QStringList::split( QRegExp("[;\\s]"), enumerateActions() );
    for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ ) {
	useful = TRUE;
#ifdef CHECK_RANGE
	if ( dict[*a] )
	    qWarning("%s: Action %s already defined!", library().latin1(), (*a).latin1() );
	else
#endif
	    dict.insert( *a, this );
    }

    QString filter = enumerateFileTypes();
    filter.replace( QRegExp("\n"), ";;" );
    QStringList fl = QStringList::split( ";;", filter );
    for ( QStringList::Iterator f = fl.begin(); f != fl.end(); f++ ) {
	useful = TRUE;
#ifdef CHECK_RANGE
	if ( dict[*f] )
	    qWarning("%s: File %s already supported!", library().latin1(), (*f).latin1() );
	else
#endif
	    dict.insert( *f, this );
    }

    return useful;
}

/*!
  Calls the appropriate plugin's createWidget-method and returns the result.
*/
QWidget* QDefaultPlugIn::createWidget( const QString& classname, bool init, QWidget* parent, const char* name )
{
    use();

    if ( createWidgetPtr )
	return createWidgetPtr( classname, init, parent, name );
    return 0;
}

/*!
  Calls the appropriate plugin's enumerateWidgets-method and returns the result.
*/
const char* QDefaultPlugIn::enumerateWidgets()
{
    use();

    if ( enumerateWidgetsPtr )
	return enumerateWidgetsPtr();
    return "";
}

/*!
  Calls the appropriate plugin's processFile-method and returns the result.
*/
QWidget* QDefaultPlugIn::processFile( QIODevice *f, const QString& filetype )
{
    use();

    if ( processFilePtr )
	return processFilePtr( f, filetype );
    return 0;
}

/*!
  Calls the appropriate plugin's enumerateFileTypes-method and returns the result.
*/
const char* QDefaultPlugIn::enumerateFileTypes()
{
    use();

    if ( enumerateFileTypesPtr )
	return enumerateFileTypesPtr();
    return "";
}

/*!
  Calls the appropriate plugin's createAction-method and returns the result.
*/
QAction* QDefaultPlugIn::createAction( const QString& actionname, bool& self, QObject *parent )
{
    use();

    if ( createActionPtr )
	return createActionPtr( actionname, self, parent );
    return 0;
}

/*!
  Calls the appropriate plugin's enumerateActions-method and returns the result.
*/
const char* QDefaultPlugIn::enumerateActions()
{
    use();

    if ( enumerateActionsPtr )
	return enumerateActionsPtr();
    return "";
}

/*!
  \class QDefaultPlugInManager qdefaultpluginmanager.h

  \brief Implements a QPlugInManager that handles plugins for custom widgets, actions and filetypes

  \sa QPlugInManager
*/

/*!
  Creates a QDefaultPlugInManager.

  \sa QPlugInManager
*/
QDefaultPlugInManager::QDefaultPlugInManager( const QString& path, QPlugIn::LibraryPolicy pol )
: QPlugInManager<QDefaultPlugIn>( path, pol )
{
}

/*! \reimp
*/
QWidget* QDefaultPlugInManager::newWidget( const QString& classname, bool init, QWidget* parent, const char* name )
{
    QDefaultPlugIn* plugin = (QDefaultPlugIn*)pHnds[ classname ];
    if ( plugin )
	return plugin->createWidget( classname, init, parent, name );
    return 0;
}

/*!
  Returns a list of all widget classes supported by registered plugins.
*/
QStringList QDefaultPlugInManager::enumerateWidgets()
{
    QStringList list;
    QList<QPlugIn> plist = plugInList();
    QListIterator<QPlugIn> it ( plist );
   
    while( it.current() ) {
	QDefaultPlugIn* plugin = (QDefaultPlugIn*)it.current();
	QStringList widgets = QStringList::split( QRegExp("[;\\s]"), plugin->enumerateWidgets() );
	for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ )
	    list << *w;
	++it;
    }

    return list;
}

/*! \reimp
*/
QWidget* QDefaultPlugInManager::processFile( QIODevice* file, const QString& filetype )
{
    QDefaultPlugIn* plugin = (QDefaultPlugIn*)pHnds[filetype.latin1()];
    if ( plugin )
	return plugin->processFile( file, filetype );

    return 0;
}

/*! Returns a list of all file types supported by registered plugins.
*/
QStringList QDefaultPlugInManager::enumerateFileTypes()
{
    QStringList list;
    QList<QPlugIn> plist = plugInList();
    QListIterator<QPlugIn> it (plist);

    while ( it.current() ) {
	QDefaultPlugIn* plugin = (QDefaultPlugIn*)it.current();
	QString filter = plugin->enumerateFileTypes();
	filter.replace( QRegExp("\n"), ";;" );
	QStringList ft = QStringList::split( ";;", filter );
	for ( QStringList::Iterator f = ft.begin(); f != ft.end(); f++ )
	    list << *f;
	++it;
    }

    return list;
}

/*! \reimp
*/
QAction* QDefaultPlugInManager::newAction( const QString& actionname, bool& self, QObject* parent )
{
    QDefaultPlugIn* plugin = (QDefaultPlugIn*)pHnds[ actionname ];
    if ( plugin )
	return plugin->createAction( actionname, self, parent );
    return 0;
}

/*!
  Returns a list of all action names supported by registered plugins.
*/
QStringList QDefaultPlugInManager::enumerateActions()
{
    QStringList list;
    QDictIterator<QPlugIn> it( pHnds );

    while( it.current() ) {
	QDefaultPlugIn* plugin = (QDefaultPlugIn*)it.current();
    	QStringList actions = QStringList::split( QRegExp("[;\\s]"), plugin->enumerateActions() );
	for ( QStringList::Iterator a = actions.begin(); a != actions.end(); a++ )
	    list << *a;
	++it;
    }

    return list;
}
