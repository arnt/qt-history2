#ifndef QSQLTABLE_H
#define QSQLTABLE_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qtable.h"
#include "qpainter.h"
#include "qsqldatabase.h"
#include "qsql.h"
#include "qsqlrowset.h"
#include "qsqlview.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qsqleditorfactory.h"
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
    QString      nullText() const;
    void         setTrueText( const QString& trueText );
    QString      trueText() const;
    void         setFalseText( const QString& falseText );
    QString      falseText() const;

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
    QSqlFieldList currentFieldSelection() const;

    void         setEditorFactory( QSqlEditorFactory * f );

signals:
    void         currentChanged( const QSqlFieldList* fields );

public slots:
    void 	 findString( const QString & str, bool caseSensitive,
			     bool backwards );

protected:
    QWidget *    createEditor( int row, int col, bool initFromCell ) const;
    int          indexOf( uint i ) const;
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
    void 	 setCellContentFromEditor( int row, int col );
private slots:
    void         loadNextPage();
    void         loadLine( int l );
    void         setCurrentSelection( int row, int col );

private:
    void         setNumCols ( int r );
    QSqlTablePrivate* d;
};

#endif
#endif
