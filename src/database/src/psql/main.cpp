#include "../../qsqldriverinterface.h"
#include "qsql_psql.h"
#include <qstringlist.h>

class QPSQLDriverInterface : public QPlugInInterface, public QSqlDriverInterface
{
public:
    QPSQLDriverInterface(){}

    QStringList interfaceList();
    QUnknownInterface* queryInterface( const QString& request );

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};

QStringList QPSQLDriverInterface::interfaceList()
{
    QStringList list;

    list << "QPSQLDriverInterface";

    return list;
}

QUnknownInterface* QPSQLDriverInterface::queryInterface( const QString& request )
{
    if ( request == "QPSQLDriverInterface" )
	return new QPSQLDriverInterface;
    return 0;
}

QSqlDriver* QPSQLDriverInterface::create( const QString &name )
{
    if ( name == "QPSQL" )
	return new QPSQLDriver();
    return 0;
}

QStringList QPSQLDriverInterface::featureList()
{
    QStringList l;
    l.append("QPSQL");
    return l;
}

Q_EXPORT_INTERFACE(QSqlDriverInterface,QPSQLDriverInterface)
