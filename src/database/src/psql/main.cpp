#include "../../qsqldriverinterface.h"
#include "qsql_psql.h"
#include <qstringlist.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class QPSQLDriverInterface : public QSqlDriverInterface
{
public:
    QPSQLDriverInterface(){}
    QCString queryPlugInInterface() const { return "QSqlDriverInterface"; }

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

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QSqlDriverInterface* loadInterface()
{
    return new QPSQLDriverInterface();
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


