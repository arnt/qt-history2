#include "filterplugin.h"

/*!
  Creates a FilterPlugInManager.

  \sa QPlugInManager
*/
FilterPlugInManager::FilterPlugInManager( const QString& path, const QString& filter,
					  QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QInterfaceManager<FilterInterface>( path, filter, appIface, pol )
{
}

QStringList FilterPlugInManager::import( const QString& filter, const QString& filename )
{
    QStringList list;
    FilterInterface* iface = queryInterface( filter );
    if ( iface ) {
	list = iface->import( filter, filename );
	iface->release();
    }
    return list;
}
