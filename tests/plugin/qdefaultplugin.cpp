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

    createWidgetPtr = (CreateWidgetProc) getSymbolAddress( "createWidget" );
    widgetsPtr = (StringProc) getSymbolAddress( "widgets" );

    createActionPtr = (CreateActionProc) getSymbolAddress( "createAction" );
    actionsPtr = (StringProc) getSymbolAddress( "actions" );

    return TRUE;
}

/*! \reimp
*/
bool QDefaultPlugIn::addToManager( QPlugInDict& dict )
{
    bool useful = FALSE;

    QStringList wl = QStringList::split( QRegExp("[;\\s]"), widgets() );
    for ( QStringList::Iterator w = wl.begin(); w != wl.end(); w++ ) {
	useful = TRUE;
#ifdef CHECK_RANGE
	if ( dict[*w] )
	    qWarning("%s: Widget %s already defined!", library().latin1(), (*w).latin1() );
	else
#endif
	    dict.insert( *w, this );
    }

    QStringList al = QStringList::split( QRegExp("[;\\s]"), actions() );
    for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ ) {
	useful = TRUE;
#ifdef CHECK_RANGE
	if ( dict[*a] )
	    qWarning("%s: Action %s already defined!", library().latin1(), (*a).latin1() );
	else
#endif
	    dict.insert( *a, this );
    }

    return useful;
}

/*! \reimp
*/
bool QDefaultPlugIn::removeFromManager( QPlugInDict& dict )
{
    bool res = TRUE;

    QStringList wl = QStringList::split( QRegExp("[;\\s]"), widgets() );
    for ( QStringList::Iterator w = wl.begin(); w != wl.end(); w++ )
        res = res && dict.remove( *w );

    QStringList al = QStringList::split( QRegExp("[;\\s]"), actions() );
    for ( QStringList::Iterator a = al.begin(); a != al.end(); a++ )
        res = res && dict.remove( *a );

    return res;
}

/*!
  Calls the appropriate plugin's create-method and returns the result.
*/
QWidget* QDefaultPlugIn::create( const QString& classname, QWidget* parent, const char* name )
{
    use();

    if ( createWidgetPtr )
	return createWidgetPtr( classname, parent, name );
    return 0;
}

/*!
  Calls the appropriate plugin's widgets-method and returns the result.
*/
const char* QDefaultPlugIn::widgets()
{
    use();

    if ( widgetsPtr )
	return widgetsPtr();
    return "";
}

/*!
  Calls the appropriate plugin's create-method and returns the result.
*/
QAction* QDefaultPlugIn::create( const QString& actionname, bool& self, QObject *parent )
{
    use();

    if ( createActionPtr )
	return createActionPtr( actionname, self, parent );
    return 0;
}

/*!
  Calls the appropriate plugin's actions-method and returns the result.
*/
const char* QDefaultPlugIn::actions()
{
    use();

    if ( actionsPtr )
	return actionsPtr();
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
QWidget* QDefaultPlugInManager::newWidget( const QString& classname, QWidget* parent, const char* name )
{
    QDefaultPlugIn* plugin = (QDefaultPlugIn*)plugDict[ classname ];
    if ( plugin )
	return plugin->create( classname, parent, name );
    return 0;
}

/*!
  Returns a list of all widget classes supported by registered plugins.
*/
QStringList QDefaultPlugInManager::widgets()
{
    QStringList list;
    QDictIterator<QPlugIn> it (libDict);
   
    while( it.current() ) {
	QDefaultPlugIn* plugin = (QDefaultPlugIn*)it.current();
	QStringList widgets = QStringList::split( QRegExp("[;\\s]"), plugin->widgets() );
	for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ )
	    list << *w;
	++it;
    }

    return list;
}

/*! \reimp
*/
QAction* QDefaultPlugInManager::newAction( const QString& actionname, bool& self, QObject* parent )
{
    QDefaultPlugIn* plugin = (QDefaultPlugIn*)plugDict[ actionname ];
    if ( plugin )
	return plugin->create( actionname, self, parent );
    return 0;
}

/*!
  Returns a list of all action names supported by registered plugins.
*/
QStringList QDefaultPlugInManager::actions()
{
    QStringList list;
    QDictIterator<QPlugIn> it (libDict);

    while( it.current() ) {
	QDefaultPlugIn* plugin = (QDefaultPlugIn*)it.current();
    	QStringList actions = QStringList::split( QRegExp("[;\\s]"), plugin->actions() );
	for ( QStringList::Iterator a = actions.begin(); a != actions.end(); a++ )
	    list << *a;
	++it;
    }

    return list;
}
