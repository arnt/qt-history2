#ifndef QSQLCONNECTION_H
#define QSQLCONNECTION_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qdict.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDatabase;

class Q_EXPORT QSqlConnection : public QObject
{
public:
    static QSqlDatabase* database( const QString& name = "default" );
    static QSqlDatabase* addDatabase( const QString& type,
				      const QString & db,
				      const QString & user = QString::null,
				      const QString & password = QString::null,
				      const QString & host = QString::null,
				      const QString & name = "default" );
    static void          removeDatabase( const QString& name );
    static void          free();
private:
    static QSqlConnection* instance();
    QSqlConnection( QObject* parent=0, const char* name=0 );
    ~QSqlConnection();
    QDict< QSqlDatabase > dbDict;
};

#endif // QT_NO_SQL
#endif
