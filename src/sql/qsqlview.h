#ifndef QSQLVIEW_H
#define QSQLVIEW_H

#ifndef QT_H
#include "qstring.h"
#include "qsqlfield.h"
#include "qsql.h"
#include "qsqlindex.h"
#include "qsqlconnection.h"
#endif // QT_H

#ifndef QT_NO_SQL

// View Modes
#define SQL_ReadOnly            0x0000
#define SQL_Insert	        0x0001
#define SQL_Update		0x0002
#define SQL_Delete		0x0004
#define SQL_Writable		0x0007

class QSqlDatabase;
class QSqlViewPrivate;
class Q_EXPORT QSqlView : public QSqlFieldList, public QSql
{
public:
    QSqlView( const QString & name = QString::null, bool autopopulate = TRUE, const QString& databaseName = QSqlConnection::defaultDatabase );
    QSqlView( const QSqlView & s );
    QSqlView& operator=( const QSqlView& s );
    ~QSqlView();

    QVariant          value( int i );
    QSqlIndex         primaryIndex( bool prime = FALSE ) const;
    void              setPrimaryIndex( QSqlIndex idx );
    virtual int       insert( bool invalidate = TRUE );
    virtual int       update( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );
    virtual int       del( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );

    void              setMode( int flags );
    int               mode() const;
    bool              isReadOnly() const;
    bool              canInsert() const;
    bool              canUpdate() const;
    bool              canDelete() const;
    void              detach();

    bool              select();
    bool              select( const QSqlIndex& sort );
    bool              select( const QSqlIndex & filter, const QSqlIndex & sort );
    bool              select( const QString & filter, const QSqlIndex & sort = QSqlIndex() );
    QSqlIndex         sort() const;
    QString           filter() const;
    void              setName( const QString& name, bool autopopulate = TRUE );
    QString           name() const;

protected:
    void              postSeek();

    QSqlFieldList &   operator=( const QSqlFieldList & list );
    bool              setQuery( const QString & str );
    QString           fieldEqualsValue( const QString& prefix, const QString& fieldSep, const QSqlIndex & i = QSqlIndex() );
    virtual QVariant  calculateField( uint fieldNumber );

    virtual int       update( const QString & filter, bool invalidate = TRUE );
    virtual int       del( const QString & filter, bool invalidate = TRUE );

private:
    QSqlFieldList     fields() const;     //hide
    void              sync();
    int               apply( const QString& q, bool invalidate );
    QSqlViewPrivate*  d;
};

#endif // QT_NO_SQL
#endif
