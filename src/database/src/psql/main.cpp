#include "../../qsqldriverinterface.h"
#include "qsql_psql.h"
#include <qstringlist.h>

class QPSQLDriverInterface : public QSqlDriverInterface
{
public:
    QPSQLDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};


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
