#include "../../qsqldriverinterface.h"
#include "qsql_mysql.h"
#include <qstringlist.h>

class QMySQLDriverInterface : public QSqlDriverInterface
{
public:
    QMySQLDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};


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

Q_EXPORT_INTERFACE(QSqlDriverInterface, QMySQLDriverInterface)
