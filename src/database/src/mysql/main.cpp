#include "../../qsqldriverinterface.h"
#include "qsql_mysql.h"
#include <qstringlist.h>

class QMySQLDriverInterface : public QPlugInInterface, public QSqlDriverInterface
{
public:
    QMySQLDriverInterface(){}

    QStringList interfaceList();
    QUnknownInterface* queryInterface( const QString& request );

    QSqlDriver* create( const QString &name );
    QStringList featureList();    
};

QStringList QMySQLDriverInterface::interfaceList()
{
    QStringList list;

    list << "QMySQLDriverInterface";

    return list;
}

QUnknownInterface* QMySQLDriverInterface::queryInterface( const QString& request )
{
    if ( request == "QMySQLDriverInterface" )
	return new QMySqlDriverInterface;
    return 0;
}

QSqlDriver* QMySQLDriverInterface::create( const QString &name )
{
    if ( name == "QMYSQL" )
	return new QMySQLDriver();
    return 0;
}

QStringList QMySQLDriverInterface::featureList()
{
    QStringList l;
    l.append("QMYSQL");
    return l;
}

Q_EXPORT_INTERFACE(QMySQLDriverInterface)
