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

class Q_EXPORT QSqlRowset : public QSqlFieldList, public QSql
{
public:
    QSqlRowset( QSqlDatabase * db, const QString & table );
    QSqlRowset( const QSqlRowset & s );

    QVariant& operator[]( int i );
    QVariant& operator[]( const QString& name );
    QVariant& value( int i ) { return QSqlFieldList::value( i ); }

    bool select();
    bool select( const QSqlIndex& sort );
    bool select( const QSqlIndex & filter, const QSqlIndex & sort );
    QString name() const { return tableName; }

    void dumpRecords();

protected:
    bool select( const QString & filter, const QSqlIndex & sort );
    QSqlFieldList & operator=( const QSqlFieldList & list );
    bool query( const QString & str );
    QString fieldEqualsValue( const QString& fieldSep, const QSqlIndex & i = QSqlIndex() );

private:
    QSqlFieldList   fields() const;     //hide
    void      sync();
    int       lastAt;
    QString   tableName;
};

#endif // QT_NO_SQL
#endif
