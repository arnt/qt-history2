#ifndef QSQLTABLE_H
#define QSQLTABLE_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qtable.h"
#include "qpainter.h"
#include "qsql.h"
#include "qsqlrowset.h"
#include "qsqlview.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlTablePrivate;
class Q_EXPORT QSqlTable : public QTable
{
    Q_OBJECT
public:
    QSqlTable ( QWidget * parent = 0, const char * name = 0 );
    ~QSqlTable();

    void         setNullText( const QString& nullText );
    QString      nullText();

    void         addColumn( const QSqlField& field );
    void         removeColumn( uint col );
    void         setColumn( uint col, const QSqlField& field );
    void         addColumns( const QSqlFieldList& fieldList );

    void         setQuery( const QString& query, const QString& databaseName = QSqlConnection::defaultDatabase, bool autoPopulate = TRUE );
    void         setQuery( const QSql& query, bool autoPopulate = TRUE );
    void         setRowset( const QString& name, const QString& databaseName = QSqlConnection::defaultDatabase, bool autoPopulate = TRUE );
    void         setRowset( const QSqlRowset& rowset, bool autoPopulate = TRUE );
    void         setView( const QString& name, const QString& databaseName = QSqlConnection::defaultDatabase, bool autoPopulate = TRUE );
    void         setView( const QSqlView& view, bool autoPopulate = TRUE );
    void         sortColumn ( int col, bool ascending = TRUE,
			      bool wholeRows = FALSE );
    QString      text ( int row, int col ) const;
    QVariant     value ( int row, int col ) const;

protected:
    QWidget *    createEditor( int row, int col, bool initFromCell ) const;
    int          indexOf( uint i );
    void         reset();
    void         setSize( const QSql* sql );
    void         setNumRows ( int r );
    void         paintCell ( QPainter * p, int row, int col, const QRect & cr,
			     bool selected );
    void         columnClicked ( int col );
    void         resizeData ( int len );

    QTableItem * item ( int row, int col ) const;
    void         setItem ( int row, int col, QTableItem * item );
    void         clearCell ( int row, int col ) ;
    void         setPixmap ( int row, int col, const QPixmap & pix );
    void         takeItem ( QTableItem * i );

private slots:
    void         loadNextPage();
    void         loadLine( int l );

private:
    void         setNumCols ( int r );
    QSqlTablePrivate* d;
};

#endif
#endif
