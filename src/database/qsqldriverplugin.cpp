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
	QApplicationInterface* appIface, QPlugIn::LibraryPolicy pol )
: QInterfaceManager<QSqlDriverInterface>( path, filter, appIface, pol )
{
}

/*!
  \reimpl
*/
QSqlDriver* QSqlDriverPlugInManager::create( const QString& name )
{
    QSqlDriverInterface *iface = (*this)[name];
    return iface ? iface->create( name ) : 0;
}


#endif // QT_NO_SQL

