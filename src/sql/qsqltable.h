#ifndef QSQLTABLE_H
#define QSQLTABLE_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qtable.h"
#include "qpainter.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "qsqlcursor.h"
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


    void         addColumn( const QSqlField* field );
    void         removeColumn( uint col );
    void         setColumn( uint col, const QSqlField* field );
    void         addColumns( const QSqlRecord& fieldList );

    QString      nullText() const;
    QString      trueText() const;
    QString      falseText() const;
    bool         confirmEdits() const;
    bool         confirmCancels() const;
    bool         isReadOnly() const;
    bool         isColumnReadOnly( int col ) const;

    virtual void setCursor( QSqlCursor* cursor = 0, bool autoPopulate = TRUE );
    virtual void setNullText( const QString& nullText );
    virtual void setTrueText( const QString& trueText );
    virtual void setFalseText( const QString& falseText );
    virtual void setConfirmEdits( bool confirm );
    virtual void setConfirmCancels( bool confirm );
    virtual void setAutoDelete( bool enable );
    virtual void setReadOnly( bool b );
    virtual void setColumnReadOnly( int col, bool b );

    QSqlCursor*  cursor() const;

    void         sortColumn ( int col, bool ascending = TRUE,
			      bool wholeRows = FALSE );
    void         refresh( QSqlIndex idx = QSqlIndex() );
    QString      text ( int row, int col ) const;
    QVariant     value ( int row, int col ) const;
    QSqlRecord   currentFieldSelection() const;

    void         installEditorFactory( QEditorFactory * f );
    void         installPropertyMap( QSqlPropertyMap* m );

signals:
    void         currentChanged( const QSqlRecord* record );

public slots:
    void 	 find( const QString & str, bool caseSensitive,
			     bool backwards );

protected slots:
    virtual void insertCurrent();
    virtual void updateCurrent();
    virtual void deleteCurrent();

protected:
    friend class QSqlTablePrivate;
    enum Confirm {
	Yes = 0,
	No = 1,
	Cancel = 2
    };
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
    virtual bool primeInsert( QSqlCursor* cursor );
    virtual bool primeUpdate( QSqlCursor* cursor );
    virtual bool primeDelete( QSqlCursor* cursor );

    bool         eventFilter( QObject *o, QEvent *e );
    void         resizeEvent ( QResizeEvent * );
    void         contentsMousePressEvent( QMouseEvent* e );
    void         endEdit( int row, int col, bool accept, bool replace );
    QWidget *    createEditor( int row, int col, bool initFromCell ) const;
    //    void         setCurrentCell( int row, int col );
    void         activateNextCell();
    int          indexOf( uint i ) const;
    void         reset();
    void         setSize( const QSqlCursor* sql );
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
    void         refresh( QSqlCursor* cursor, QSqlIndex idx = QSqlIndex() );
    void         setNumCols ( int r );
    void         updateRow( int row );
    void         endInsert();
    void         endUpdate();
    QSqlTablePrivate* d;
};

#endif
#endif
