#include "widgetplugin.h"

/*!
  Creates a QDefaultPlugInManager.

  \sa QPlugInManager
*/
WidgetPlugInManager::WidgetPlugInManager( const QString& path, const QString& filter, 
					  QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QInterfaceManager<WidgetInterface>( path, filter, appIface, pol )
{
}

QWidget* WidgetPlugInManager::create( const QString& classname, QWidget* parent, const char* name )
{
    QWidget *w = 0;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	w = iface->create( classname, parent, name );
	iface->release();
    }
    return w;
}

QString WidgetPlugInManager::group( const QString& classname )
{
    QString str;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	str = iface->group( classname );
	iface->release();
    }
    return str;
}

QString WidgetPlugInManager::iconSet( const QString& classname )
{
    QString str;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	str = iface->iconSet( classname );
	iface->release();
    }
    return str;
}

QIconSet WidgetPlugInManager::iconset( const QString& classname )
{
    QIconSet set;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	set = iface->iconset( classname );
	iface->release();
    }
    return set;
}

QString WidgetPlugInManager::includeFile( const QString& classname )
{
    QString str;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	str = iface->includeFile( classname );
	iface->release();
    }
    return str;
}

QString WidgetPlugInManager::toolTip( const QString& classname )
{
    QString str;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	str = iface->toolTip( classname );
	iface->release();
    }
    return str;
}

QString WidgetPlugInManager::whatsThis( const QString& classname )
{
    QString str;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	str = iface->whatsThis( classname );
	iface->release();
    }
    return str;
}

bool WidgetPlugInManager::isContainer( const QString& classname )
{
    bool is = FALSE;
    WidgetInterface* iface = queryInterface( classname );
    if ( iface ) {
	is = iface->isContainer( classname );
	iface->release();
    }
    return is;
}
