/****************************************************************************
**
** Definition of QDataTable class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QDATATABLE_H
#define QDATATABLE_H

#include "qfeatures.h"

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qtable.h"
#include "qsql.h"
#include "qsqlcursor.h"
#include "qsqlindex.h"
#include "qsqleditorfactory.h"
#include "qiconset.h"
#endif // QT_H

class QPainter;
class QSqlField;
class QSqlPropertyMap;
class QDataTablePrivate;

class Q_EXPORT QDataTable : public QTable
{
    Q_OBJECT

    Q_PROPERTY( QString nullText READ nullText WRITE setNullText )
    Q_PROPERTY( QString trueText READ trueText WRITE setTrueText )
    Q_PROPERTY( QString falseText READ falseText WRITE setFalseText )
    Q_PROPERTY( bool confirmEdits READ confirmEdits WRITE setConfirmEdits )
    Q_PROPERTY( bool confirmInsert READ confirmInsert WRITE setConfirmInsert )
    Q_PROPERTY( bool confirmUpdate READ confirmUpdate WRITE setConfirmUpdate )
    Q_PROPERTY( bool confirmDelete READ confirmDelete WRITE setConfirmDelete )
    Q_PROPERTY( bool confirmCancels READ confirmCancels WRITE setConfirmCancels )
    Q_PROPERTY( QString filter READ filter WRITE setFilter )
    Q_PROPERTY( QStringList sort READ sort WRITE setSort )
    Q_PROPERTY( int numCols READ numCols )
    Q_PROPERTY( int numRows READ numRows )

public:
    QDataTable ( QWidget * parent = 0, const char * name = 0 );
    QDataTable ( QSqlCursor* cursor, bool autoPopulate = FALSE, QWidget * parent = 0, const char * name = 0 );
    ~QDataTable();

    virtual void addColumn( const QString& fieldName, const QString& label = QString::null, const QIconSet& iconset = QIconSet() );
    virtual void removeColumn( uint col );
    virtual void setColumn( uint col, const QString& fieldName, const QString& label = QString::null, const QIconSet& iconset = QIconSet() );

    QString      nullText() const;
    QString      trueText() const;
    QString      falseText() const;
    bool         confirmEdits() const;
    bool         confirmInsert() const;
    bool         confirmUpdate() const;
    bool         confirmDelete() const;
    bool         confirmCancels() const;
    bool         autoDelete() const;
    QString      filter() const;
    QStringList  sort() const;
    void setCursor ( const QCursor & cursor ) { QTable::setCursor( cursor ); }
    const QCursor& cursor () const { return QTable::cursor(); }

    void setCursor( QSqlCursor* cursor ) { setCursor( cursor, FALSE, FALSE ); }
    virtual void setSqlCursor( QSqlCursor* cursor = 0, bool autoPopulate = FALSE, bool autoDelete = FALSE ) { setCursor( cursor, autoPopulate, autoDelete ); }
    virtual void setSqlCursor( QSqlCursor* cursor ) { setCursor( cursor, FALSE, FALSE ); }
    virtual void setCursor( QSqlCursor* cursor, bool autoPopulate, bool autoDelete = FALSE );
    QSqlCursor* sqlCursor() const;

    virtual void setNullText( const QString& nullText );
    virtual void setTrueText( const QString& trueText );
    virtual void setFalseText( const QString& falseText );
    virtual void setConfirmEdits( bool confirm );
    virtual void setConfirmInsert( bool confirm );
    virtual void setConfirmUpdate( bool confirm );
    virtual void setConfirmDelete( bool confirm );
    virtual void setConfirmCancels( bool confirm );
    virtual void setAutoDelete( bool enable );
    virtual void setFilter( const QString& filter );
    virtual void setSort( const QStringList& sort );
    virtual void setSort( const QSqlIndex& sort );

    void         refresh( bool refreshCursor, bool refreshColumns );
    void         sortColumn ( int col, bool ascending = TRUE,
			      bool wholeRows = FALSE );
    QString      text ( int row, int col ) const;
    QVariant     value ( int row, int col ) const;
    QSqlRecord*  currentRecord() const;

    void         installEditorFactory( QSqlEditorFactory * f );
    void         installPropertyMap( QSqlPropertyMap* m );

    int          numCols() const;
    int          numRows() const;
    void         setNumCols( int c );
    void         setNumRows ( int r );
    bool         findBuffer( const QSqlIndex& idx, int atHint = 0 );

signals:
    void         currentChanged( const QSqlRecord* record );
    void         primeInsert( QSqlRecord* buf );
    void         primeUpdate( QSqlRecord* buf );
    void         primeDelete( QSqlRecord* buf );
    void         beforeInsert( QSqlRecord* buf );
    void         beforeUpdate( QSqlRecord* buf );
    void         beforeDelete( QSqlRecord* buf );
    void         cursorChanged( QSql::Op mode );

public slots:
    virtual void find( const QString & str, bool caseSensitive,
			     bool backwards );
    virtual void sortAscending( int col );
    virtual void sortDescending( int col );
    virtual void refresh();

protected slots:
    virtual void insertCurrent();
    virtual void updateCurrent();
    virtual void deleteCurrent();

protected:
    enum Confirm { //## fix - right now this depends on messagebox
	Yes = 0,
	No = 1,
	Cancel = -1
    };

    virtual Confirm confirmEdit( QSql::Op m );
    virtual Confirm confirmCancel( QSql::Op m );

    virtual void handleError( const QSqlError& e );

    virtual bool beginInsert();
    virtual QWidget* beginUpdate ( int row, int col, bool replace );

    bool         eventFilter( QObject *o, QEvent *e );
    void         resizeEvent ( QResizeEvent * );
    void         contentsMousePressEvent( QMouseEvent* e );
    void         endEdit( int row, int col, bool accept, bool replace );
    QWidget *    createEditor( int row, int col, bool initFromCell ) const;
    void         activateNextCell();
    int          indexOf( uint i ) const;
    void         reset();
    void         setSize( QSqlCursor* sql );
    void         repaintCell( int row, int col );
    void         paintCell ( QPainter * p, int row, int col, const QRect & cr,
			     bool selected );
    virtual void paintField( QPainter * p, const QSqlField* field, const QRect & cr,
			     bool selected );
    virtual int  fieldAlignment( const QSqlField* field );
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
    void         init();
    QWidget*     beginEdit ( int row, int col, bool replace );
    void         updateRow( int row );
    void         endInsert();
    void         endUpdate();
    QDataTablePrivate* d;
};

#endif
#endif
