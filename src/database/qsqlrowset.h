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

class QSqlRowset : public QSqlFieldList, public QSql
{
public:
    QSqlRowset( QSqlDatabase * db, const QString & table );
    QSqlRowset( const QSqlRowset & s );

    QVariant& operator[]( int i );
    QVariant& operator[]( const QString& name );

    bool select( const QString & filter = QString::null, const QSqlIndex & sort = QSqlIndex() );
    bool select( const QSqlIndex & filter, const QSqlIndex & sort = QSqlIndex() );
    QString name() const { return tableName; }

    void dumpRecords();

protected:
    QSqlFieldList & operator=( const QSqlFieldList & list );
    bool query( const QString & str );
    QString whereClause( const QSqlIndex & i );

private:
    void      updateFieldValues();
    int       lastAt;
    QString   tableName;
};

#endif // QT_NO_SQL
#endif
