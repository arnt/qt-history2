/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#27 $
**
** Implementation of the QDockArea class
**
** Created : 001010
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
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

#include "qdockarea.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qvector.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qwidgetlist.h>
#include <qmap.h>
#include <qmainwindow.h>

//#define QDOCKAREA_DEBUG

struct DockData
{
    DockData() : w( 0 ), rect() {}
    DockData( QDockWindow *dw, const QRect &r ) : w( dw ), rect( r ) {}
    QDockWindow *w;
    QRect rect;
};


void QDockAreaLayout::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    layoutItems( r );
}

QLayoutIterator QDockAreaLayout::iterator()
{
    return 0;
}

QSize QDockAreaLayout::sizeHint() const
{
    if ( !dockWindows || !dockWindows->first() )
	return QSize( 0,0 );

    int w = 0;
    int h = 0;
    QListIterator<QDockWindow> it( *dockWindows );
    QDockWindow *dw = 0;
    it.toFirst();
    int y = -1;
    int x = -1;
    int ph = 0;
    int pw = 0;
    while ( ( dw = it.current() ) != 0 ) {
	int plush = 0, plusw = 0;
	++it;
	if ( hasHeightForWidth() ) {
	    if ( y != dw->y() )
		plush = ph;
	    y = dw->y();
	    ph = dw->height();
	} else {
	    if ( x != dw->x() )
		plusw = pw;
	    x = dw->x();
	    pw = dw->width();
	}
	h = QMAX( h, dw->height() + plush );
	w = QMAX( w, dw->width() + plusw );
    }

    if ( hasHeightForWidth() )
	return QSize( 0, h );
    return QSize( w, 0 );
}

bool QDockAreaLayout::hasHeightForWidth() const
{
    return orient == Horizontal;
}

void QDockAreaLayout::init()
{
    cached_width = 0;
    cached_height = 0;
    cached_hfw = -1;
    cached_wfh = -1;
}

QSize QDockAreaLayout::minimumSize() const
{
    if ( !dockWindows || !dockWindows->first() )
	return QSize( 0, 0 );
    QSize s;

    QListIterator<QDockWindow> it( *dockWindows );
    QDockWindow *dw = 0;
    while ( ( dw = it.current() ) != 0 ) {
 	++it;
 	s = s.expandedTo( dw->minimumSizeHint() )
 	    .expandedTo( dw->minimumSize());
    }

    if ( s.width() < 0 )
	s.setWidth( 0 );
    if ( s.height() < 0 )
	s.setHeight( 0 );

    return s;
}

void QDockAreaLayout::invalidate()
{
    cached_width = 0;
    cached_height = 0;
}

static int start_pos( const QRect &r, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	if ( !qApp->reverseLayout() )
	    return QMAX( 0, r.x() );
	else
	    return r.x() + r.width();
    } else {
	return QMAX( 0, r.y() );
    }
}

static void add_size( int s, int &pos, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	if ( !qApp->reverseLayout() )
	    pos += s;
	else
	    pos -= s;
    } else {
	pos += s;
    }
}

static int space_left( const QRect &r, int pos, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	if ( !qApp->reverseLayout() )
	    return ( r.x() + r.width() ) - pos;
	else
	    return pos - r.x();
    } else {
	return ( r.y() + r.height() ) - pos;
    }
}

static int dock_extent( QDockWindow *w, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	int wid;
	if ( ( wid = w->fixedExtent().width() ) != -1 )
	    return wid;
	return w->sizeHint().width();
    } else {
	int hei;
	if ( ( hei = w->fixedExtent().height() ) != -1 )
	    return hei;
	return w->sizeHint().height();
    }
}

static int dock_strut( QDockWindow *w, Qt::Orientation o )
{
    if ( o != Qt::Horizontal ) {
	int wid;
	if ( ( wid = w->fixedExtent().width() ) != -1 )
	    return wid;
	return w->sizeHint().width();
    } else {
	int hei;
	if ( ( hei = w->fixedExtent().height() ) != -1 )
	    return hei;
	return w->sizeHint().height();
    }
}

static void set_geometry( QDockWindow *w, int pos, int sectionpos, int extent, int strut, Qt::Orientation o )
{
    if ( o == Qt::Horizontal )
	w->setGeometry( pos, sectionpos, extent, strut );
    else
	w->setGeometry( sectionpos, pos, strut, extent );
}

static int size_extent( const QSize &s, Qt::Orientation o, bool swap = FALSE )
{
    return o == Qt::Horizontal ? ( swap ? s.height() : s.width() ) : ( swap ? s.width() :  s.height() );
}

static int point_pos( const QPoint &p, Qt::Orientation o, bool swap = FALSE )
{
    return o == Qt::Horizontal ? ( swap ? p.y() : p.x() ) : ( swap ? p.x() : p.y() );
}

static void place_line( QValueList<DockData> &lastLine, Qt::Orientation o, int linestrut, int fullextent, int tbstrut )
{
    QDockWindow *last = 0;
    QRect lastRect;
    for ( QValueList<DockData>::Iterator it = lastLine.begin(); it != lastLine.end(); ++it ) {
	if ( (*it).w->inherits( "QToolBar" ) && tbstrut != -1 )
	    (*it).rect.setHeight( tbstrut );
	if ( !last ) {
	    last = (*it).w;
	    lastRect = (*it).rect;
	    continue;
	}
	if ( !last->isStretchable() )
	    set_geometry( last, lastRect.x(), lastRect.y(), lastRect.width(), lastRect.height(), o );
	else
	    set_geometry( last, lastRect.x(), lastRect.y(), (*it).rect.x() - lastRect.x(),
			  last->isResizeEnabled() ? linestrut : lastRect.height(), o );
	last = (*it).w;
	lastRect = (*it).rect;
    }
    if ( !last )
	return;
    if ( !last->isStretchable() )
	set_geometry( last, lastRect.x(), lastRect.y(), lastRect.width(), lastRect.height(), o );
    else
	set_geometry( last, lastRect.x(), lastRect.y(), fullextent - lastRect.x() - ( o == Qt::Vertical ? 1 : 0 ),
		      last->isResizeEnabled() ? linestrut : lastRect.height(), o );
}

int QDockAreaLayout::layoutItems( const QRect &rect, bool testonly )
{
    if ( !dockWindows || !dockWindows->first() )
	return 0;
    // some corrections
    QRect r = rect;
    if ( orientation() == Vertical )
	r.setHeight( r.height() - 3 );

    // init
    lines.clear();
    ls.clear();
    QListIterator<QDockWindow> it( *dockWindows );
    QDockWindow *dw = 0;
    int start = start_pos( r, orientation() );
    int pos = start;
    int sectionpos = 0;
    int linestrut = 0;
    QValueList<DockData> lastLine;
    int tbstrut = -1;

    // go through all widgets in the dock
    while ( ( dw = it.current() ) != 0 ) {
 	++it;
	if ( !dw->isVisibleTo( parentWidget ) )
	    continue;
	// find position for the widget: This is the maximum of the
	// end of the previous widget and the offset of the widget. If
	// the position + the width of the widget dosn't fit into the
	// dock, try moving it a bit back, if possible.
	int op = pos;
	if ( !dw->isStretchable() )
	    pos = QMAX( pos, dw->offset() );
	if ( pos + dock_extent( dw, orientation() )> size_extent( r.size(), orientation() ) - 1 )
	    pos = QMAX( op, size_extent( r.size(), orientation() ) - 1 - dock_extent( dw, orientation() ) );
	// if the current widget doesn't fit into the line anymore and it is not the first widget of the line
	if ( !lastLine.isEmpty() &&
	     ( space_left( r, pos, orientation() ) < dock_extent( dw, orientation() ) || dw->newLine() ) ) {
	    if ( !testonly ) // place the last line, if not in test mode
		place_line( lastLine, orientation(), linestrut, size_extent( r.size(), orientation() ), tbstrut );
	    // remember the line coordinats of the last line
	    if ( orientation() == Horizontal )
		lines.append( QRect( 0, sectionpos, r.width(), linestrut ) );
	    else
		lines.append( QRect( sectionpos, 0, linestrut, r.height() ) );
	    // do some clearing for the next line
	    lastLine.clear();
	    sectionpos += linestrut;
	    linestrut = 0;
	    pos = start;
	    tbstrut = -1;
	}

	// remeber first widget of a line
	if ( lastLine.isEmpty() ) {
	    ls.append( dw );
	    // try to make the best position
	    int op = pos;
	    if ( !dw->isStretchable() )
		pos = QMAX( pos, dw->offset() );
	    if ( pos + dock_extent( dw, orientation() )> size_extent( r.size(), orientation() ) - 1 )
		pos = QMAX( op, size_extent( r.size(), orientation() ) - 1 - dock_extent( dw, orientation() ) );
	}
	// do some calculations and add the remember the rect which the docking widget requires for the placing
	lastLine.append( DockData( dw, QRect( pos, sectionpos,
					      dock_extent( dw, orientation() ), dock_strut( dw, orientation() ) ) ) );
	if ( dw->inherits( "QToolBar" ) )
	    tbstrut = QMAX( tbstrut, dock_strut( dw, orientation() ) );
	linestrut = QMAX( dock_strut( dw, orientation() ), linestrut );
	add_size( dock_extent( dw, orientation() ), pos, orientation() );
    }

    // if some stuff was not placed/stored yet, do it now
    if ( !testonly )
	place_line( lastLine, orientation(), linestrut, size_extent( r.size(), orientation() ), tbstrut );
    if ( orientation() == Horizontal )
	lines.append( QRect( 0, sectionpos, r.width(), linestrut ) );
    else
	lines.append( QRect( sectionpos, 0, linestrut, r.height() ) );
    if ( *(--lines.end()) == *(--(--lines.end())) )
	lines.remove( lines.at( lines.count() - 1 ) );

    return sectionpos + linestrut + ( orientation() == Horizontal ? 2 : 0 );
}

int QDockAreaLayout::heightForWidth( int w ) const
{
    if ( cached_width != w ) {
	QDockAreaLayout * mthis = (QDockAreaLayout*)this;
	mthis->cached_width = w;
	int h = mthis->layoutItems( QRect( 0, 0, w, 0 ), TRUE );
	mthis->cached_hfw = h;
	return h;
    }
    return cached_hfw;
}

int QDockAreaLayout::widthForHeight( int h ) const
{
    if ( cached_height != h ) {
	QDockAreaLayout * mthis = (QDockAreaLayout*)this;
	mthis->cached_height = h;
	int w = mthis->layoutItems( QRect( 0, 0, 0, h ), TRUE );
	mthis->cached_wfh = w;
	return w;
    }
    return cached_wfh;
}




/*! \class QDockArea qdockarea.h

  \brief The QDockArea class manages and layouts QDockWindows.

  \ingroup application

  A QDockArea is a container which manages a number of QDockWindows by
  layouting them. In cooperation with QDockWindows, it is responsible
  for docking and undocking of QDockWindows and moving them inside the
  dock area. A QDockArea also does wrapping of toolbars to fill the
  available space as good as possible.

  Normally you do not directly use a QDockArea, but rather use the
  four built in QDockAreas of a QMainWindow to be able to dock and
  undock QDockWindows (and QToolBars, which are QDockWindows) in a
  QMainWindow.

  In some cases it makes sense to use a QDockArea somewhere else, to
  allow docking of widgets there.
*/

/*! \fn Orientation orientation() const

  Returns the orientation of this dock area.
 */

/*! \fn Gravity gravity() const

  Returns the gravity of this dock area.
 */

/*! \fn void rightButtonPressed( const QPoint &globalPos )

  This signal is emitted if the user pressed with the right mouse
  button onto the area. This can be used for right mouse button
  menus. \a globalPos is the global position of the mouse when the
  event occured.
 */

/*!
  \enum QDockArea::Gravity

  This enum specifies wheather the dock windows in that area can be
  resized normally (to the right/bottom) or revers (left/top). The
  splitter-like handles for resizable dock windows are placed and controlled
  according to that information.

  <ul>
  <li>\c  Normal
  <li>\c Reverse
  </ul>
*/

/*! Creates a QDockArea with the orientation \a o, Gravity \a g as
  child of \a parent.
*/

QDockArea::QDockArea( Orientation o, Gravity g, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o ), layout( 0 ), grav( g )
{
    dockWindows = new QList<QDockWindow>;
    layout = new QDockAreaLayout( this, o, dockWindows, -1, -1, "toollayout" );
    installEventFilter( this );
}

/*!  Destructor.
 */

QDockArea::~QDockArea()
{
    delete dockWindows;
}

/*! Moves the QDockWindow \a w in the dock area. If \a w is not
  already docked in this are, \a w gets docked first. If \a index is
  -1 or larger then the number of docked widgets, \a w is appended at
  the end, else inserted at the position \a index.
*/

void QDockArea::moveDockWindow( QDockWindow *w, int index )
{
    QDockWindow *dockWindow = 0;
    int dockWindowIndex = findDockWindow( w );
    if ( dockWindowIndex == -1 ) {
	dockWindow = w;
	dockWindow->reparent( this, QPoint( 0, 0 ), TRUE );
	w->installEventFilter( this );
	updateLayout();
	setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				    orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );
	dockWindows->append( w );
    }
    w->dockArea = this;
    w->curPlace = QDockWindow::InDock;
    w->updateGui();

    if ( index != -1 ) {
	dockWindows->removeRef( w );
	dockWindows->insert( index, w );
    }
}

/*! Returns TRUE, of the QDockArea contains the dock window \a w. If
  this is the case, and a non-null pointer is passed as \a index, the
  value of \a index is set to the index at which \a w is placed at the
  moment.
*/

bool QDockArea::hasDockWindow( QDockWindow *w, int *index )
{
    int i = dockWindows->findRef( w );
    if ( index )
	*index = i;
    return i != -1;
}

int QDockArea::lineOf( int index )
{
    QList<QDockWindow> lineStarts = layout->lineStarts();
    int i = 0;
    for ( QDockWindow *w = lineStarts.first(); w; w = lineStarts.next(), ++i ) {
	if ( dockWindows->find( w ) >= index )
	    return i;
    }
    return i;
}

/*! Moves the QDockWindow \a w inside the dock are where \a p is the
  new position (global screen coordinates), \a r is the suggested
  rectangle of the dock window and \a swap tells if the orientation of
  the dockwidget needs to be changed.

  This function is provided because QDockWindow needs to call it. You
  never should need to call that yourself.
*/

void QDockArea::moveDockWindow( QDockWindow *w, const QPoint &p, const QRect &r, bool swap )
{
    QDockWindow *dockWindow = 0;
    int dockWindowIndex = findDockWindow( w );
    QList<QDockWindow> lineStarts = layout->lineStarts();
    QValueList<QRect> lines = layout->lineList();
    bool wasAloneInLine = FALSE;
    QPoint pos = mapFromGlobal( p );
    QRect lr = *lines.at( lineOf( dockWindowIndex ) );
    if ( dockWindowIndex != -1 ) {
	if ( lineStarts.find( w ) != -1 &&
	     ( dockWindowIndex < (int)dockWindows->count() - 1 && lineStarts.find( dockWindows->at( dockWindowIndex + 1 ) ) != -1 ||
	       dockWindowIndex == (int)dockWindows->count() - 1 ) )
	    wasAloneInLine = TRUE;
	dockWindow = dockWindows->take( dockWindowIndex );
	if ( !wasAloneInLine ) { // only do the pre-layout if the widget isn't the only one in its line
	    if ( lineStarts.findRef( dockWindow ) != -1 && dockWindowIndex < (int)dockWindows->count() )
		dockWindows->at( dockWindowIndex )->setNewLine( TRUE );
	    layout->layoutItems( QRect( 0, 0, width(), height() ), TRUE );
	}
    } else {
	dockWindow = w;
	dockWindow->reparent( this, QPoint( 0, 0 ), TRUE );
	if ( swap )
	    dockWindow->resize( dockWindow->height(), dockWindow->width() );
	w->installEventFilter( this );
    }

    lineStarts = layout->lineStarts();
    lines = layout->lineList();

    QRect rect = QRect( mapFromGlobal( r.topLeft() ), r.size() );
    dockWindow->setOffset( point_pos( rect.topLeft(), orientation() ) );
    if ( dockWindows->isEmpty() ) {
	dockWindows->append( dockWindow );
    } else {
	int dockLine = -1;
	bool insertLine = FALSE;
	int i = 0;
	QRect lineRect;
	// find the line which we touched with the mouse
	for ( QValueList<QRect>::Iterator it = lines.begin(); it != lines.end(); ++it, ++i ) {
	    if ( point_pos( pos, orientation(), TRUE ) >= point_pos( (*it).topLeft(), orientation(), TRUE ) &&
		 point_pos( pos, orientation(), TRUE ) <= point_pos( (*it).topLeft(), orientation(), TRUE ) +
		 size_extent( (*it).size(), orientation(), TRUE ) ) {
		dockLine = i;
		lineRect = *it;
		break;
	    }
	}
	if ( dockLine == -1 ) { // outside the dock...
	    insertLine = TRUE;
	    if ( point_pos( pos, orientation(), TRUE ) < 0 ) // insert as first line
		dockLine = 0;
	    else
		dockLine = lines.count(); // insert after the last line
	} else { // inside the dock (we have found a dockLine)
	    if ( point_pos( pos, orientation(), TRUE ) < point_pos( lineRect.topLeft(), orientation(), TRUE ) +
		 size_extent( lineRect.size(), orientation(), TRUE ) / 5 ) { 	// mouse was at the very beginning of the line
		insertLine = TRUE;					// insert a new line before that with the docking widget
	    } else if ( point_pos( pos, orientation(), TRUE ) > point_pos( lineRect.topLeft(), orientation(), TRUE ) +
			4 * size_extent( lineRect.size(), orientation(), TRUE ) / 5 ) {	// mouse was at the very and of the line
		insertLine = TRUE;						// insert a line after that with the docking widget
		dockLine++;
	    }
	}
	
	if ( !insertLine && wasAloneInLine && lr.contains( pos ) ) // if we are alone in a line and just moved in there, re-insert it
	    insertLine = TRUE;
	
#if defined(QDOCKAREA_DEBUG)
	qDebug( "insert in line %d, and insert that line: %d", dockLine, insertLine );
	qDebug( "     (btw, we have %d lines)", lines.count() );
#endif
	QDockWindow *dw = 0;
	if ( dockLine >= (int)lines.count() ) { // insert after last line
	    dockWindows->append( dockWindow );
	    dockWindow->setNewLine( TRUE );
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "insert at the end" );
#endif
	} else if ( dockLine == 0 && insertLine ) { // insert before first line
	    dockWindows->insert( 0, dockWindow );
	    dockWindows->at( 1 )->setNewLine( TRUE );
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "insert at the begin" );
#endif
	} else { // insert somewhere in between
	    // make sure each line start has a new line
	    for ( dw = lineStarts.first(); dw; dw = lineStarts.next() )
		dw->setNewLine( TRUE );
				
	    // find the index of the first widget in the search line	
	    int searchLine = dockLine;
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "search line start of %d", searchLine );
#endif
	    QDockWindow *lsw = lineStarts.at( searchLine );
	    int index = dockWindows->find( lsw );
	    if ( index == -1 ) { // the linestart widget hasn't been found, try to find it harder
		if ( lsw == w && dockWindowIndex <= (int)dockWindows->count())
		    index = dockWindowIndex;
		else
		    index = 0;
		if ( index < (int)dockWindows->count() )
		    (void)dockWindows->at( index ); // move current to index
	    }
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "     which starts at %d", index );
#endif
	    if ( !insertLine ) { // if we insert the docking widget in the existing line
		// find the index for the widget
		bool inc = TRUE;
		bool firstTime = TRUE;
		for ( dw = dockWindows->current(); dw; dw = dockWindows->next() ) {
		    if ( orientation() == Horizontal )
			dw->setFixedExtentWidth( -1 );
		    else
			dw->setFixedExtentHeight( -1 );
		    if ( !firstTime && lineStarts.find( dw ) != -1 ) // we are in the next line, so break
			break;
		    if ( point_pos( pos, orientation() ) <
			 point_pos( dw->pos(), orientation() ) + size_extent( dw->size(), orientation() ) / 2 ) {
			inc = FALSE;
		    }
		    if ( inc )
			index++;
		    firstTime = FALSE;
		}
#if defined(QDOCKAREA_DEBUG)
		qDebug( "insert at index: %d", index );
#endif
		// if we insert it just before a widget which has a new line, transfer the newline to the docking widget
		// but not if we didn't only mave a widget in its line which was alone in the line before
		if ( !( wasAloneInLine && lr.contains( pos ) )
		     && index >= 0 && index < (int)dockWindows->count() &&
		     dockWindows->at( index )->newLine() && lineOf( index ) == dockLine ) {
#if defined(QDOCKAREA_DEBUG)
		    qDebug( "get rid of the old newline and get me one" );
#endif
		    dockWindows->at( index )->setNewLine( FALSE );
		    dockWindow->setNewLine( TRUE );
		} else if ( wasAloneInLine && lr.contains( pos ) ) {
		    dockWindow->setNewLine( TRUE );
		} else { // if we are somewhere in a line, get rid of the newline
		    dockWindow->setNewLine( FALSE );
		}
	    } else { // insert in a new line, so make sure the dock widget and the widget which will be after it have a newline
#if defined(QDOCKAREA_DEBUG)
		qDebug( "insert a new line" );
#endif
		if ( index < (int)dockWindows->count() ) {
#if defined(QDOCKAREA_DEBUG)
		    qDebug( "give the widget at %d a newline", index );
#endif
		    QDockWindow* nldw = dockWindows->at( index );
		    if ( nldw )
			nldw->setNewLine( TRUE );
		}
#if defined(QDOCKAREA_DEBUG)
		qDebug( "give me a newline" );
#endif
		dockWindow->setNewLine( TRUE );
	    }
	    // finally insert the widget
	    dockWindows->insert( index, dockWindow );
	}
    }

    updateLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );
}

/*! Removes the dock window \a w from this dock area. If \a
  makeFloating is TRUE, \a w gets floated, and if \a swap is TRUE, the
  orientation of \a w gets swapped.

  Normally you never need to call that function yourself, as this
  function is only used by QDockWindow. Better use QDockWindow::dock()
  and QDockWindow::undock() instead.
*/

void QDockArea::removeDockWindow( QDockWindow *w, bool makeFloating, bool swap )
{
    w->removeEventFilter( this );
    QDockWindow *dockWindow = 0;
    int i = findDockWindow( w );
    if ( i == -1 )
	return;
    dockWindow = dockWindows->at( i );
    dockWindows->remove( i );
    QList<QDockWindow> lineStarts = layout->lineStarts();
    if ( lineStarts.findRef( dockWindow ) != -1 && i < (int)dockWindows->count() )
	dockWindows->at( i )->setNewLine( TRUE );
    if ( makeFloating )
	dockWindow->reparent( topLevelWidget(), WStyle_Customize | WStyle_NoBorderEx | WType_TopLevel | WStyle_Dialog,
			      QPoint( 0, 0 ), FALSE );
    if ( swap )
	dockWindow->resize( dockWindow->height(), dockWindow->width() );
    updateLayout();
    if ( dockWindows->isEmpty() )
	setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
}

int QDockArea::findDockWindow( QDockWindow *w )
{
    return dockWindows->findRef( w );
}

void QDockArea::updateLayout()
{
    layout->invalidate();
    layout->activate();
}

/*! \reimp
 */

bool QDockArea::eventFilter( QObject *o, QEvent *e )
{
    if ( o->inherits( "QDockWindow" ) ) {
	if ( e->type() == QEvent::Close ) {
	    o->removeEventFilter( this );
	    QApplication::sendEvent( o, e );
	    if ( ( (QCloseEvent*)e )->isAccepted() )
		removeDockWindow( (QDockWindow*)o, FALSE, FALSE );
	    return TRUE;
	}
    }
    return FALSE;
}

/*!  Invalidates the offset of the next dock window in this dock area.
 */

void QDockArea::invalidNextOffset( QDockWindow *dw )
{
    int i = dockWindows->find( dw );
    if ( i == -1 || i >= (int)dockWindows->count() - 1 )
	return;
    if ( ( dw = dockWindows->at( ++i ) ) )
	dw->setOffset( 0 );
}

/*! Returns the number of dock windows in this dock.
 */

int QDockArea::count() const
{
    return dockWindows->count();
}

/*! Returns whether this dock are contains dock windows or not.
 */

bool QDockArea::isEmpty() const
{
    return dockWindows->isEmpty();
}


/*! Returns the list of dock windows of this area.
 */

QList<QDockWindow> QDockArea::dockWindowList() const
{
    return *dockWindows;
}

/*! Lines up the dock windows in this area to waste no space. If \a
  keepNewLines is TRUE, only space inside lines is cleaned up, else
  also the number of lines might be changed.
*/

void QDockArea::lineUp( bool keepNewLines )
{
    for ( QDockWindow *dw = dockWindows->first(); dw; dw = dockWindows->next() ) {
	dw->setOffset( 0 );
	if ( !keepNewLines )
	    dw->setNewLine( FALSE );
    }
    layout->activate();
}

/*! \reimp
 */

void QDockArea::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton )
	emit rightButtonPressed( e->globalPos() );
}

QDockArea::DockWindowData *QDockArea::dockWindowData( QDockWindow *w )
{
    DockWindowData *data = new DockWindowData;
    data->index = findDockWindow( w );
    if ( data->index == -1 ) {
	delete data;
	return 0;
    }
    QList<QDockWindow> lineStarts = layout->lineStarts();
    int i = -1;
    for ( QDockWindow *dw = dockWindows->first(); dw; dw = dockWindows->next() ) {
 	if ( lineStarts.findRef( dw ) != -1 )
 	    ++i;
 	if ( dw == w )
 	    break;
    }
    data->line = i;
    data->offset = point_pos( QPoint( w->x(), w->y() ), orientation() );
    data->area = this;
    return data;
}

void QDockArea::dockWindow( QDockWindow *dockWindow, DockWindowData *data )
{
    if ( !data )
	return;

    dockWindow->reparent( this, QPoint( 0, 0 ), FALSE );
    dockWindow->installEventFilter( this );
    dockWindow->dockArea = this;
    dockWindow->updateGui();

    if ( dockWindows->isEmpty() ) {
	dockWindows->append( dockWindow );
    } else {
	QList<QDockWindow> lineStarts = layout->lineStarts();
	int index = 0;
	if ( (int)lineStarts.count() > data->line )
	    index = dockWindows->find( lineStarts.at( data->line ) );
	if ( index == -1 ) {
	    index = 0;
	    (void)dockWindows->at( index );
	}
	bool firstTime = TRUE;
	int offset = data->offset;
	for ( QDockWindow *dw = dockWindows->current(); dw; dw = dockWindows->next() ) {
	    if ( !firstTime && lineStarts.find( dw ) != -1 )
		break;
	    if ( offset <
		 point_pos( dw->pos(), orientation() ) + size_extent( dw->size(), orientation() ) / 2 )
		break;
	    index++;
	    firstTime = FALSE;
	}
	if ( index >= 0 && index < (int)dockWindows->count() &&
	     dockWindows->at( index )->newLine() && lineOf( index ) == data->line ) {
	    dockWindows->at( index )->setNewLine( FALSE );
	    dockWindow->setNewLine( TRUE );
	} else {
	    dockWindow->setNewLine( FALSE );
	}

	dockWindows->insert( index, dockWindow );
    }
    dockWindow->show();
    updateLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );

}

/*! Returns wheather \a w can be accepted to be docked into this dock
  area.
*/

bool QDockArea::isDockWindowAccepted( QDockWindow *dw )
{
    if ( !dw )
	return FALSE;
    if ( forbiddenWidgets.findRef( dw ) != -1 )
	return FALSE;
    if ( !parentWidget() || !parentWidget()->inherits( "QMainWindow" ) )	
	return TRUE;
    QMainWindow *mw = (QMainWindow*)parentWidget();
    if ( !mw->hasDockWindow( dw ) )
	return FALSE;
    if ( !mw->isDockEnabled( this ) )
	return FALSE;
    if ( !mw->isDockEnabled( dw, this ) )
	return FALSE;
    return TRUE;
}

/*! Specifies wheather \a dw can be accepted to be docked into this
  dock area.
*/

void QDockArea::setAcceptDockWindow( QDockWindow *dw, bool accept )
{
    if ( accept )
	forbiddenWidgets.removeRef( dw );
    else if ( forbiddenWidgets.findRef( dw ) == -1 )
	forbiddenWidgets.append( dw );
}
