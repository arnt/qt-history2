#include "qwidgetplugin.h"
#include <qwidget.h>

/*!
  \class QWidgetPlugIn qdefaultplugin.h

  \brief A plugin loader implementing the QWidgetInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QWidgetPlugIn::QWidgetPlugIn( const QString& file, LibraryPolicy pol, const char* fn )
    : QPlugIn( file, pol, fn )
{
}

/*! \reimp
*/
QWidget* QWidgetPlugIn::create( const QString& classname, QWidget* parent, const char* name )
{
    if ( !use() )
	return 0;

    return ((QWidgetInterface*)plugInterface())->create( classname, parent, name );
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
QWidgetPlugInManager::QWidgetPlugInManager( const QString& path, const QString& filter, 
					   QPlugIn::LibraryPolicy pol, const char* fn  )
: QPlugInManager<QWidgetPlugIn>( path, filter, pol, fn )
{
}

/*! \reimp
*/
QWidget* QWidgetPlugInManager::newWidget( const QString& classname, QWidget* parent, const char* name )
{
    QWidgetPlugIn* plugin = (QWidgetPlugIn*)plugIn( classname );
    if ( plugin )
	return plugin->create( classname, parent, name );
    return 0;
}

/*!
  Returns a list of all widget classes supported by registered plugins.
*/
QStringList QWidgetPlugInManager::widgets()
{
    return featureList();
}
