/****************************************************************************
**
** Definition of QTable widget class
**
** Created : 000607
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QTABLE_H
#define QTABLE_H

#include <qscrollview.h>
#include <qpixmap.h>
#include <qvector.h>
#include <qheader.h>
#include <qarray.h>
#include <qlist.h>
#include <qguardedptr.h>
#include <qshared.h>

class QTableHeader;
class QValidator;
class QTable;
class QPaintEvent;
class QTimer;
class QResizeEvent;

class QTableItem : public Qt
{
    friend class QTable;

public:
    enum EditType { Never, OnCurrent, OnActivate, Always };

    QTableItem( QTable *table, EditType et, const QString &t );
    QTableItem( QTable *table, EditType et, const QString &t, const QPixmap &p );
    virtual ~QTableItem();

    virtual QPixmap pixmap() const;
    virtual QString text() const;
    virtual void setPixmap( const QPixmap &p );
    virtual void setText( const QString &t );
    QTable *table() const { return t; }

    virtual int alignment() const;
    virtual void setWordWrap( bool b );
    bool wordWrap() const;

    EditType editType() const;
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor( QWidget *w );
    virtual void setReplacable( bool );
    bool isReplacable() const;

    virtual QString key() const;
    virtual QSize sizeHint() const;

    virtual void setSpan( int rs, int cs );
    int rowSpan() const;
    int colSpan() const;

    virtual void setRow( int r );
    virtual void setCol( int c );
    int row() const;
    int col() const;
    
protected:
    virtual void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );

private:
    void updateEditor( int oldRow, int oldCol );

    QString txt;
    QPixmap pix;
    QTable *t;
    EditType edType;
    uint wordwrap : 1;
    uint tcha : 1;
    int rw, cl;
    int rowspan, colspan;

};

class QTable : public QScrollView
{
    Q_OBJECT
    Q_PROPERTY( int numRows READ numRows WRITE setNumRows )
    Q_PROPERTY( int numCols READ numCols WRITE setNumCols )
    Q_PROPERTY( bool showGrid READ showGrid WRITE setShowGrid )
    Q_PROPERTY( bool rowMovingEnabled READ rowMovingEnabled WRITE setRowMovingEnabled )
    Q_PROPERTY( bool columnMovingEnabled READ columnMovingEnabled WRITE setColumnMovingEnabled )

    friend class QTableItem;
    friend class QTableHeader;

public:
    QTable( QWidget *parent = 0, const char *name = 0 );
    QTable( int numRows, int numCols, QWidget *parent = 0, const char *name = 0 );
    ~QTable();

    QHeader *horizontalHeader() const;
    QHeader *verticalHeader() const;

    virtual void setItem( int row, int col, QTableItem *item );
    virtual void setText( int row, int col, const QString &text );
    virtual void setPixmap( int row, int col, const QPixmap &pix );
    virtual QTableItem *item( int row, int col ) const;
    virtual QString text( int row, int col ) const;
    virtual QPixmap pixmap( int row, int col ) const;
    virtual void clearCell( int row, int col );

    virtual QRect cellGeometry( int row, int col ) const;
    virtual int columnWidth( int col ) const;
    virtual int rowHeight( int row ) const;
    virtual int columnPos( int col ) const;
    virtual int rowPos( int row ) const;
    virtual int columnAt( int pos ) const;
    virtual int rowAt( int pos ) const;

    virtual void setNumRows( int r );
    virtual void setNumCols( int r );
    int numRows() const;
    int numCols() const;

    void updateCell( int row, int col );

    bool eventFilter( QObject * o, QEvent * );

    void setCurrentCell( int row, int col );
    int currentRow() const { return curRow; }
    int currentColumn() const { return curCol; }
    void ensureCellVisible( int row, int col );

    bool isSelected( int row, int col ) const;
    bool isRowSelected( int row, bool full = FALSE ) const;
    bool isColumnSelected( int col, bool full = FALSE ) const;
    void clearSelection();
    int selectionCount() const;
    bool selection( int num, int &topRow, int &leftCol, int &bottomRow, int &rightCol );

    void setShowGrid( bool b );
    bool showGrid() const;

    void setColumnMovingEnabled( bool b );
    bool columnMovingEnabled() const;
    void setRowMovingEnabled( bool b );
    bool rowMovingEnabled() const;

    virtual void sortColumn( int col, bool ascending = TRUE, bool wholeRows = FALSE );
    virtual void setSorting( bool b );
    bool sorting() const;

    virtual void hideRow( int row );
    virtual void hideColumn( int col );
    virtual void showRow( int row );
    virtual void showColumn( int col );

    virtual void setColumnWidth( int col, int w );
    virtual void setRowHeight( int row, int h );

    virtual void adjustColumn( int col );
    virtual void adjustRow( int row );

    virtual void setColumnStretchable( int col, bool stretch );
    virtual void setRowStretchable( int row, bool stretch );
    bool isColumnStretchable( int col ) const;
    bool isRowStretchable( int row ) const;

    virtual void takeItem( QTableItem *i );

    virtual void setCellWidget( int row, int col, QWidget *e );
    virtual QWidget *cellWidget( int row, int col ) const;
    virtual void clearCellWidget( int row, int col );

    virtual void swapRows( int row1, int row2 );
    virtual void swapColumns( int col1, int col2 );
    virtual void swapCells( int row1, int col1, int row2, int col2 );
    
protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    virtual void paintCell( QPainter *p, int row, int col, const QRect &cr, bool selected );
    void contentsMousePressEvent( QMouseEvent* );
    void contentsMouseMoveEvent( QMouseEvent* );
    void contentsMouseDoubleClickEvent( QMouseEvent* );
    void contentsMouseReleaseEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void focusInEvent( QFocusEvent* );
    void focusOutEvent( QFocusEvent* );
    void resizeEvent( QResizeEvent * );
    void showEvent( QShowEvent *e );

    virtual void paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch );
    virtual void activateNextCell();
    bool focusNextPrevChild( bool next );
    virtual QWidget *createEditor( int row, int col, bool initFromCell ) const;
    virtual void setCellContentFromEditor( int row, int col );
    virtual QWidget *beginEdit( int row, int col, bool replace );
    virtual void endEdit( int row, int col, bool accept, bool replace );

    virtual void resizeData( int len );
    virtual void insertWidget( int row, int col, QWidget *w );
    int indexOf( int row, int col ) const;

protected slots:
    virtual void columnWidthChanged( int col );
    virtual void rowHeightChanged( int row );
    virtual void columnIndexChanged( int s, int oi, int ni );
    virtual void rowIndexChanged( int s, int oi, int ni );
    virtual void columnClicked( int col );

signals:
    void currentChanged( int row, int col );

private slots:
    void doAutoScroll();

private:
    struct SelectionRange
    {
	SelectionRange()
	    : active( FALSE ), topRow( -1 ), leftCol( -1 ), bottomRow( -1 ),
	      rightCol( -1 ), anchorRow( -1 ), anchorCol( -1 ) {}
	void init( int row, int col );
	void expandTo( int row, int col );
	bool operator==( const SelectionRange &s ) const;
	
	bool active;
	int topRow, leftCol, bottomRow, rightCol;
	int anchorRow, anchorCol;
    };
    enum EditMode { NotEditing, Editing, Replacing };

    void updateGeometries();
    void repaintSelections( SelectionRange *oldSelection, SelectionRange *newSelection,
			    bool updateVertical = TRUE, bool updateHorizontal = TRUE );
    QRect rangeGeometry( int topRow, int leftCol, int bottomRow, int rightCol, bool &optimize );
    void fixRow( int &row, int y );
    void fixCol( int &col, int x );

    void init( int numRows, int numCols );
    QSize tableSize() const;
    bool isEditing() const;
    void repaintCell( int row, int col );

private:
    QVector<QTableItem> contents;
    QVector<QWidget> widgets;
    int curRow;
    int curCol;
    QTableHeader *leftHeader, *topHeader;
    EditMode edMode;
    int editCol, editRow;
    QList<SelectionRange> selections;
    SelectionRange *currentSelection;
    QTimer *autoScrollTimer;
    bool sGrid, mRows, mCols;
    int lastSortCol;
    bool asc;
    bool doSort;
    bool mousePressed;

};

class QTableHeader : public QHeader
{
    Q_OBJECT

public:
    enum SectionState {
	Normal,
	Bold,
	Selected
    };

    QTableHeader( int, QTable *t, QWidget *parent=0, const char *name=0 );
    void addLabel( const QString &s );

    void setSectionState( int s, SectionState state );
    SectionState sectionState( int s ) const;

    int sectionSize( int section ) const;
    int sectionPos( int section ) const;
    int sectionAt( int section ) const;

    void setSectionStretchable( int s, bool b );
    bool isSectionStretchable( int s ) const;

signals:
    void sectionSizeChanged( int s );

protected:
    void paintEvent( QPaintEvent *e );
    void paintSection( QPainter *p, int index, QRect fr );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );
    void resizeEvent( QResizeEvent *e );

private slots:
    void doAutoScroll();
    void sectionWidthChanged( int col, int os, int ns );
    void indexChanged( int sec, int oldIdx, int newIdx );

private:
    void updateSelections();
    void saveStates();
    void setCaching( bool b );
    void swapSections( int oldIdx, int newIdx );

private:
    QArray<SectionState> states, oldStates;
    QArray<bool> stretchable;
    QArray<int> sectionSizes, sectionPoses;
    bool mousePressed;
    int pressPos, startPos, endPos;
    QTable *table;
    QTimer *autoScrollTimer;
    QWidget *line1, *line2;
    bool caching;
    int resizedSection;
    bool isResizing;
    int numStretchs;

};

#endif // TABLE_H
