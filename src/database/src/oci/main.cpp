#include "../../qsqldriverinterface.h"
#include "qsql_oci.h"
#include <qstringlist.h>

class QOCIDriverInterface : public QPlugInInterface, public QSqlDriverInterface
{
public:
    QOCIDriverInterface(){}

    QStringList interfaceList();
    QUnknownInterface* queryInterface( const QString& request );

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};

QStringList QOCIDriverInterface::interfaceList()
{
    QStringList list;

    list << "QOCIDriverInterface";

    return list;
}

QUnknownInterface* QOCIDriverInterface::queryInterface( const QString& request )
{
    if ( request == "QOCIDriverInterface" )
	return new QOCIDriverInterface;
    return 0;
}


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
