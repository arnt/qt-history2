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

// #define SQL_ReadOnly	        0x0001		
// #define SQL_WriteOnly		0x0002		
// #define SQL_ReadWrite		0x0003		
// #define SQL_Append		0x0004		
// #define SQL_Truncate		0x0008		
// #define SQL_Translate		0x0010	
// #define SQL_ModeMask		0x00ff

class QSqlDatabase;

class Q_EXPORT QSqlView : public QSqlFieldList, public QSql
{
public:
    QSqlView( const QString & name = QString::null, const QString& databaseName = QSqlConnection::defaultDatabase );
    QSqlView( const QSqlView & s );
    QSqlView& operator=( const QSqlView& s );
    ~QSqlView();

    QVariant&         operator[]( int i );
    QVariant&         operator[]( const QString& name );
    QVariant          value( int i );
    QVariant          value( const QString& name );
    void              setValue( int i, const QVariant& value );
    void              setValue( const QString& name, const QVariant& value );

    QSqlIndex         primaryIndex() const;
    virtual int       insert( bool invalidate = TRUE );
    virtual int       update( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );
    virtual int       del( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );
    
    void              setMode( int flags );
    int               mode() const;

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
    virtual QVariant  calculateField( uint fieldNumber );
    
    virtual int       update( const QString & filter, bool invalidate = TRUE );
    virtual int       del( const QString & filter, bool invalidate = TRUE );
    
private:
    QSqlFieldList     fields() const;     //hide
    void              sync();
    int               lastAt;
    QString           nm;
    QSqlIndex         srt;
    QString           ftr;
    int               apply( const QString& q, bool invalidate );
};

#endif // QT_NO_SQL
#endif
