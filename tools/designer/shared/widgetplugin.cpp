#include "widgetplugin.h"
#include <qwidget.h>

/*!
  \class WidgetPlugIn widgetplugin.h

  \brief A plugin loader implementing the WidgetInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
WidgetPlugIn::WidgetPlugIn( const QString& file, QApplicationInterface* appIface, LibraryPolicy pol )
    : QPlugIn( file, appIface, pol )
{
}

/*! \reimp
*/
QWidget* WidgetPlugIn::create( const QString& classname, QWidget* parent, const char* name )
{
    if ( !use() )
	return 0;

    return ((WidgetInterface*)plugInterface())->create( classname, parent, name );
}

/*! \reimp
*/
QString WidgetPlugIn::group( const QString& classname )
{
    if ( !use() )
	return QString::null;

    return ((WidgetInterface*)plugInterface())->group( classname );
}

/*! \reimp
*/
QString WidgetPlugIn::iconSet( const QString& classname )
{
    if ( !use() )
	return QString::null;

    return ((WidgetInterface*)plugInterface())->iconSet( classname );
}

/*! \reimp
*/
QIconSet WidgetPlugIn::iconset( const QString& classname )
{
    if ( !use() )
	return QIconSet();

    return ((WidgetInterface*)plugInterface())->iconset( classname );
}

/*! \reimp
*/
QString WidgetPlugIn::includeFile( const QString& classname )
{
    if ( !use() )
	return QString::null;

    return ((WidgetInterface*)plugInterface())->includeFile( classname );
}

/*! \reimp
*/
QString WidgetPlugIn::toolTip( const QString& classname )
{
    if ( !use() )
	return QString::null;

    return ((WidgetInterface*)plugInterface())->toolTip( classname );
}

/*! \reimp
*/
QString WidgetPlugIn::whatsThis( const QString& classname )
{
    if ( !use() )
	return QString::null;

    return ((WidgetInterface*)plugInterface())->whatsThis( classname );
}

/*! \reimp
*/
bool WidgetPlugIn::isContainer( const QString& classname )
{
    if ( !use() )
	return FALSE;

    return ((WidgetInterface*)plugInterface())->isContainer( classname );
}

/*!
  \class WidgetPlugInManager qwidgetplugin.h

  \brief Implements a QPlugInManager that handles plugins for custom widgets

  \sa QPlugInManager
*/

/*!
  Creates a QDefaultPlugInManager.

  \sa QPlugInManager
*/
WidgetPlugInManager::WidgetPlugInManager( const QString& path, const QString& filter, 
					  QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QPlugInManager<WidgetPlugIn>( path, filter, appIface, pol )
{
}

/*! \reimp
*/
QWidget* WidgetPlugInManager::create( const QString& classname, QWidget* parent, const char* name )
{
    WidgetPlugIn* plugin = plugIn( classname );
    if ( plugin )
	return plugin->create( classname, parent, name );
    return 0;
}

QString WidgetPlugInManager::group( const QString& classname )
{
    WidgetPlugIn* plugin = plugIn( classname );
    if ( plugin )
	return plugin->group( classname );
    return QString::null;
}

QString WidgetPlugInManager::iconSet( const QString& classname )
{
    WidgetPlugIn* plugin = plugIn( classname );
    if ( plugin )
	return plugin->iconSet( classname );
    return QString::null;
}

QString WidgetPlugInManager::includeFile( const QString& classname )
{
    WidgetPlugIn* plugin = plugIn( classname );
    if ( plugin )
	return plugin->includeFile( classname );
    return QString::null;
}

QString WidgetPlugInManager::toolTip( const QString& classname )
{
    WidgetPlugIn* plugin = plugIn( classname );
    if ( plugin )
	return plugin->toolTip( classname );
    return QString::null;
}

QString WidgetPlugInManager::whatsThis( const QString& classname )
{
    WidgetPlugIn* plugin = plugIn( classname );
    if ( plugin )
	return plugin->whatsThis( classname );
    return QString::null;
}

bool WidgetPlugInManager::isContainer( const QString& classname )
{
    WidgetPlugIn* plugin = plugIn( classname );
    if ( plugin )
	return plugin->isContainer( classname );
    return FALSE;
}

