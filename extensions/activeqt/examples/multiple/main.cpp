#include <qaxfactory.h>

#include "ax1.h"
#include "ax2.h"

class ActiveQtFactory : public QAxFactory
{
public:
    ActiveQtFactory( const QUuid &lib, const QUuid &app )
	: QAxFactory( lib, app )
    {}
    QStringList featureList() const
    {
	QStringList list;
	list << "QAxWidget1";
	list << "QAxWidget2";
	return list;
    }
    QWidget *create( const QString &key, QWidget *parent, const char *name )
    {
	if ( key == "QAxWidget1" )
	    return new QAxWidget1( parent, name );
	if ( key == "QAxWidget2" )
	    return new QAxWidget2( parent, name );

	return 0;
    }
    QMetaObject *metaObject( const QString &key ) const
    {
	if ( key == "QAxWidget1" )
	    return QAxWidget1::staticMetaObject();
	if ( key == "QAxWidget2" )
	    return QAxWidget2::staticMetaObject();

	return 0;
    }
    QUuid classID( const QString &key ) const
    {
	if ( key == "QAxWidget1" )
	    return "{1D9928BD-4453-4bdd-903D-E525ED17FDE5}";
	if ( key == "QAxWidget2" )
	    return "{58139D56-6BE9-4b17-937D-1B1EDEDD5B71}";

	return QUuid();
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( key == "QAxWidget1" )
	    return "{99F6860E-2C5A-42ec-87F2-43396F4BE389}";
	if ( key == "QAxWidget2" )
	    return "{B66280AB-08CC-4dcc-924F-58E6D7975B7D}";

	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( key == "QAxWidget1" )
	    return "{0A3E9F27-E4F1-45bb-9E47-63099BCCD0E3}";
	if ( key == "QAxWidget2" )
	    return "{D72BACBA-03C4-4480-B4BB-DE4FE3AA14A0}";

	return QUuid();
    }
    QString exposeToSuperClass( const QString &key ) const
    {
	if ( key == "QAxWidget2" )
	    return key;
	return QAxFactory::exposeToSuperClass( key );
    }
    bool hasStockEvents( const QString &key ) const
    {
	if ( key == "QAxWidget2" )
	    return TRUE;
	return FALSE;
    }
};

QAXFACTORY_EXPORT( ActiveQtFactory, "{98DE28B6-6CD3-4e08-B9FA-3D1DB43F1D2F}", "{05828915-AD1C-47ab-AB96-D6AD1E25F0E2}" )

void main()
{
}
