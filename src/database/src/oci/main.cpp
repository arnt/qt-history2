#include "../../qsqldriverinterface.h"
#include "qsql_oci.h"
#include <qstringlist.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class QOCIDriverInterface : public QSqlDriverInterface
{
public:
    QOCIDriverInterface(){}
    QCString queryPlugInInterface() const { return "QSqlDriverInterface"; }

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

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QSqlDriverInterface* loadInterface()
{
    qDebug("QOCI QSqlDriverInterface* loadInterface()");
    return new QOCIDriverInterface();
}

LIBEXPORT bool onConnect()
{
    qDebug("QOCI onConnect");
    return TRUE;
}

LIBEXPORT bool onDisconnect()
{
    qDebug("QOCI onDisconnect");
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus

