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
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->create( classname, parent, name ) : 0;
}

QString WidgetPlugInManager::group( const QString& classname )
{
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->group( classname ) : QString::null;
}

QString WidgetPlugInManager::iconSet( const QString& classname )
{
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->iconSet( classname ) : QString::null;
}

QIconSet WidgetPlugInManager::iconset( const QString& classname )
{
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->iconset( classname ) : QIconSet();
}

QString WidgetPlugInManager::includeFile( const QString& classname )
{
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->includeFile( classname ) : QString::null;
}

QString WidgetPlugInManager::toolTip( const QString& classname )
{
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->toolTip( classname ) : QString::null;
}

QString WidgetPlugInManager::whatsThis( const QString& classname )
{
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->whatsThis( classname ) : QString::null;
}

bool WidgetPlugInManager::isContainer( const QString& classname )
{
    WidgetInterface* iface = (*this)[classname];
    return iface ? iface->isContainer( classname ) : 0;
}

