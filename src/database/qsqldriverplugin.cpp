#include "qsqldriverplugin.h"

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
    qDebug("QSqlDriverPlugIn::create");
    if ( !use() )
	return 0;
    qDebug("QSqlDriverPlugIn::create calling plugInterface->create()");
    QSqlDriver* d = ((QSqlDriverInterface*)plugInterface())->create( name );
    return d;
}

/*!
  \reimpl
*/
QStringList QSqlDriverPlugIn::featureList()
{
    qDebug("QSqlDriverPlugIn::featureList()");
    if ( !use() ) {
	qDebug("could not use, returning 0");
	return 0;
    }
    qDebug("QSqlDriverPlugIn::featureList(), returning plugInterface->featureList()");
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
