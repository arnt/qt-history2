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
    QODBCDriverInterface(){ qDebug("QODBCDriverInterface()");}
    ~QODBCDriverInterface(){ qDebug("~QODBCDriverInterface(){}");}

    QCString queryPlugInInterface() const { return "QSqlDriverInterface"; }

    QSqlDriver* create( const QString &name );
    QStringList featureList();
};


QSqlDriver* QODBCDriverInterface::create( const QString &name )
{
    qDebug("ODBC create called");
    if ( name == "QODBC" ) {
	qDebug("ODBC returning pointer to new ODBC driver");
	return new QODBCDriver();
    }
    qDebug("ODBC create returning 0");
    return 0;
}

QStringList QODBCDriverInterface::featureList()
{
    qDebug("QODBCDriverInterface::featureList()");
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
    qDebug("creating ODBC interface");
    return new QODBCDriverInterface();
}

LIBEXPORT bool onConnect(QApplication* p)
{
    qDebug("app: %p", p);
    qDebug("connecting ODBC.so");
    return TRUE;
}

LIBEXPORT bool onDisconnect(QApplication* p)
{
    qDebug("disconnecting ODBC.so");
    qDebug("app: %p", p);
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus

