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

class QTableItem : public Qt, public QShared
{
    friend class QTable;

public:
    enum EditType { Never, OnCurrent, OnActivate, Always };

    QTableItem( QTable *table, const QString &t );
    QTableItem( QTable *table, const QString &t, const QPixmap &p );
    virtual ~QTableItem();

    virtual QPixmap pixmap() const;
    virtual QString text() const;
    virtual void setPixmap( const QPixmap &p );
    virtual void setText( const QString &t );
    QTable *table() const { return t; }

    virtual int alignment() const;
    virtual void setWordWrap( bool b );
    bool wordWrap() const;

    virtual void setEditType( EditType );
    EditType editType() const;
    virtual QWidget *editor() const;
    virtual void setContentFromEditor( QWidget *w );
    virtual void setTypeChangeAllowed( bool );
    bool isTypeChangeAllowed() const;

    virtual QString key() const;
    virtual QSize sizeHint() const;

    virtual void setSpan( int rs, int cs );
    int rowSpan() const;
    int columnSpan() const;

protected:
    virtual void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );

private:
    QString txt;
    QPixmap pix;
    QTable *t;
    EditType edType;
    uint wordwrap : 1;
    uint tcha : 1;
    QGuardedPtr<QWidget> lastEditor;
    int row, col;
    int rowspan, colspan;

};

class QTable : public QScrollView
{
    Q_OBJECT
    friend class QTableItem;
    friend class QTableHeader;

public:
    enum EditMode { NotEditing, Editing, Replacing };

    QTable( int numRows, int numCols, QWidget* parent=0, const char* name=0 );
    ~QTable();

    QHeader *horizontalHeader() const;
    QHeader *verticalHeader() const;

    virtual void setCellContent( int row, int col, QTableItem *item );
    virtual void setCellText( int row, int col, const QString &text );
    virtual void setCellPixmap( int row, int col, const QPixmap &pix );
    QTableItem *cellContent( int row, int col ) const;
    QString cellText( int row, int col ) const;
    QPixmap cellPixmap( int row, int col ) const;
    virtual void clearCell( int row, int col );

    virtual QRect cellGeometry( int row, int col ) const;
    virtual int columnWidth( int col ) const;
    virtual int rowHeight( int row ) const;
    virtual int columnPos( int col ) const;
    virtual int rowPos( int row ) const;
    virtual int columnAt( int pos ) const;
    virtual int rowAt( int pos ) const;
    QSize tableSize() const;

    virtual void setRows( int r );
    virtual void setColumns( int r );
    int rows() const;
    int columns() const;

    void updateCell( int row, int col );

    virtual void setDefaultValidator( QValidator *validator );
    virtual QValidator *defaultValidator() const;
    virtual QWidget *defaultEditor() const;
    QWidget *editor( int row, int col, bool initFromCell ) const;
    virtual void beginEdit( int row, int col, bool replace );
    virtual void endEdit( int row, int col, bool accept, QWidget *editor, EditMode mode );
    virtual void setCellContentFromEditor( int row, int col, QWidget *editor );
    bool isEditing() const;
    EditMode editMode() const;

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

    void setColumnsMovable( bool b );
    bool columnsMovable() const;
    void setRowsMovable( bool b );
    bool rowsMovable() const;

    virtual void sortColumn( int col, bool ascending = TRUE );
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

protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
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

    void paintCell( QPainter *p, int row, int col, const QRect &cr, bool selected );
    int indexOf( int row, int col ) const;
    void updateGeometries();
    void repaintSelections( SelectionRange *oldSelection, SelectionRange *newSelection,
			    bool updateVertical = TRUE, bool updateHorizontal = TRUE );
    QRect rangeGeometry( int topRow, int leftCol, int bottomRow, int rightCol );
    void fixRow( int &row, int y );
    void fixCol( int &col, int x );
    void editTypeChanged( QTableItem *i, QTableItem::EditType old );

private:
    QVector<QTableItem> contents;
    int curRow;
    int curCol;
    QTableHeader *leftHeader, *topHeader;
    QValidator *defValidator;
    EditMode edMode;
    int editCol, editRow;
    QGuardedPtr<QWidget> editorWidget;
    QList<SelectionRange> selections;
    SelectionRange *currentSelection;
    QTimer *autoScrollTimer;
    bool sGrid, mRows, mCols;
    int lastSortCol;
    bool asc;
    bool doSort;

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

private:
    void updateSelections();
    void saveStates();
    void setCaching( bool b );

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
