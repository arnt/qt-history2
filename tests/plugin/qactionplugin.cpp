#include "qactionplugin.h"
#include <qaction.h>

/*!
  \class QActionPlugIn qdefaultplugin.h

  \brief A plugin loader implementing the QActionInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QActionPlugIn::QActionPlugIn( const QString& file, LibraryPolicy pol, const char* fn )
    : QPlugIn( file, pol, fn )
{
}

/*! \reimp
*/
QAction* QActionPlugIn::create( const QString& classname, QObject* parent )
{
    if ( !use() )
	return 0;

    QAction* w = ((QActionInterface*)plugInterface())->create( classname, parent );
    return w;
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
QActionPlugInManager::QActionPlugInManager( const QString& path, const QString& filter, 
					   QPlugIn::LibraryPolicy pol, const char* fn  )
: QPlugInManager<QActionPlugIn>( path, filter, pol, fn )
{
}

/*! \reimp
*/
QAction* QActionPlugInManager::newAction( const QString& classname, QObject* parent )
{
    QActionPlugIn* plugin = (QActionPlugIn*)plugIn( classname );
    if ( plugin )
	return plugin->create( classname, parent );
    return 0;
}

/*!
  Returns a list of all widget classes supported by registered plugins.
*/
QStringList QActionPlugInManager::actions()
{
    return features();
}
