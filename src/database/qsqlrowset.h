#ifndef QSQLROWSET_H
#define QSQLROWSET_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qmap.h"
#include "qsql.h"
#include "qsqlerror.h"
#include "qsqlindex.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDatabase;

class QSqlRowset : public QSqlFieldList
{
public:
    QSqlRowset( QSqlDatabase * db, const QString & table );
    QSqlRowset( const QSqlRowset & s );

    bool select( const QString & filter = QString::null, const QSqlIndex & sort = QSqlIndex() );
    bool select( const QSqlIndex & filter, const QSqlIndex & sort = QSqlIndex() );

    bool seek( int i, bool relative = FALSE);
    bool next();
    bool previous();
    bool first();
    bool last();
    QSqlError lastError() const { return r ? r->lastError() : QSqlError() ; }
    int  affectedRows();
    QString name() const { return tableName; }
    
    void dumpRecords();

protected:
    QSqlFieldList & operator=( const QSqlFieldList & list );
    bool query( const QString & str );
    QString whereClause( const QSqlIndex & i );

private:
    void updateFieldValues();

    QSqlDatabase * db;
    QString   tableName;    
    QSql *    r;
    QSqlIndex pk;
};

#endif // QT_NO_SQL
#endif
