/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayout.cpp#72 $
**
** Implementation of layout classes
**
** Created : 960416
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qlayout.h"

#include "qwidget.h"
#include "qlist.h"
#include "qsizepolicy.h"

#include "qlayoutengine.h"


// This cannot be a subclass of QLayoutItem, since it can contain different item classes.
class QLayoutBox
{
public:
    QLayoutBox( QLayoutItem *lit ) { item = lit; }
    //    QLayoutBox( QLayout *layout ) { item = new QLayoutLayoutItem( layout ); }
    QLayoutBox( QWidget *wid ) { item = new QWidgetItem( wid ); }
    QLayoutBox( int w, int h, QSizePolicy::SizeType hData=QSizePolicy::Minimum,
		QSizePolicy::SizeType vData= QSizePolicy::Minimum )
	{ item = new QSpacerItem( w, h, hData, vData ); }
    ~QLayoutBox() { delete item; }

    QSize sizeHint() const { return item->sizeHint(); }
    QSize minimumSize() const { return item->minimumSize(); }
    QSize maximumSize() const { return item->maximumSize(); }
    QSizePolicy::ExpandData expanding() const { return item->expanding(); }
    bool isEmpty() const { return item->isEmpty(); }
    QLayoutItem::SearchResult removeWidget( QWidget *w ) { return item->removeW(w); }

    bool hasHeightForWidth() const { return item->hasHeightForWidth(); }
    int heightForWidth( int w ) const { return item->heightForWidth(w); }

    void setAlignment( int a ) { item->setAlignment( a ); }
    void setGeometry( const QRect &r ) { item->setGeometry( r ); }
    int alignment() const { return item->alignment(); }
private:
    friend class QLayoutArray;
    QLayoutItem *item;
    int row, col;
};


class QMultiBox
{
public:
    QMultiBox( QLayoutBox *box, int toRow, int toCol )
	:box_(box), torow(toRow), tocol(toCol) {}
    QLayoutBox *box() { return box_; }
private:
    friend QLayoutArray;
    QLayoutBox *box_;
    int torow, tocol;
};

class QLayoutArray
{
public:
    QLayoutArray();
    QLayoutArray( int nRows, int nCols );
    ~QLayoutArray();

    void add( QLayoutBox*, int row, int col );
    void add( QLayoutBox*, int row1, int row2, int col1, int col2  );
    QSize sizeHint( int ) const;
    QSize minimumSize( int ) const;
    QSize maximumSize( int ) const;

    QSizePolicy::ExpandData expanding();

    void distribute( QRect, int );
    int numRows() const { return rr; }
    int numCols() const { return cc; }
    void expand( int rows, int cols )
	{ setSize( QMAX(rows,rr), QMAX(cols,cc) ); }
    void setRowStretch( int r, int s ) { expand(r+1,0); rowData[r].stretch=s; }
    void setColStretch( int c, int s ) { expand(0,c+1); colData[c].stretch=s; }

    bool removeWidget( QWidget* );
    void setReversed( bool r, bool c ) { hReversed = c; vReversed = r; }
    void setDirty() { needRecalc = TRUE; hfw_width = -1; }

    bool hasHeightForWidth();
    int heightForWidth( int, int defB );

    bool findWidget( QWidget* w, int *row, int *col );

private:
    void recalcHFW( int w, int s );
    void init();
    QSize findSize( QCOORD QLayoutStruct::*, int ) const;
    void addData ( QLayoutBox *b, bool r = TRUE, bool c = TRUE );
    void setSize( int rows, int cols );
    void setupLayoutData();
    void setupHfwLayoutData();
    int rr;
    int cc;
    bool hReversed;
    bool vReversed;
    QArray<QLayoutStruct> rowData;
    QArray<QLayoutStruct> colData;
    QArray<QLayoutStruct> *hfwData;
    QList<QLayoutBox> things;
    QList<QMultiBox> *multi;
    bool needRecalc;
    bool has_hfw;
    int hfw_width;
    int hfw_height;
};

QLayoutArray::QLayoutArray()
{
    init();
}

QLayoutArray::QLayoutArray( int nRows, int nCols )
    :rowData(0), colData(0)
{
    init();
    setSize( nRows, nCols );
}

QLayoutArray::~QLayoutArray()
{
    delete multi;
    delete hfwData;
}


void QLayoutArray::init()
{
    setDirty();
    multi = 0;
    rr = cc = 0;
    hfwData = 0;
    things.setAutoDelete( TRUE );
    hReversed = vReversed = FALSE;
}

bool QLayoutArray::hasHeightForWidth()
{
    setupLayoutData();
    return has_hfw;
}

/* Assumes that setupLayoutData has been called.
   And that qGeomCalc has filled in colData with appropriate values
*/
void QLayoutArray::recalcHFW( int w, int spacing )
{
    //go through all children, using colData and heightForWidth()
    //and put the results in hfw_rowData
    if ( !hfwData )
	hfwData = new QArray<QLayoutStruct>( rr );
    setupHfwLayoutData();
    QArray<QLayoutStruct> &rData = *hfwData;

    int h = 0;
    int n = 0;
    for ( int r = 0; r < rr; r++ ) {
	h = h + rData[r].sizeHint;
	if ( !rData[r].empty )
	    n++;
    }
    if ( n )
	h += (n-1)*spacing;
    h = QMIN( QCOORD_MAX, h );//not 32-bit safe

    hfw_height = h;
    hfw_width = w;
}

int QLayoutArray::heightForWidth( int w, int spacing )
{
    setupLayoutData();
    if ( has_hfw && w != hfw_width ) {
	qGeomCalc( colData, cc, 0, w, spacing );
	recalcHFW( w, spacing );
    }
    return hfw_height;
}



bool QLayoutArray::findWidget( QWidget* w, int *row, int *col )
{
    QListIterator<QLayoutBox> it( things );
    QLayoutBox * box;
    while ( (box=it.current()) != 0 ) {
	++it;
	if ( box->item->widget() == w ) {
	    if ( row )
		*row = box->row;
	    if ( col )
		*col = box->col;
	    return TRUE;
	}
    }
    if ( multi ) {
	QListIterator<QMultiBox> it( *multi );
	QMultiBox * mbox;
	while ( (mbox=it.current()) != 0 ) {
	    ++it;
	    box = mbox->box();
	    if ( box->item->widget() == w ) {
		if ( row )
		    *row = box->row;
		if ( col )
		    *col = box->col;
		return TRUE;
	    }

	}
    }
    return FALSE;

}




bool QLayoutArray::removeWidget( QWidget *w )
{
    QListIterator<QLayoutBox> it( things );
    QLayoutBox * box;
    while ( (box=it.current()) != 0 ) {
	++it;
	switch ( box->removeWidget( w ) ) {
	case QLayoutItem::NotFound:
	    break;
	case QLayoutItem::FoundAndDeleteable:
	    things.removeRef( box );
	    return TRUE;
	case QLayoutItem::Found:	
	    return TRUE;
	}
    }
    if ( multi ) {
	QListIterator<QMultiBox> it( *multi );
	QMultiBox * mbox;
	while ( (mbox=it.current()) != 0 ) {
	    ++it;
	    box = mbox->box();
	    switch ( box->removeWidget( w ) ) {
	    case QLayoutItem::NotFound:
		break;
	    case QLayoutItem::FoundAndDeleteable:
		multi->removeRef( mbox );
		return TRUE;
	    case QLayoutItem::Found:	
		return TRUE;
	    }
	}
    }
    setDirty();
    return FALSE;
}

QSize QLayoutArray::findSize( QCOORD QLayoutStruct::*size, int spacer ) const
{
    QLayoutArray *This = (QLayoutArray*)this;
    This->setupLayoutData(); //###A very clever optimizer could cause trouble
    int w = 0;
    int h = 0;
    int n = 0;
    for ( int r = 0; r < rr; r++ ) {
	h = h + rowData[r].*size;
	if ( !rowData[r].empty )
	    n++;
    }
    if ( n )
	h += (n-1)*spacer;
    n = 0;
    for ( int c = 0; c < cc; c++ ) {
	w = w + colData[c].*size;
	if ( !colData[c].empty )
	    n++;
    }
    if ( n )
	w += (n-1)*spacer;
    w = QMIN( QCOORD_MAX, w );//not 32-bit safe
    h = QMIN( QCOORD_MAX, h );

    return QSize(w,h);
}

QSizePolicy::ExpandData QLayoutArray::expanding()
{
    setupLayoutData();
    bool hExp = FALSE;
    bool vExp = FALSE;

    for ( int r = 0; r < rr; r++ ) {
       vExp = vExp || rowData[r].expansive;
    }
    for ( int c = 0; c < cc; c++ ) {
	hExp = hExp || colData[c].expansive;
    }

    return (QSizePolicy::ExpandData) (( hExp ? QSizePolicy::Horizontal : 0 )
		  | ( vExp ? QSizePolicy::Vertical : 0 ) );
}

QSize QLayoutArray::sizeHint( int spacer ) const
{
    return findSize( &QLayoutStruct::sizeHint, spacer );
}

QSize QLayoutArray::maximumSize( int spacer ) const
{
    return findSize( &QLayoutStruct::maximumSize, spacer );
}

QSize QLayoutArray::minimumSize( int spacer ) const
{
    return findSize( &QLayoutStruct::minimumSize, spacer );
}

void QLayoutArray::setSize( int r, int c )
{
    if ( (int)rowData.size() < r ) {
	int newR = QMAX(r,rr*2);
	rowData.resize( newR );
	for ( int i = rr; i < newR; i++ )
	    rowData[i].init();
    }
    if ( (int)colData.size() < c ) {
	int newC = QMAX(c,cc*2);
	colData.resize( newC );
	for ( int i = cc; i < newC; i++ )
	    colData[i].init();
    }
    rr = r;
    cc = c;
}

void QLayoutArray::add( QLayoutBox *box, int row, int col )
{
    expand( row+1, col+1 );
    box->row = row;
    box->col = col;
    things.append( box );
    setDirty();
}


void QLayoutArray::add( QLayoutBox *box,  int row1, int row2,
			int col1, int col2  )
{
    expand( row2+1, col2+1 );
    box->row = row1;
    box->col = col1;
    QMultiBox *mbox = new QMultiBox( box, row2, col2 );
    if ( !multi )
	multi = new QList<QMultiBox>;
    multi->append( mbox );
    setDirty();
}

void QLayoutArray::addData ( QLayoutBox *box, bool r, bool c )
{
    QSize hint = box->sizeHint();
    QSize minS = box->minimumSize();
    QSize maxS = box->maximumSize();

    if ( c ) {
    colData[box->col].sizeHint = QMAX( hint.width(),
				      colData[box->col].sizeHint );
    colData[box->col].minimumSize = QMAX( minS.width(),
				      colData[box->col].minimumSize );
    colData[box->col].maximumSize = QMIN( maxS.width(),
				      colData[box->col].maximumSize );
    colData[box->col].expansive = colData[box->col].expansive ||
				  (box->expanding() & QSizePolicy::Horizontal);
    }
    if ( r ) {
    rowData[box->row].sizeHint = QMAX( hint.height(),
				      rowData[box->row].sizeHint );
    rowData[box->row].minimumSize = QMAX( minS.height(),
				      rowData[box->row].minimumSize );
    rowData[box->row].maximumSize = QMIN( maxS.height(),
				      rowData[box->row].maximumSize );
    rowData[box->row].expansive = rowData[box->row].expansive ||
				  (box->expanding() & QSizePolicy::Vertical);
    }
    if ( !box->isEmpty() ) {
	//#### empty boxes ( i.e. spacers) do not get borders. This is hacky, but compatible.
	if ( c )
	    colData[box->col].empty = FALSE;
	if ( r )
	    rowData[box->row].empty = FALSE;
    }

}


static void distributeMultiBox( QArray<QLayoutStruct> &chain,
				int start, int end,
				int minSize, int /*sizeHint*/ )
{
    //###distribute the sizes somehow.
    //####we ignore sizeHint, for now
    int i;
    int w = 0;
    bool exp = FALSE;

    for ( i = start; i <= end; i++ ) {
	w += chain[i].minimumSize;
	exp = exp || chain[i].expansive &&
	             chain[i].maximumSize > chain[i].minimumSize;
    }
    if ( w < minSize ) {
	//	debug( "Big multicell" );
	int diff = minSize - w;
	for ( i = start; i <= end; i++ ) {
	    if ( chain[i].maximumSize > chain[i].minimumSize
		 && ( chain[i].expansive || !exp ) ) {
		chain[i].minimumSize += diff; //#################
		if ( chain[i].sizeHint < chain[i].minimumSize )
		     chain[i].sizeHint = chain[i].minimumSize;
		break;
	    }
	}
    }
}




void QLayoutArray::setupLayoutData()
{
#ifndef QT_LAYOUT_DISABLE_CACHING
    if ( !needRecalc )
        return;
#endif
    has_hfw = FALSE;
    int i;
    for ( i = 0; i < rr; i++ ) {
        rowData[i].initParameters();
    }
    for ( i = 0; i < cc; i++ ) {
        colData[i].initParameters();
    }
    QListIterator<QLayoutBox> it( things );
    QLayoutBox * box;
    while ( (box=it.current()) != 0 ) {
        ++it;
        addData( box );
        has_hfw = has_hfw || box->item->hasHeightForWidth();

    }

    if ( multi ) {
        QListIterator<QMultiBox> it( *multi );
        QMultiBox * mbox;
        while ( (mbox=it.current()) != 0 ) {
            ++it;
            QLayoutBox *box = mbox->box();
            int r1 = box->row;
            int c1 = box->col;
            int r2 = mbox->torow;
            int c2 = mbox->tocol;
            if ( r2 < 0 )
                r2 = rr-1;
            if ( c2 < 0 )
                c2 = cc-1;
            QSize hint = box->sizeHint();
            QSize min = box->minimumSize();
            if ( r1 == r2 ) {
                addData( box, TRUE, FALSE );
            } else {
                distributeMultiBox( rowData, r1, r2,
                                    min.height(), hint.height() );
            }
            if ( c1 == c2 ) {
                addData( box, FALSE, TRUE );
            } else {
                distributeMultiBox( colData, c1, c2,
                                    min.width(), hint.width() );
            }
        }
    }
    for ( i = 0; i < rr; i++ ) {
        rowData[i].expansive = rowData[i].maximumSize > rowData[i].sizeHint &&
                            ( rowData[i].expansive || rowData[i].stretch > 0 );
    }
    for ( i = 0; i < cc; i++ ) {
        colData[i].expansive = colData[i].maximumSize > colData[i].sizeHint &&
                            ( colData[i].expansive || colData[i].stretch > 0 );
    }


    needRecalc = FALSE;
}

/*
  similar to setupLayoutData, but uses
  heightForWidth( colData ) instead of sizeHint
  assumes that setupLayoutData and qGeomCalc( colData ) has been called
 */
void QLayoutArray::setupHfwLayoutData()
{
    QArray<QLayoutStruct> &rData = *hfwData;
    int i;
    for ( i = 0; i < rr; i++ ) {
	rData[i] = rowData[i];
    }
    QListIterator<QLayoutBox> it( things );
    QLayoutBox * box;
    while ( (box=it.current()) != 0 ) {
	++it;
	if ( box->hasHeightForWidth() ) {
	    int hint = box->heightForWidth( colData[box->col].size );
	    rData[box->row].sizeHint = QMAX( hint,
					     rData[box->row].sizeHint );
	    rData[box->row].minimumSize = QMAX( hint,
						rData[box->row].minimumSize );
	}
    }

    if ( multi ) {
	QListIterator<QMultiBox> it( *multi );
	QMultiBox * mbox;
	while ( (mbox=it.current()) != 0 ) {
	    ++it;
	    QLayoutBox *box = mbox->box();
            int r1 = box->row;
            int c1 = box->col;
            int r2 = mbox->torow;
            int c2 = mbox->tocol;
            if ( r2 < 0 )
                r2 = rr-1;
            if ( c2 < 0 )
                c2 = cc-1;
	    QSize hint = box->sizeHint(); //#### must hfw-ify!
	    //(however, distributeMultiBox ignores sizeHint now...)
	    QSize min = box->minimumSize();
	    if ( r1 == r2 ) {
		addData( box, TRUE, FALSE );
	    } else {
		distributeMultiBox( rData, r1, r2,
				    min.height(), hint.height() );
	    }
	}
    }
    for ( i = 0; i < rr; i++ ) {
	rData[i].expansive = rData[i].maximumSize > rData[i].sizeHint &&
			     ( rData[i].expansive || rData[i].stretch > 0 );
    }
}

void QLayoutArray::distribute( QRect r, int spacing )
{
    setupLayoutData();

    qGeomCalc( colData, cc, r.x(), r.width(), spacing );
    QArray<QLayoutStruct> *rDataPtr;
    if ( has_hfw ) {
	recalcHFW( r.width(), spacing );
	qGeomCalc( *hfwData, rr, r.y(), r.height(), spacing );
	rDataPtr = hfwData;
    } else {
	qGeomCalc( rowData, rr, r.y(), r.height(), spacing );
	rDataPtr = &rowData;
    }
    QArray<QLayoutStruct> &rData = *rDataPtr; //cannot assign to reference

    QListIterator<QLayoutBox> it( things );
    QLayoutBox * box;
    while ( (box=it.current()) != 0 ) {
	++it;
	int x = colData[box->col].pos;
	int y = rData[box->row].pos;
	int w = colData[box->col].size;
	int h = rData[box->row].size;
	if ( hReversed )
	    x = r.left() + r.right() - x - w;
	if ( vReversed )
	    y = r.top() + r.bottom() - y - h;
	QRect rr( x, y, w, h );
	box->setGeometry(rr);
    }
    if ( multi ) {
	QListIterator<QMultiBox> it( *multi );
	QMultiBox * mbox;
	while ( (mbox=it.current()) != 0 ) {
	    ++it;
	    QLayoutBox *box = mbox->box();
	    int r2 = mbox->torow;
	    int c2 = mbox->tocol;
	    if ( r2 < 0 )
		r2 = rr-1;
	    if ( c2 < 0 )
		c2 = cc-1;

	    int x = colData[box->col].pos;
	    int y = rData[box->row].pos;
	    int x2 = colData[c2].pos + colData[c2].size;
	    int y2 = rData[r2].pos + rData[r2].size;
	    int w = x2 - x + 1;
	    int h = y2 - y + 1;
	    // this code is copied from above:
	    if ( hReversed )
		x = r.left() + r.right() - x - w;
	    if ( vReversed )
		y = r.top() + r.bottom() - y - h;
	    QRect rr( x, y, w, h );
	    box->setGeometry(rr);
	    //end copying
	
	}
    }
}


/*!
  \class QGridLayout qlayout.h

  \brief The QGridLayout class lays out widgets in a grid.

  \ingroup geomanagement

  QGridLayout takes the space it gets (from its parent layout or from
  the mainWidget()), divides it up into rows and columns, and puts
  each of the widgets it manages into the correct cell(s).

  Columns and rows behave identically; we will discuss columns but
  there are equivalent functions for rows.

  Each column has a minimum width and a stretch factor.  The minimum
  width is the greatest of that set using addColSpacing() and the
  minimum width of each widget in that column.  The stretch factor is
  set using setColStretch() and determines how much of the available
  space the column will get, over and above its necessary minimum.

  Normally, each managed widget or layout is put into a cell of its own
  using addWidget() or addLayout(), but you can also put widget into
  multiple cells using addMultiCellWidget().  However, if you do that,
  QGridLayout does not take the managed widget's minimum size into
  consideration (because it cannot know what column the minimum
  width should belong to).  Thus you must set the minimum width of
  each column using addColSpacing().

  This illustration shows a fragment of a dialog with a five-column,
  three-row grid (the grid is shown overlaid in magenta):

  <img src="gridlayout.gif" width="425" height="150">

  Columns 0, 2 and 4 in this dialog fragment are made up of a QLabel,
  a QLineEdit and a QListBox.  Columns 1 and 2 are placeholders, made
  with setColSpacing().	 Row 0 consists of three QLabel objects, row 1
  of three QLineEdit objects and row 2 of three QListBox objects.

  Since we did not want any space between the rows, we had to use
  placeholder columns to get the right amount of space between the
  columns.

  Note that the columns and rows are not equally wide/tall: If you
  want two columns to be equally wide, you must set the columns'
  minimum widths and stretch factors to be the same yourself.  You do
  this using addColSpacing() and setStretch().

  If the QGridLayout is not the top-level layout (ie. is not managing
  all of the widget's area and children), you must add it to its
  parent layout when you have created it, but before you can do
  anything with it.  The normal way to add a layout is by calling
  parentLayout->addLayout().

  Once you have done that, you can start putting widgets and other
  layouts in the cells of your grid layout using addWidget(),
  addLayout() and addMultiCellWidget().

  Finally, if the grid is the top-level layout, you activate() it.

  QGridLayout also includes two margin widths: The border width and
  the inter-box width.	The border width is the width of the reserved
  space along each of the QGridLayout's four sides.  The intra-widget
  width is the width of the automatically allocated spacing between
  neighbouring boxes.

  The border width defaults to 0, and the intra-widget width defaults
  to the same as the border width.  Both are set using arguments to
  the constructor.
*/


/*! \base64 gridlayout.gif

  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

*/

/*!
  Constructs a new QGridLayout with \a nRows rows, \a nCols columns
   and main widget \a  parent.	\a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder is the default number of pixels
  between cells.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name.
*/

QGridLayout::QGridLayout( QWidget *parent, int nRows, int nCols, int border ,
			  int autoBorder , const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    init( nRows, nCols );
}


/*!
  Constructs a new grid with \a nRows rows and \a  nCols columns,
  If \a autoBorder is -1, this QGridLayout will inherits its parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You have to insert this grid into another layout before using it.
*/

QGridLayout::QGridLayout( int nRows, int nCols,
			  int autoBorder, const char *name )
     : QLayout( autoBorder, name )
{
    init( nRows, nCols );
}


/*!
  Deletes this grid. Geometry management is terminated if
  this is a top-level grid.
*/

QGridLayout::~QGridLayout()
{
}

/*!
  Returns the number of rows in this grid.
  */
int QGridLayout::numRows() const
{
    return array->numRows();
}

/*!
  Returns the number of columns in this grid.
  */
int QGridLayout::numCols() const
{
    return array->numCols();
}


/*!
  Returns the preferred size of this grid.
*/

QSize QGridLayout::sizeHint() const
{
    QSize s =  array->sizeHint( defaultBorder() );
    if ( isTopLevel() )
	s += QSize( 2*margin(), 2*margin() );
    return s;
}
/*!
  Returns the minimum size needed by this grid.
*/

QSize QGridLayout::minimumSize() const
{
    QSize s =  array->minimumSize( defaultBorder() );
    if ( isTopLevel() )
	s += QSize( 2*margin(), 2*margin() );
    return s;
}
/*!
  Returns the maximum size needed by this grid.
*/

QSize QGridLayout::maximumSize() const
{
    QSize s =  array->maximumSize( defaultBorder() );
    if ( isTopLevel() )
	QSize( QMIN( 2*margin()+s.width(), QCOORD_MAX ),
	       QMIN( 2*margin()+s.height(), QCOORD_MAX ) );
    return s;
}




/*!
  Returns whether this layout's preferred height depends on its width.
*/

bool QGridLayout::hasHeightForWidth() const
{
    return ((QGridLayout*)this)->array->hasHeightForWidth();
}


/*!
  Returns the layout's preferred height when it is \a w pixels wide.
*/

int QGridLayout::heightForWidth( int w ) const
{
    return ((QGridLayout*)this)->array->heightForWidth( w, defaultBorder() ) 
	+ 2*margin();
}




/*!
  Searches for \a w in this layout (not including child layouts).  If
  \a w is found, it sets \a row and \a col to the row and column and
  returns TRUE. If \a w is not found, FALSE is returned.

  \warning If a widget spans  multiple rows/columns, the top-left cell is returne
*/

bool QGridLayout::findWidget( QWidget* w, int *row, int *col )
{
    return array->findWidget( w, row, col );
}


/*!
  Removes \a w from geometry management.
 */
bool QGridLayout::removeWidget( QWidget *w )
{
    return array->removeWidget( w );
}

/*!
  Resizes managed widgets within the rectangle \a s.
 */
void QGridLayout::setGeometry( const QRect &s )
{
    QLayout::setGeometry( s );
    array->distribute( s, defaultBorder() );
}

/*!
  Expands this grid so that it will have \a nRows rows and \a nCols columns.
  Will not shrink the grid.
 */
void QGridLayout::expand( int nRows, int nCols )
{
    array->expand( nRows, nCols );
}

/*!
  Sets up the table and other internal stuff
*/

void QGridLayout::init( int nRows, int nCols )
{
    array = new QLayoutArray( nRows, nCols );
}

/*!
  Adds \a item to the end of this layout.

  \warning This function is not yet finished.
*/

void QGridLayout::addItem( QLayoutItem *item )
{
    int r =0;
    int c =0;
    //######################################################################
    add( item, r, c );
}



/*!
  Adds \a item at position \a row, \a col. The layout takes over ownership
  of \a item.
*/

void QGridLayout::add( QLayoutItem *item, int row, int col )
{
    QLayoutBox *box = new QLayoutBox( item );
    array->add( box, row, col );
}



/*!
  Adds the widget \a w to the cell grid, spanning multiple rows/columns.

  Alignment is specified by \a align

*/

void QGridLayout::addMultiCell( QLayoutItem *item, int fromRow, int toRow,
  int fromCol, int toCol, int align )
{
    QLayoutBox *b = new QLayoutBox( item );
    b->setAlignment( align );
    array->add( b, fromRow, toRow, fromCol, toCol );

}


/*!
  Adds the widget \a w to the cell grid at \a row, \a col.
  The top left position is (0,0)

  Alignment is specified by \a align which takes the same arguments as
  QLabel::setAlignment().  Note that widgets take all the space they
  can get; alignment has no effect unless you have set
  QWidget::maximumSize().

*/

void QGridLayout::addWidget( QWidget *w, int row, int col, int align )
{
    QWidgetItem *b = new QWidgetItem( w );
    b->setAlignment( align );
    add( b, row, col );
}

/*!
  Adds the widget \a w to the cell grid, spanning multiple rows/columns.

  Alignment is specified by \a align which takes the same arguments as
  QLabel::setAlignment().
*/

void QGridLayout::addMultiCellWidget( QWidget *w, int fromRow, int toRow,
				      int fromCol, int toCol, int align	 )
{
    QLayoutBox *b = new QLayoutBox( w );
    b->setAlignment( align );
    array->add( b, fromRow, toRow, fromCol, toCol );
}


/*!
  Places another layout at position (\a row, \a col) in the grid.
  The top left position is (0,0)
*/

void QGridLayout::addLayout( QLayout *layout, int row, int col)
{
    addChildLayout( layout );
    add( layout, row, col );
}


/*!
  Sets the stretch factor of row \a row to \a stretch.
  The first row is number 0.

  The stretch factor  is relative to the other rows in this grid.
  Rows with higher stretch factor take more of the available space.

  The default stretch factor is 0.
  If the stretch factor is 0 and no other row in this table can
  grow at all, the row may still grow.
*/

void QGridLayout::setRowStretch( int row, int stretch )
{
    array->setRowStretch( row, stretch );
}


/*!
  Sets the stretch factor of column \a col to \a stretch.
  The first column is number 0.

  The stretch factor  is relative to the other columns in this grid.
  Columns with higher stretch factor take more of the available space.

  The default stretch factor is 0.
  If the stretch factor is 0 and no other column in this table can
  grow at all, the column may still grow.
*/

void QGridLayout::setColStretch( int col, int stretch )
{
    array->setColStretch( col, stretch );
}


/*!
  Sets the minimum height of \a row to \a minsize pixels.
 */
void QGridLayout::addRowSpacing( int row, int minsize )
{
    QLayoutItem *b = new QSpacerItem( 0, minsize );
    //b.setAlignment( align );
    add( b, row, 0 );
}

/*!
  Sets the minimum width of \a col to \a minsize pixels.
 */
void QGridLayout::addColSpacing( int col, int minsize )
{
    QLayoutItem *b = new QSpacerItem( minsize, 0 );
    //b.setAlignment( align );
    add( b, 0, col );
}



/*!
  Returns the expansiveness of this layout
*/

QSizePolicy::ExpandData QGridLayout::expanding()
{
    return array->expanding();
}

/*!
  Sets which of the four corners of the grid corresponds to (0,0).
*/

void QGridLayout::setOrigin( Corner c )
{
    array->setReversed( c == BottomLeft || c == BottomRight,
			c == TopRight || c == BottomRight );
}
/*!
  Resets cached information.
*/

void QGridLayout::invalidate()
{
    array->setDirty();
}




/*!
  \class QBoxLayout qlayout.h

  \brief The QBoxLayout class lines up child widgets horizontally or
  vertically.

  \ingroup geomanagement

  QBoxLayout takes the space it gets (from its parent layout or from
  the mainWidget()), divides it up into a row of boxes and makes each
  managed widget fill one box.

  If the QBoxLayout is \c Horizontal, the boxes are beside each other,
  with suitable sizes.	Each widget (or other box) will get at least
  its minimum sizes and at most its maximum size, and any excess space
  is shared according to the stretch factors (more about that below).

  If the QBoxLayout is \c Vertical, the boxes are above and below each
  other, again with suitable sizes.

  The easiest way to create a QBoxLayout is to use one of the
  convenience classes QHBoxLayout (for \c Horizontal boxes) or
  QVBoxLayout (for \c Vertical boxes). You can also use the QBoxLayout
  constructor directly, specifying its direction as \c LeftToRight, \c
  Down, \c RightToLeft or \c Up.

  If the QBoxLayout is not the top-level layout (ie. is not managing
  all of the widget's area and children), you must add it to its
  parent layout before you can do anything with it.  The normal way to
  add a layout is by calling parentLayout->addLayout().

  Once you have done that, you can add boxes to the QBoxLayout using
  one of four functions: <ul>

  <li> addWidget() to add a widget to the QBoxLayout and set the
  widget's stretch factor.  (The stretch factor is along the row of
  boxes.)

  <li> addSpacing() to create an empty box; this is one of the
  functions you use to create nice and spacious dialogs.  See below
  for ways to set margins.

  <li> addStretch() to create an empty, stretchable box.

  <li> addLayout() to add a box containing another QLayout to the row
  and set that layout's stretch factor.

  </ul>

  Finally, if the layout is a top-level one, you activate() it.

  QBoxLayout also includes two margin widths: The border width and the
  inter-box width.  The border width is the width of the reserved
  space along each of the QBoxLayout's four sides.  The intra-widget
  width is the width of the automatically allocated spacing between
  neighbouring boxes.  (You can use addSpacing() to get more space.)

  The border width defaults to 0, and the intra-widget width defaults
  to the same as the border width.  Both are set using arguments to
  the constructor.

  You will almost always want to use the convenience classes for
  QBoxLayout: QVBoxLayout and QHBoxLayout, because of their simpler
  constructors.
*/

static inline bool horz( QBoxLayout::Direction dir )
{
    return dir == QBoxLayout::RightToLeft || dir == QBoxLayout::LeftToRight;
}

#if 0
class QBoxLayoutData {
public:
    QArray<QLayoutStruct> arr;
};
#endif

/*!
  Creates a new QBoxLayout with direction \a d and main widget \a
  parent.  \a parent may not be 0.

  \a border is the number of pixels between the edge of the widget and
  the managed children.	 \a autoBorder is the default number of pixels
  between neighbouring children.  If \a autoBorder is -1 the value
  of \a border is used.

  \a name is the internal object name

  \sa direction()
*/

QBoxLayout::QBoxLayout( QWidget *parent, Direction d,
			int border, int autoBorder, const char *name )
    : QGridLayout( parent, 0, 0, border, autoBorder, name )
{
    //    data = new QBoxLayoutData;
    dir = d;
    if ( d == RightToLeft || d == BottomToTop )
	setOrigin( BottomRight );
}

/*!
  If \a autoBorder is -1, this QBoxLayout will inherit its parent's
  defaultBorder(), otherwise \a autoBorder is used.

  You have to insert this box into another layout before using it.
*/

QBoxLayout::QBoxLayout( Direction d,
			int autoBorder, const char *name )
    : QGridLayout( 0, 0, autoBorder, name )
{
    //    data = new QBoxLayoutData;
    dir = d;
    if ( d == RightToLeft || d == BottomToTop )
	setOrigin( BottomRight );
}


/*!
  Destroys this box.
*/

QBoxLayout::~QBoxLayout()
{
}


/*!
  Adds \a item to this box.
*/

void QBoxLayout::addItem( QLayoutItem *item )
{
    if ( horz( dir ) ) {
	int n = numCols();
	QGridLayout::add( item, 0, n ) ;
    } else {
	int n = numRows();
	QGridLayout::add( item, n, 0 ) ;
    }

}



/*!
  Adds \a layout to the box, with serial stretch factor \a stretch.

  \sa addWidget(), addSpacing()
*/

void QBoxLayout::addLayout( QLayout *layout, int stretch )
{
    if ( horz( dir ) ) {
	int n = numCols();
	QGridLayout::addLayout( layout, 0, n ) ;
	setColStretch( n, stretch );
    } else {
	int n = numRows();
	QGridLayout::addLayout( layout, n, 0 ) ;
	setRowStretch( n, stretch );
    }
}

/*!
  Adds a non-stretchable space with size \a size.  QBoxLayout gives
  default border and spacing. This function adds additional space.

  \sa addStretch()
*/
void QBoxLayout::addSpacing( int size )
{
    //### hack in QGridLayout: spacers do not get insideSpacing

    if ( horz( dir ) ) {
	int n = numCols();
	expand( 1, n+1 );
	QLayoutItem *b = new QSpacerItem( size, 0, QSizePolicy::Fixed,
					  QSizePolicy::Minimum );
	QGridLayout::add( b, 0, n ) ;
    } else {
	int n = numRows();
	expand( n+1, 1 );
	QLayoutItem *b = new QSpacerItem( 0, size, QSizePolicy::Minimum,
					  QSizePolicy::Fixed );
	QGridLayout::add( b, n, 0 ) ;
    }
}

/*!
  Adds a stretchable space with zero minimum size
  and stretch factor \a stretch.

  \sa addSpacing()
*/
void QBoxLayout::addStretch( int stretch )
{
    //### hack in QGridLayout: spacers do not get insideSpacing

    if ( horz( dir ) ) {
	int n = numCols();
	expand( 1, n+1 );
	QLayoutItem *b = new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				       QSizePolicy::Expanding );
	QGridLayout::add( b, 0, n ) ;
	setColStretch( n, stretch );
    } else {
	int n = numRows();
	expand( n+1, 1 );
	QLayoutItem *b = new QSpacerItem( 0, 0,  QSizePolicy::Expanding,
				       QSizePolicy::Expanding );
	QGridLayout::add( b, n, 0 ) ;
	setRowStretch( n, stretch );
    }
}

/*!
  Limits the perpendicular dimension of the box (e.g. height if the
  box is LeftToRight) to a minimum of \a size. Other constraints may
  increase the limit.
*/

void QBoxLayout::addStrut( int size )
{
    warning( "QBoxLayout::addStrut( %d ), not yet tested", size );

    if ( horz( dir ) ) {
	expand( 1, 1 );
	QGridLayout::addRowSpacing( 0, size ) ;
    } else {
	expand( 1, 1 );
	QGridLayout::addColSpacing( 0, size ) ;
    }


}

/*!
  Adds \a widget to the box, with stretch factor \a stretch and
  alignment \a align.

  The stretch factor applies only in the \link direction() direction
  \endlink of the QBoxLayout, and is relative to the other boxes and
  widgets in this QBoxLayout.  Widgets and boxes with higher stretch
  factor grow more.

  If the stretch factor is 0 and nothing else in the QBoxLayout can
  grow at all, the widget may still grow up to its \link
  QWidget::setMaximumSize() maximum size. \endlink

  Alignment is perpendicular to direction(), alignment in the
  serial direction is done with addStretch().

  For horizontal boxes,	 the possible alignments are
  <ul>
  <li> \c AlignCenter centers vertically in the box.
  <li> \c AlignTop aligns to the top border of the box.
  <li> \c AlignBottom aligns to the bottom border of the box.
  </ul>

  For vertical boxes, the possible alignments are
  <ul>
  <li> \c AlignCenter centers horizontally in the box.
  <li> \c AlignLeft aligns to the left border of the box.
  <li> \c AlignRight aligns to the right border of the box.
  </ul>

  Alignment only has effect if the size of the box is greater than the
  widget's maximum size.

  \sa addLayout(), addSpacing()
*/

void QBoxLayout::addWidget( QWidget *widget, int stretch, int align )
{
    if ( horz( dir ) ) {
	int n = numCols();
	QGridLayout::addWidget( widget, 0, n, align ) ;
	setColStretch( n, stretch );
    } else {
	int n = numRows();
	QGridLayout::addWidget( widget, n, 0, align ) ;
	setRowStretch( n, stretch );
    }
}



/*!
  Sets the stretch factor for widget \a w to \a stretch and returns
  TRUE, if \a w is found in this layout (not including child layouts).

  Returns FALSE if \a w is not found.
*/

bool QBoxLayout::setStretchFactor( QWidget *w, int stretch )
{
    int r, c;
    if ( !findWidget(w, &r, &c ) )
	return FALSE;
    if ( horz( dir ) )
	setColStretch( c, stretch );
    else
	setRowStretch( r, stretch );
    return TRUE;
}


/*!
  \fn QBoxLayout::Direction QBoxLayout::direction() const

  Returns the (serial) direction of the box. addWidget(), addBox()
  and addSpacing() works in this direction; the stretch stretches
  in this direction. \link QBoxLayout::addWidget Alignment \endlink
  works perpendicular to this direction.

  The directions are \c LeftToRight, \c RightToLeft, \c TopToBottom
  and \c BottomToTop. For the last two, the shorter aliases \c Down and
  \c Up are also available.

  \sa addWidget(), addBox(), addSpacing()
*/





/*!
  \class QHBoxLayout qlayout.h

  \brief The QHBoxLayout class lines up widgets horizontally.

  \ingroup geomanagement

  This class provides an easier way to construct horizontal box layout
  objects.  See \l QBoxLayout for more details.

  The simplest way to use this class is:

  \code
     QBoxLayout * l = new QHBoxLayout( widget );
     l->addWidget( aWidget );
     l->addWidget( anotherWidget );
     l->activate()
  \endcode

  \sa QVBoxLayout QGridLayout
*/


/*!
  Creates a new top-level horizontal box.
 */
QHBoxLayout::QHBoxLayout( QWidget *parent, int border,
			  int autoBorder, const char *name )
    : QBoxLayout( parent, LeftToRight, border, autoBorder, name )
{

}

/*!
  Creates a new horizontal box. You have to add it to another
  layout before using it.
 */
QHBoxLayout::QHBoxLayout( int autoBorder, const char *name )
    :QBoxLayout( LeftToRight, autoBorder, name )
{
}


/*!
  Destroys this box.
*/

QHBoxLayout::~QHBoxLayout()
{
}



/*!
  \class QVBoxLayout qlayout.h

  \brief The QVBoxLayout class lines up widgets vertically.

  \ingroup geomanagement

  This class provides an easier way to construct vertical box layout
  objects.  See \l QBoxLayout for more details.

  The simplest way to use this class is:

  \code
     QBoxLayout * l = new QVBoxLayout( widget );
     l->addWidget( aWidget );
     l->addWidget( anotherWidget );
     l->activate()
  \endcode

  \sa QHBoxLayout QGridLayout
*/

/*!
  Creates a new top-level vertical box.
 */
QVBoxLayout::QVBoxLayout( QWidget *parent, int border,
			  int autoBorder, const char *name )
    : QBoxLayout( parent, TopToBottom, border, autoBorder, name )
{

}

/*!
  Creates a new vertical box. You have to add it to another
  layout before using it.
 */
QVBoxLayout::QVBoxLayout( int autoBorder, const char *name )
    :QBoxLayout( TopToBottom, autoBorder, name )
{
}

/*!
  Destroys this box.
*/

QVBoxLayout::~QVBoxLayout()
{
}
