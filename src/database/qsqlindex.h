#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#include "qsqlresultinfo.h"
#include "qsqldatabase.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlIndex 
{
public:
    QSqlIndex( const QSqlDatabase* database, const QString& tablename );
    ~QSqlIndex();
    void             append( QSqlFieldInfo field );
    QString          tableName() { return table; }
    QSqlFieldInfoList fields();
private:
    const QString table;
    QSqlFieldInfoList fieldList;
    const QSqlDatabase* db;
};

#endif	// QT_NO_SQL
#endif
