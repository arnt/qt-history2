/****************************************************************************
**
** Definition of QTable widget class
**
** Created : 000607
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the table module of the Qt GUI Toolkit.
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

#ifndef QTABLE_H
#define QTABLE_H

#ifndef QT_H
#include <qscrollview.h>
#include <qpixmap.h>
#include <qptrvector.h>
#include <qheader.h>
#include <qmemarray.h>
#include <qptrlist.h>
#include <qguardedptr.h>
#include <qshared.h>
#include <qintdict.h>
#include <qstringlist.h>
#endif // QT_H


#ifndef QT_NO_TABLE

class QTableHeader;
class QValidator;
class QTable;
class QPaintEvent;
class QTimer;
class QResizeEvent;
class QComboBox;
class QCheckBox;
class QDragObject;

struct QTablePrivate;
struct QTableHeaderPrivate;


class Q_EXPORT QTableSelection
{
public:
    QTableSelection();
    void init( int row, int col );
    void expandTo( int row, int col );
    bool operator==( const QTableSelection &s ) const;

    int topRow() const { return tRow; }
    int bottomRow() const { return bRow; }
    int leftCol() const { return lCol; }
    int rightCol() const { return rCol; }
    int anchorRow() const { return aRow; }
    int anchorCol() const { return aCol; }

    bool isActive() const { return active; }

private:
    uint active : 1;
    uint inited : 1;
    int tRow, lCol, bRow, rCol;
    int aRow, aCol;
};


class Q_EXPORT QTableItem : public Qt
{
public:
    enum EditType { Never, OnTyping, WhenCurrent, Always };

    QTableItem( QTable *table, EditType et, const QString &text );
    QTableItem( QTable *table, EditType et, const QString &text,
                const QPixmap &p );
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
    virtual void setReplaceable( bool );
    bool isReplaceable() const;

    virtual QString key() const;
    virtual QSize sizeHint() const;

    virtual void setSpan( int rs, int cs );
    int rowSpan() const;
    int colSpan() const;

    virtual void setRow( int r );
    virtual void setCol( int c );
    int row() const;
    int col() const;

    virtual void paint( QPainter *p, const QColorGroup &cg,
                        const QRect &cr, bool selected );

    void updateEditor( int oldRow, int oldCol );

    virtual void setEnabled( bool b );
    bool isEnabled() const;

    virtual int rtti() const;
    static int RTTI;

private:
    QString txt;
    QPixmap pix;
    QTable *t;
    EditType edType;
    uint wordwrap : 1;
    uint tcha : 1;
    uint enabled : 1;
    int rw, cl;
    int rowspan, colspan;

};

class Q_EXPORT QComboTableItem : public QTableItem
{
public:
    QComboTableItem( QTable *table, const QStringList &list, bool editable = FALSE );
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor( QWidget *w );
    virtual void paint( QPainter *p, const QColorGroup &cg,
                        const QRect &cr, bool selected );
    virtual void setCurrentItem( int i );
    virtual void setCurrentItem( const QString &i );
    int currentItem() const;
    QString currentText() const;
    int count() const;
    QString text( int i ) const;
    virtual void setEditable( bool b );
    bool isEditable() const;
    virtual void setStringList( const QStringList &l );

    int rtti() const;
    static int RTTI;

private:
    QComboBox *cb;
    QStringList entries;
    int current;
    bool edit;
    static QComboBox *fakeCombo;

};

class Q_EXPORT QCheckTableItem : public QTableItem
{
public:
    QCheckTableItem( QTable *table, const QString &txt );
    virtual QWidget *createEditor() const;
    virtual void setContentFromEditor( QWidget *w );
    virtual void paint( QPainter *p, const QColorGroup &cg,
                        const QRect &cr, bool selected );
    virtual void setChecked( bool b );
    bool isChecked() const;

    int rtti() const;
    static int RTTI;

private:
    QCheckBox *cb;
    bool checked;

};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QVector<QTableItem>;
template class Q_EXPORT QVector<QWidget>;
template class Q_EXPORT QPtrList<QTableSelection>;
template class Q_EXPORT QIntDict<int>;
// MOC_SKIP_END
#endif


class Q_EXPORT QTable : public QScrollView
{
    Q_OBJECT
    Q_PROPERTY( int numRows READ numRows WRITE setNumRows )
    Q_PROPERTY( int numCols READ numCols WRITE setNumCols )
    Q_PROPERTY( bool showGrid READ showGrid WRITE setShowGrid )
    Q_PROPERTY( bool rowMovingEnabled READ rowMovingEnabled WRITE setRowMovingEnabled )
    Q_PROPERTY( bool columnMovingEnabled READ columnMovingEnabled WRITE setColumnMovingEnabled )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool sorting READ sorting WRITE setSorting )

    friend class QTableHeader;
    friend class QComboTableItem;
    friend class QCheckTableItem;

public:
    QTable( QWidget *parent = 0, const char *name = 0 );
    QTable( int numRows, int numCols,
            QWidget *parent = 0, const char *name = 0 );
    ~QTable();

    QHeader *horizontalHeader() const;
    QHeader *verticalHeader() const;

    enum SelectionMode { Single, Multi, NoSelection  };
    virtual void setSelectionMode( SelectionMode mode );
    SelectionMode selectionMode() const;

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
    virtual int columnAt( int x ) const;
    virtual int rowAt( int y ) const;

    virtual int numRows() const;
    virtual int numCols() const;

    void updateCell( int row, int col );

    bool eventFilter( QObject * o, QEvent * );

    int currentRow() const { return curRow; }
    int currentColumn() const { return curCol; }
    void ensureCellVisible( int row, int col );

    bool isSelected( int row, int col ) const;
    bool isRowSelected( int row, bool full = FALSE ) const;
    bool isColumnSelected( int col, bool full = FALSE ) const;
    int numSelections() const;
    QTableSelection selection( int num ) const;
    virtual int addSelection( const QTableSelection &s );
    virtual void removeSelection( const QTableSelection &s );
    virtual void removeSelection( int num );
    virtual int currentSelection() const;

    bool showGrid() const;

    bool columnMovingEnabled() const;
    bool rowMovingEnabled() const;

    virtual void sortColumn( int col, bool ascending = TRUE,
                             bool wholeRows = FALSE );
    bool sorting() const;

    virtual void takeItem( QTableItem *i );

    virtual void setCellWidget( int row, int col, QWidget *e );
    virtual QWidget *cellWidget( int row, int col ) const;
    virtual void clearCellWidget( int row, int col );

    virtual void paintCell( QPainter *p, int row, int col,
                            const QRect &cr, bool selected );
    virtual void paintFocus( QPainter *p, const QRect &r );
    QSize sizeHint() const;

    bool isReadOnly() const;
    bool isRowReadOnly( int row ) const;
    bool isColumnReadOnly( int col ) const;

    void setEnabled( bool b );

    void repaintSelections();

public slots:
    virtual void setNumRows( int r );
    virtual void setNumCols( int r );
    virtual void setShowGrid( bool b );
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
    virtual void setSorting( bool b );
    virtual void swapRows( int row1, int row2 );
    virtual void swapColumns( int col1, int col2 );
    virtual void swapCells( int row1, int col1, int row2, int col2 );

    virtual void setLeftMargin( int m );
    virtual void setTopMargin( int m );
    virtual void setCurrentCell( int row, int col );
    void clearSelection( bool repaint = TRUE );
    virtual void setColumnMovingEnabled( bool b );
    virtual void setRowMovingEnabled( bool b );

    virtual void setReadOnly( bool b );
    virtual void setRowReadOnly( int row, bool ro );
    virtual void setColumnReadOnly( int col, bool ro );

    virtual void setDragEnabled( bool b );
    bool dragEnabled() const;

    virtual void insertRows( int row, int count = 1 );
    virtual void insertColumns( int col, int count = 1 );
    virtual void removeRow( int row );
    virtual void removeRows( const QMemArray<int> &rows );
    virtual void removeColumn( int col );
    virtual void removeColumns( const QMemArray<int> &cols );

    virtual void editCell( int row, int col, bool replace = FALSE );

protected:
    enum EditMode { NotEditing, Editing, Replacing };
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void contentsMousePressEvent( QMouseEvent* );
    void contentsMouseMoveEvent( QMouseEvent* );
    void contentsMouseDoubleClickEvent( QMouseEvent* );
    void contentsMouseReleaseEvent( QMouseEvent* );
    void contentsContextMenuEvent( QContextMenuEvent * e );
    void keyPressEvent( QKeyEvent* );
    void focusInEvent( QFocusEvent* );
    void focusOutEvent( QFocusEvent* );
    void resizeEvent( QResizeEvent * );
    void showEvent( QShowEvent *e );
    void setEditMode( EditMode mode, int row, int col );
#ifndef QT_NO_DRAGANDDROP
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *e );
    virtual QDragObject *dragObject();
    virtual void startDrag();
#endif

    virtual void paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch );
    virtual void activateNextCell();
    virtual QWidget *createEditor( int row, int col, bool initFromCell ) const;
    virtual void setCellContentFromEditor( int row, int col );
    virtual QWidget *beginEdit( int row, int col, bool replace );
    virtual void endEdit( int row, int col, bool accept, bool replace );

    virtual void resizeData( int len );
    virtual void insertWidget( int row, int col, QWidget *w );
    int indexOf( int row, int col ) const;

    void windowActivationChange( bool );

protected slots:
    virtual void columnWidthChanged( int col );
    virtual void rowHeightChanged( int row );
    virtual void columnIndexChanged( int section, int fromIndex, int toIndex );
    virtual void rowIndexChanged( int section, int fromIndex, int toIndex );
    virtual void columnClicked( int col );

signals:
    void currentChanged( int row, int col );
    void clicked( int row, int col, int button, const QPoint &mousePos );
    void doubleClicked( int row, int col, int button, const QPoint &mousePos );
    void pressed( int row, int col, int button, const QPoint &mousePos );
    void selectionChanged();
    void valueChanged( int row, int col );
    void contextMenuRequested( int row, int col, const QPoint &pos );
#ifndef QT_NO_DRAGANDDROP
    void dropped( QDropEvent *e );
#endif

private slots:
    void doAutoScroll();
    void doValueChanged();

private:
    void updateGeometries();
    void repaintSelections( QTableSelection *oldSelection,
                            QTableSelection *newSelection,
                            bool updateVertical = TRUE,
                            bool updateHorizontal = TRUE );
    QRect rangeGeometry( int topRow, int leftCol,
                         int bottomRow, int rightCol, bool &optimize );
    void fixRow( int &row, int y );
    void fixCol( int &col, int x );

    void init( int numRows, int numCols );
    QSize tableSize() const;
    bool isEditing() const;
    void repaintCell( int row, int col );
    void contentsToViewport2( int x, int y, int& vx, int& vy );
    QPoint contentsToViewport2( const QPoint &p );
    void viewportToContents2( int vx, int vy, int& x, int& y );
    QPoint viewportToContents2( const QPoint &p );

    void updateRowWidgets( int row );
    void updateColWidgets( int col );

private:
    QPtrVector<QTableItem> contents;
    QPtrVector<QWidget> widgets;
    int curRow;
    int curCol;
    QTableHeader *leftHeader, *topHeader;
    EditMode edMode;
    int editCol, editRow;
    QPtrList<QTableSelection> selections;
    QTableSelection *currentSel;
    QTimer *autoScrollTimer;
    int lastSortCol;
    bool sGrid : 1;
    bool mRows : 1;
    bool mCols : 1;
    bool asc : 1;
    bool doSort : 1;
    bool mousePressed : 1;
    bool readOnly : 1;
    bool shouldClearSelection : 1;
    bool dEnabled : 1;
    bool context_menu : 1;
    SelectionMode selMode;
    int pressedRow, pressedCol;
    QTablePrivate *d;
    QIntDict<int> roRows;
    QIntDict<int> roCols;
    int startDragRow;
    int startDragCol;
    QPoint dragStartPos;
    int oldCurrentRow, oldCurrentCol;

};

#endif // QT_NO_TABLE
#endif // TABLE_H
