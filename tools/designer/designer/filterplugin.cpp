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
    FilterInterface* iface = (*this)[filter];
    return iface ? iface->import( filter, filename ) : 0;
}
