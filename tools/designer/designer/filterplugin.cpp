#include "filterplugin.h"

/*!
  \class FilterPlugIn qfilterplugin.h

  \brief A plugin loader implementing the FilterInterface
*/

/*!
  Constructs a filter plugin with file \a file and policy \a pol.
*/
FilterPlugIn::FilterPlugIn( const QString& file, LibraryPolicy pol, const char* fn )
    : QPlugIn( file, pol, fn )
{
}

QStringList FilterPlugIn::import( const QString& filter, const QString& filename )
{
    if ( !use() )
	return QStringList();

    return ((FilterInterface*)plugInterface())->import( filter, filename );
}

/*!
  \class FilterPlugInManager qfilterplugin.h

  \brief Implements a QPlugInManager that handles plugins for import/export filters

  \sa QPlugInManager
*/

/*!
  Creates a FilterPlugInManager.

  \sa QPlugInManager
*/
FilterPlugInManager::FilterPlugInManager( const QString& path, const QString& filter,
					   QPlugIn::LibraryPolicy pol, const char* fn  )
: QPlugInManager<FilterPlugIn>( path, filter, pol, fn )
{
}

QStringList FilterPlugInManager::import( const QString& filter, const QString& filename )
{
    FilterPlugIn* plugin = (FilterPlugIn*)plugIn( filter );
    if ( plugin )
	return plugin->import( filter, filename );
    return QStringList();
}
