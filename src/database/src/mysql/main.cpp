#include "../../qsqldriverinterface.h"
#include "qsql_mysql.h"
#include <qstringlist.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class QMySQLDriverInterface : public QSqlDriverInterface
{
public:
    QMySQLDriverInterface(){}
    QCString queryPlugInInterface() const { return "QSqlDriverInterface"; }

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

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QSqlDriverInterface* loadInterface()
{
    return new QMySQLDriverInterface();
}

LIBEXPORT bool onConnect()
{
    return TRUE;
}

LIBEXPORT bool onDisconnect()
{
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus


