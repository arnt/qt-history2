#include "qsqldriverplugin.h"

#ifndef QT_NO_SQL

/*!
  \class QSqlDriverPlugIn

  \brief A plugin loader implementing the QSqlDriverInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QSqlDriverPlugIn::QSqlDriverPlugIn( const QString& file, QApplicationInterface* appIface, LibraryPolicy pol )
    : QPlugIn( file, appIface, pol )
{
}

/*!
  \reimpl
*/
QSqlDriver* QSqlDriverPlugIn::create( const QString& name )
{
    if ( !use() )
	return 0;
    return ((QSqlDriverInterface*)plugInterface())->create( name );
}

/*!
  \class QSqlDriverPlugInManager

  \brief Implements a QPlugInManager that handles plugins for SQL drivers

  \sa QPlugInManager
*/

/*!
  Creates a QSqlDriverPlugInManager

  \sa QPlugInManager
*/
QSqlDriverPlugInManager::QSqlDriverPlugInManager( const QString& path, const QString& filter,
	QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QPlugInManager<QSqlDriverPlugIn>( path, filter, appIface, pol )
{
}

#endif // QT_NO_SQL

