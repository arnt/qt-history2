#include "../../qsqldriverinterface.h"
#include "qsql_odbc.h"
#include <qstringlist.h>
#include <qapplication.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class QODBCDriverInterface : public QSqlDriverInterface
{
public:
    QODBCDriverInterface(){}
    ~QODBCDriverInterface(){}

    QCString queryPlugInInterface() const { return "QSqlDriverInterface"; }

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

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QSqlDriverInterface* loadInterface()
{
    return new QODBCDriverInterface();
}

#if defined(__cplusplus)
}
#endif // __cplusplus

