#include "actionplugin.h"

ActionPlugInManager::ActionPlugInManager( const QString& path, const QString& filter,
					  QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QInterfaceManager<ActionInterface>( path, filter, appIface, pol )
{
}

QAction* ActionPlugInManager::create( const QString& actionname, QObject* parent )
{
    QAction *a;
    ActionInterface *iface = queryInterface( actionname );
    if ( iface ) {
	a = iface->create( actionname, parent );
	iface->release();
    }
    return a;
}

QString ActionPlugInManager::group( const QString& actionname )
{
    QString str;
    ActionInterface *iface = queryInterface( actionname );
    if ( iface ) {
	str = iface->group( actionname );
	iface->release();
    }
    return str;
}
