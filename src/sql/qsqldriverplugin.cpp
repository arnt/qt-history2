#include "qsqldriverplugin.h"

#ifndef QT_NO_SQL

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
	QApplicationInterface* appIface, QLibrary::Policy pol )
: QInterfaceManager<QSqlDriverInterface>( "QSqlDriverInterface", path, filter, appIface, pol )
{
}

/*!
  \reimpl
*/
QSqlDriver* QSqlDriverPlugInManager::create( const QString& name )
{
    QSqlDriver *driver = 0;
    QSqlDriverInterface *iface = queryInterface( name );
    if ( iface ) {
	driver = iface->create( name );
	iface->release();
    }
    return driver;
}


#endif // QT_NO_SQL

