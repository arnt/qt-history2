#include "../../qsqldriverinterface.h"
#include "qsql_psql.h"
#include <qstringlist.h>

class QPSQLDriverInterface : public QSqlDriverInterface
{
public:
    QPSQLDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList() const;
};

QSqlDriver* QPSQLDriverInterface::create( const QString &name )
{
    if ( name == "QPSQL" )
	return new QPSQLDriver();
    return 0;
}

QStringList QPSQLDriverInterface::featureList() const
{
    QStringList l;
    l.append("QPSQL");
    return l;
}

class QPSQLDriverPlugIn : public QPlugInInterface
{
public:
    QStringList interfaceList() const;
    QUnknownInterface* queryInterface( const QString& request );
};

QStringList QPSQLDriverPlugIn::interfaceList() const
{
    QStringList list;

    list << "QPSQLDriverInterface";

    return list;
}

QUnknownInterface* QPSQLDriverPlugIn::queryInterface( const QString& request )
{
    if ( request == "QPSQLDriverInterface" )
	return new QPSQLDriverInterface;
    return 0;
}

Q_EXPORT_INTERFACE(QPSQLDriverPlugIn)
