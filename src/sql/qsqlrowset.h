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
    ~QSqlRowset();
    QSqlRowset&       operator=( const QSqlRowset & s );

    QVariant&         operator[]( int i );
    QVariant&         operator[]( const QString& name );
    QVariant          value( int i );
    QVariant          value( const QString& name );
    void              setValue( int i, const QVariant& value );
    void              setValue( const QString& name, const QVariant& value );

    bool              select();
    bool              select( const QSqlIndex& sort );
    bool              select( const QSqlIndex & filter, const QSqlIndex & sort );
    bool              select( const QString & filter, const QSqlIndex & sort = QSqlIndex() );
    QSqlIndex         sort() const { return srt; }
    QString           filter() const { return ftr; }
    void              setName( const QString& name );
    QString           name() const { return nm; }

protected:
    QSqlFieldList &   operator=( const QSqlFieldList & list );
    bool              setQuery( const QString & str );
    QString           fieldEqualsValue( const QString& prefix, const QString& fieldSep, const QSqlIndex & i = QSqlIndex() );
    QVariant          calculateField( uint fieldNumber );

private:
    QSqlFieldList     fields() const;     //hide
    void              sync();
    int               lastAt;
    QString           nm;
    QSqlIndex         srt;
    QString           ftr;
};

#endif // QT_NO_SQL
#endif
