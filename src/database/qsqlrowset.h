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
    
    bool insert();
    bool update( const QSqlIndex & i );
    bool del( const QSqlIndex & i );

    bool select();
    bool select( const QSqlIndex & i, const QString & filter );
    bool select( const QSqlIndex & i );
    bool select( const QSqlIndex & i, const QSqlIndex & j );

    bool seek( int i, bool relative = FALSE);
    bool next();
    bool previous();
    bool first();
    bool last();

    int  affectedRows();
    int  numFields();
    void dumpRecords();

protected:
    QSqlFieldList & operator=( const QSqlFieldList & list );
    bool query( const QString & str );
    
    QString whereClause( const QSqlIndex & i );
    QString orderByClause( const QSqlIndex & i );
    QString fieldOrderClause( const QSqlIndex & i );

private:
    void updateFieldValues();
        
    QSqlDatabase * db;
    QSql *    r;
    QSqlIndex pk;
    QString   tableName;
    QSqlError err;
};

#endif // QT_NO_SQL
#endif
