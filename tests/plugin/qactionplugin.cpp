#include "qactionplugin.h"
#include <qaction.h>

/*!
  \class QActionPlugIn qdefaultplugin.h

  \brief A plugin loader implementing the QActionInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QActionPlugIn::QActionPlugIn( const QString& file, LibraryPolicy pol )
    : QPlugIn( file, pol )
{
}

/*! \reimp
*/
QAction* QActionPlugIn::create( const QString& classname, QObject* parent )
{
    if ( !use() )
	return 0;

    QAction* w = ((QActionInterface*)plugInterface())->create( classname, parent );
    guard( w );
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
QActionPlugInManager::QActionPlugInManager( const QString& path, QPlugIn::LibraryPolicy pol )
: QPlugInManager<QActionPlugIn>( path, pol )
{
}

/*! \reimp
*/
QAction* QActionPlugInManager::newAction( const QString& classname, QObject* parent )
{
    QActionPlugIn* plugin = (QActionPlugIn*)plugDict[ classname ];
    if ( plugin )
	return plugin->create( classname, parent );
    return 0;
}

/*!
  Returns a list of all widget classes supported by registered plugins.
*/
QStringList QActionPlugInManager::actions()
{
    QStringList list;
    QDictIterator<QPlugIn> it (libDict);

    while( it.current() ) {
	QStringList actions = it.current()->featureList();
	for ( QStringList::Iterator a = actions.begin(); a != actions.end(); a++ )
	    list << *a;
	++it;
    }

    return list;
}
