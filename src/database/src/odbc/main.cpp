#include "../../qsqldriverinterface.h"
#include "qsql_odbc.h"
#include <qstringlist.h>
#include <qapplication.h>

class QODBCDriverInterface : public QSqlDriverInterface
{
public:
    QODBCDriverInterface(){}
    ~QODBCDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};


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
