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
#include "qsqlconnection.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDatabase;

class Q_EXPORT QSqlRowset : public QSqlFieldList, public QSql
{
public:
    QSqlRowset( const QString & name = QString::null, const QString& databaseName = QSqlConnection::defaultDatabase );
    QSqlRowset( const QSqlRowset & s );
    QSqlRowset& operator=( const QSqlRowset & s );

    QVariant& operator[]( int i );
    QVariant& operator[]( const QString& name );
    QVariant& value( int i );
    QVariant& value( const QString& name );

    bool select();
    bool select( const QSqlIndex& sort );
    bool select( const QSqlIndex & filter, const QSqlIndex & sort );
    bool select( const QString & filter, const QSqlIndex & sort = QSqlIndex() );
    void    setName( const QString& name );
    QString name() const { return nm; }

protected:
    QSqlFieldList & operator=( const QSqlFieldList & list );
    bool query( const QString & str );
    QString fieldEqualsValue( const QString& fieldSep, const QSqlIndex & i = QSqlIndex() );
    QVariant calculateField( uint fieldNumber );

private:
    QSqlFieldList   fields() const;     //hide
    void      sync();
    int       lastAt;
    QString   nm;
};

#endif // QT_NO_SQL
#endif
