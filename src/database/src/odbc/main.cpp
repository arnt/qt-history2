#include "../../qsqldriverinterface.h"
#include "qsql_odbc.h"
#include <qstringlist.h>
#include <qapplication.h>

class QODBCDriverInterface : public QPlugInInterface, public QSqlDriverInterface
{
public:
    QODBCDriverInterface(){}
    ~QODBCDriverInterface(){}

    QStringList interfaceList();
    QUnknownInterface* queryInterface( const QString& request );

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};

QStringList QODBCDriverInterface::interfaceList()
{
    QStringList list;

    list << "QODBCDriverInterface";

    return list;
}

QUnknownInterface* QODBCDriverInterface::queryInterface( const QString& request )
{
    if ( request == "QODBCDriverInterface" )
	return new QODBCDriverInterface;
    return 0;
}

QSqlDriver* QODBCDriverInterface::create( const QString &name )
{
    if ( name == "QODBC" ) {
	return new QODBCDriver();
    }
    return 0;
}

QStringList QODBCDriverInterface::featureList()
{
    QStringList l;
    l.append("QODBC");
    return l;
}

Q_EXPORT_INTERFACE(QSqlDriverInterface, QODBCDriverInterface)
