#include "actionplugin.h"

ActionPlugInManager::ActionPlugInManager( const QString& path, const QString& filter,
					  QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QInterfaceManager<ActionInterface>( path, filter, appIface, pol )
{
}

QAction* ActionPlugInManager::create( const QString& actionname, QObject* parent )
{
    ActionInterface *iface = (*this)[actionname];
    return iface ? iface->create( actionname, parent ) : 0;
}

QString ActionPlugInManager::group( const QString& actionname )
{
    ActionInterface *iface = (*this)[actionname];
    return iface ? iface->group( actionname ) : QString::null;
}
