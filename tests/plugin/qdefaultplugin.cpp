#include "qwidgetplugin.h"
#include <qwidget.h>

/*!
  \class QWidgetPlugIn qdefaultplugin.h

  \brief A plugin loader implementing the QWidgetInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QWidgetPlugIn::QWidgetPlugIn( const QString& file, LibraryPolicy pol )
    : QPlugIn( file, pol )
{
}

/*! \reimp
*/
bool QWidgetPlugIn::addToManager( QPlugInDict& dict )
{
    if ( !use() )
	return FALSE;
    
    bool useful = FALSE;

    QStringList list = this->widgets();
    for ( QStringList::Iterator w = list.begin(); w != list.end(); w++ ) {
	useful = TRUE;
#ifdef CHECK_RANGE
	if ( dict[*w] )
	    qWarning("%s: Widget %s already defined!", library().latin1(), (*w).latin1() );
	else
#endif
	    dict.insert( *w, this );
    }

    return useful;
}

/*! \reimp
*/
bool QWidgetPlugIn::removeFromManager( QPlugInDict& dict )
{
    bool res = TRUE;

    QStringList wl = this->widgets();
    for ( QStringList::Iterator w = wl.begin(); w != wl.end(); w++ )
        res = res && dict.remove( *w );

    return res;
}

/*! \reimp
*/
QWidget* QWidgetPlugIn::create( const QString& classname, QWidget* parent, const char* name )
{
    if ( !use() )
	return 0;

    QWidget* w = ((QDefaultInterface*)iface())->create( classname, parent, name );
    guard( w );
    return w;
}

/*! \reimp
*/
QStringList QWidgetPlugIn::widgets()
{
    if ( !use() )
	return QStringList();
    QStringList list = ((QDefaultInterface*)iface())->widgets();

    return list;
}

/*!
  \class QDefaultPlugInManager qdefaultpluginmanager.h

  \brief Implements a QPlugInManager that handles plugins for custom widgets

  \sa QPlugInManager
*/

/*!
  Creates a QDefaultPlugInManager.

  \sa QPlugInManager
*/
QWidgetPlugInManager::QWidgetPlugInManager( const QString& path, QPlugIn::LibraryPolicy pol )
: QPlugInManager<QWidgetPlugIn>( path, pol )
{
}

/*! \reimp
*/
QWidget* QWidgetPlugInManager::newWidget( const QString& classname, QWidget* parent, const char* name )
{
    QDefaultPlugIn* plugin = (QDefaultPlugIn*)plugDict[ classname ];
    if ( plugin )
	return plugin->create( classname, parent, name );
    return 0;
}

/*!
  Returns a list of all widget classes supported by registered plugins.
*/
QStringList QWidgetPlugInManager::widgets()
{
    QStringList list;
    QDictIterator<QPlugIn> it (libDict);

    while( it.current() ) {
	QStringList widgets = ((QDefaultPlugIn*)it.current())->widgets();
	for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ )
	    list << *w;
	++it;
    }

    return list;
}


