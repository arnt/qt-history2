#include "../../qsqldriverinterface.h"
#include "qsql_mysql.h"
#include <qstringlist.h>

class QMySQLDriverInterface : public QSqlDriverInterface
{
public:
    QMySQLDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList() const;    
};

QSqlDriver* QMySQLDriverInterface::create( const QString &name )
{
    if ( name == "QMYSQL" )
	return new QMySQLDriver();
    return 0;
}

QStringList QMySQLDriverInterface::featureList() const
{
    QStringList l;
    l.append("QMYSQL");
    return l;
}

class QMySQLDriverPlugIn : public QPlugInInterface
{
public:
    QStringList interfaceList() const;
    QUnknownInterface* queryInterface( const QString& request );
};

QStringList QMySQLDriverPlugIn::interfaceList() const
{
    QStringList list;

    list << "QMySQLDriverInterface";

    return list;
}

QUnknownInterface* QMySQLDriverPlugIn::queryInterface( const QString& request )
{
    if ( request == "QMySQLDriverInterface" )
	return new QMySQLDriverInterface;
    return 0;
}

Q_EXPORT_INTERFACE(QMySQLDriverPlugIn)
