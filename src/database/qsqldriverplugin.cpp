#include "qsqldriverplugin.h"

#ifndef QT_NO_SQL

/*!
  \class QSqlDriverPlugIn

  \brief A plugin loader implementing the QSqlDriverInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QSqlDriverPlugIn::QSqlDriverPlugIn( const QString& file, LibraryPolicy pol, const char* fn )
    : QPlugIn( file, pol, fn )
{
}

/*!
  \reimpl
*/
QSqlDriver* QSqlDriverPlugIn::create( const QString& name )
{
    if ( !use() )
	return 0;
    return = ((QSqlDriverInterface*)plugInterface())->create( name );
}

/*!
  \reimpl
*/
QStringList QSqlDriverPlugIn::featureList()
{
    if ( !use() )
	return 0;

    return ((QSqlDriverInterface*)plugInterface())->featureList();
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
	QPlugIn::LibraryPolicy pol, const char* fn )
: QPlugInManager<QSqlDriverPlugIn>( path, filter, pol, fn )
{
}

#endif // QT_NO_SQL

