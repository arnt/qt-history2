#include "../../qsqldriverinterface.h"
#include "qsql_oci.h"
#include <qstringlist.h>

class QOCIDriverInterface : public QSqlDriverInterface
{
public:
    QOCIDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};


QSqlDriver* QOCIDriverInterface::create( const QString &name )
{
    qDebug("QOCIDriverInterface::create( const QString &name )");
    if ( name == "QOCI" ) {
	qDebug("Returning new QOCIDriver();");
	return new QOCIDriver();
    }
    return 0;
}

QStringList QOCIDriverInterface::featureList()
{
    QStringList l;
    l.append("QOCI");
    return l;
}

Q_EXPORT_INTERFACE(QSqlDriverInterface, QOCIDriverInterface)
