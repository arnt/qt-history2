#ifndef QSQLTABLE_H
#define QSQLTABLE_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qtable.h"
#include "qpainter.h"
#include "qsqldatabase.h"
#include "qsql.h"
#include "qsqlview.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qsqleditorfactory.h"
#include "qsqlform.h"
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

    void         addColumn( const QSqlField* field );
    void         removeColumn( uint col );
    void         setColumn( uint col, const QSqlField* field );
    void         addColumns( const QSqlFieldList& fieldList );

    void         setView( QSqlView* view = 0, bool autoPopulate = TRUE );
    QSqlView*    view() const;
    
    void         setReadOnly( bool b );
    bool         isReadOnly() const;

    void         sortColumn ( int col, bool ascending = TRUE,
			      bool wholeRows = FALSE );
    QString      text ( int row, int col ) const;
    QVariant     value ( int row, int col ) const;
    QSqlFieldList currentFieldSelection() const;

    void         installEditorFactory( QSqlEditorFactory * f );
    void         installPropertyMap( QSqlPropertyMap* m );

signals:
    void         currentChanged( const QSqlFieldList* fields );

public slots:
    void 	 find( const QString & str, bool caseSensitive,
			     bool backwards );

protected slots:
    virtual bool insertCurrent();
    virtual bool updateCurrent();    
    virtual bool deleteCurrent();
 
protected:
    virtual bool primeInsert( QSqlView* view );
    virtual bool primeUpdate( QSqlView* view );
    virtual bool primeDelete( QSqlView* view );    
    
    bool         eventFilter( QObject *o, QEvent *e );
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
    void refresh( QSqlView* view );    
    void         setNumCols ( int r );
    QSqlTablePrivate* d;
};

#endif
#endif
