#include "../../qsqldriverinterface.h"
#include "qsql_odbc.h"
#include <qstringlist.h>
#include <qapplication.h>

class QODBCDriverInterface : public QSqlDriverInterface
{
public:
    QODBCDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList() const;
};

QSqlDriver* QODBCDriverInterface::create( const QString &name )
{
    if ( name == "QODBC" ) {
	return new QODBCDriver();
    }
    return 0;
}

QStringList QODBCDriverInterface::featureList() const
{
    QStringList l;
    l.append("QODBC");
    return l;
}

class QODBCDriverPlugIn : public QPlugInInterface
{
public:
    QStringList interfaceList() const;
    QUnknownInterface* queryInterface( const QString& request );
};

QStringList QODBCDriverPlugIn::interfaceList() const
{
    QStringList list;

    list << "QODBCDriverInterface";

    return list;
}

QUnknownInterface* QODBCDriverPlugIn::queryInterface( const QString& request )
{
    if ( request == "QODBCDriverInterface" )
	return new QODBCDriverInterface;
    return 0;
}


Q_EXPORT_INTERFACE(QODBCDriverPlugIn)
