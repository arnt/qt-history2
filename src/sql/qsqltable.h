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
#include "qeditorfactory.h"
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
    void         setConfirmEdits( bool confirm );
    bool         confirmEdits() const;
    void         setConfirmCancels( bool confirm );
    bool         confirmCancels() const;

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
    void         refresh( QSqlIndex idx = QSqlIndex() );    
    QString      text ( int row, int col ) const;
    QVariant     value ( int row, int col ) const;
    QSqlFieldList currentFieldSelection() const;

    void         installEditorFactory( QEditorFactory * f );
    void         installPropertyMap( QSqlPropertyMap* m );

signals:
    void         currentChanged( const QSqlFieldList* fields );

public slots:
    void 	 find( const QString & str, bool caseSensitive,
			     bool backwards );

protected slots:
    virtual void insertCurrent();
    virtual void updateCurrent();
    virtual void deleteCurrent();

protected:
    enum Confirm {
	Yes = 0,
	No = 1,
	Cancel = 2
    };

    friend class QSqlTablePrivate;
    enum Mode {
	None,
	Insert,
	Update,
	Delete
    };

    virtual Confirm confirmEdit( QSqlTable::Mode m );
    virtual Confirm confirmCancel( QSqlTable::Mode m );

    virtual void handleError( const QSqlError& e );

    virtual bool beginInsert();
    virtual QWidget* beginUpdate ( int row, int col, bool replace );
    virtual bool primeInsert( QSqlView* view );
    virtual bool primeUpdate( QSqlView* view );
    virtual bool primeDelete( QSqlView* view );

    bool         eventFilter( QObject *o, QEvent *e );
    void         resizeEvent ( QResizeEvent * );
    void         contentsMousePressEvent( QMouseEvent* e );
    void         endEdit( int row, int col, bool accept, bool replace );
    QWidget *    createEditor( int row, int col, bool initFromCell ) const;
    //    void         setCurrentCell( int row, int col );
    void         activateNextCell();
    int          indexOf( uint i ) const;
    void         reset();
    void         setSize( const QSql* sql );
    void         setNumRows ( int r );
    void         paintCell ( QPainter * p, int row, int col, const QRect & cr,
			     bool selected );
    void         paintField( QPainter * p, const QSqlField* field, const QRect & cr,
			     bool selected );
    int          fieldAlignment( const QSqlField* field );
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
    void         setCurrentSelection( int row, int col );

private:
    QWidget*     beginEdit ( int row, int col, bool replace );
    void         refresh( QSqlView* view, QSqlIndex idx = QSqlIndex() );
    void         setNumCols ( int r );
    void         updateRow( int row );
    void         endInsert();
    void         endUpdate();
    QSqlTablePrivate* d;
};

#endif
#endif
