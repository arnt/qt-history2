#include "actionplugin.h"
#include <qaction.h>

/*!
  \class ActionPlugIn qdefaultplugin.h

  \brief A plugin loader implementing the ActionInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
ActionPlugIn::ActionPlugIn( const QString& file, QApplicationInterface* appIface, LibraryPolicy pol )
    : QPlugIn( file, appIface, pol )
{
}

/*! \reimp
*/
QAction* ActionPlugIn::create( const QString& actionname, QObject* parent )
{
    if ( !use() )
	return 0;

    return ((ActionInterface*)plugInterface())->create( actionname, parent );
}

/*! \reimp
*/
QString ActionPlugIn::group( const QString& actionname )
{
    if ( !use() )
	return 0;

    return((ActionInterface*)plugInterface())->group( actionname );
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
ActionPlugInManager::ActionPlugInManager( const QString& path, const QString& filter,
					  QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QPlugInManager<ActionPlugIn>( path, filter, appIface, pol )
{
}

/*! \reimp
*/
QAction* ActionPlugInManager::create( const QString& actionname, QObject* parent )
{
    ActionPlugIn* plugin = (ActionPlugIn*)plugIn( actionname );
    if ( plugin )
	return plugin->create( actionname, parent );
    return 0;
}

/*! \reimp
*/
QString ActionPlugInManager::group( const QString& actionname )
{
    ActionPlugIn* plugin = (ActionPlugIn*)plugIn( actionname );
    if ( plugin )
	return plugin->group( actionname );
    return QString::null;
}
