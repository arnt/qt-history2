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
    DockData( QDockWidget *dw, const QRect &r ) : w( dw ), rect( r ) {}
    QDockWidget *w;
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
    if ( !dockWidgets || !dockWidgets->first() )
	return QSize( 0,0 );

    int w = 0;
    int h = 0;
    QListIterator<QDockWidget> it( *dockWidgets );
    QDockWidget *dw = 0;
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
    if ( !dockWidgets || !dockWidgets->first() )
	return QSize( 0, 0 );
    QSize s;

    QListIterator<QDockWidget> it( *dockWidgets );
    QDockWidget *dw = 0;
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

static int dock_extent( QDockWidget *w, Qt::Orientation o )
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

static int dock_strut( QDockWidget *w, Qt::Orientation o )
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

static void set_geometry( QDockWidget *w, int pos, int sectionpos, int extent, int strut, Qt::Orientation o )
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
    QDockWidget *last = 0;
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
    if ( !dockWidgets || !dockWidgets->first() )
	return 0;
    // some corrections
    QRect r = rect;
    if ( orientation() == Vertical )
	r.setHeight( r.height() - 3 );

    // init
    lines.clear();
    ls.clear();
    QListIterator<QDockWidget> it( *dockWidgets );
    QDockWidget *dw = 0;
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




QDockArea::QDockArea( Orientation o, Gravity g, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o ), layout( 0 ), grav( g )
{
    dockWidgets = new QList<QDockWidget>;
    layout = new QDockAreaLayout( this, o, dockWidgets, -1, -1, "toollayout" );
    installEventFilter( this );
}

QDockArea::~QDockArea()
{
    delete dockWidgets;
}

void QDockArea::addDockWidget( QDockWidget *w, int index )
{
    QDockWidget *dockWidget = 0;
    int dockWidgetIndex = findDockWidget( w );
    if ( dockWidgetIndex == -1 ) {
	dockWidget = w;
	dockWidget->reparent( this, QPoint( 0, 0 ), TRUE );
	w->installEventFilter( this );
	updateLayout();
	setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				    orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );
	dockWidgets->append( w );
    }
    w->dockArea = this;
    w->curPlace = QDockWidget::InDock;
    w->updateGui();

    if ( index != -1 ) {
	dockWidgets->removeRef( w );
	dockWidgets->insert( index, w );
    }
}

bool QDockArea::hasDockWidget( QDockWidget *w, int *index )
{
    int i = dockWidgets->findRef( w );
    if ( index )
	*index = i;
    return i != -1;
}

int QDockArea::lineOf( int index )
{
    QList<QDockWidget> lineStarts = layout->lineStarts();
    int i = 0;
    for ( QDockWidget *w = lineStarts.first(); w; w = lineStarts.next(), ++i ) {
	if ( dockWidgets->find( w ) >= index )
	    return i;
    }
    return i;
}

void QDockArea::moveDockWidget( QDockWidget *w, const QPoint &p, const QRect &r, bool swap )
{
    QDockWidget *dockWidget = 0;
    int dockWidgetIndex = findDockWidget( w );
    QList<QDockWidget> lineStarts = layout->lineStarts();
    if ( dockWidgetIndex != -1 ) {
	dockWidget = dockWidgets->take( dockWidgetIndex );
	if ( lineStarts.findRef( dockWidget ) != -1 && dockWidgetIndex < (int)dockWidgets->count() )
	    dockWidgets->at( dockWidgetIndex )->setNewLine( TRUE );
	layout->layoutItems( QRect( 0, 0, width(), height() ), TRUE );
    } else {
	dockWidget = w;
	dockWidget->reparent( this, QPoint( 0, 0 ), TRUE );
	if ( swap )
	    dockWidget->resize( dockWidget->height(), dockWidget->width() );
	w->installEventFilter( this );
    }

    QPoint pos = mapFromGlobal( p );
    QRect rect = QRect( mapFromGlobal( r.topLeft() ), r.size() );
    dockWidget->setOffset( point_pos( rect.topLeft(), orientation() ) );
    if ( dockWidgets->isEmpty() ) {
	dockWidgets->append( dockWidget );
    } else {
	QValueList<QRect> lines = layout->lineList();
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
#if defined(QDOCKAREA_DEBUG)
	qDebug( "insert in line %d, and insert that line: %d", dockLine, insertLine );
	qDebug( "     (btw, we have %d lines)", lines.count() );
#endif
	QDockWidget *dw = 0;
	if ( dockLine >= (int)lines.count() ) { // insert before first line
	    dockWidgets->append( dockWidget );
	    dockWidget->setNewLine( TRUE );
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "insert at the end" );
#endif
	} else if ( dockLine == 0 && insertLine ) { // insert after last line
	    dockWidgets->insert( 0, dockWidget );
	    dockWidgets->at( 1 )->setNewLine( TRUE );
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
	    int index = dockWidgets->find( lineStarts.at( searchLine ) );
	    if ( index == -1 ) {
		index = 0;
		(void)dockWidgets->at( index ); // move current to index
	    }
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "     which starts at %d", index );
#endif
	    if ( !insertLine ) { // if we only insert the docking widget in the existing line
		// find the index for the widget
		bool inc = TRUE;
		bool firstTime = TRUE;
		for ( dw = dockWidgets->current(); dw; dw = dockWidgets->next() ) {
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
		if ( index >= 0 && index < (int)dockWidgets->count() &&
		     dockWidgets->at( index )->newLine() && lineOf( index ) == dockLine ) {
#if defined(QDOCKAREA_DEBUG)
		    qDebug( "get rid of the old newline and get me one" );
#endif
		    dockWidgets->at( index )->setNewLine( FALSE );
		    dockWidget->setNewLine( TRUE );
		} else { // if we are somewhere in a line, get rid of the newline
		    dockWidget->setNewLine( FALSE );
		}
	    } else { // insert in a new line, so make sure the dock widget and the widget which will be after it have a newline
		QDockWidget* nldw = dockWidgets->at( index );
		if ( nldw )
		    nldw->setNewLine( TRUE );
		dockWidget->setNewLine( TRUE );
	    }
	    // finally insert the widget
	    dockWidgets->insert( index, dockWidget );
	}
    }

    updateLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );
}

void QDockArea::removeDockWidget( QDockWidget *w, bool makeFloating, bool swap )
{
    w->removeEventFilter( this );
    QDockWidget *dockWidget = 0;
    int i = findDockWidget( w );
    if ( i == -1 )
	return;
    dockWidget = dockWidgets->at( i );
    dockWidgets->remove( i );
    QList<QDockWidget> lineStarts = layout->lineStarts();
    if ( lineStarts.findRef( dockWidget ) != -1 && i < (int)dockWidgets->count() )
	dockWidgets->at( i )->setNewLine( TRUE );
    if ( makeFloating )
	dockWidget->reparent( topLevelWidget(), WStyle_Customize | WStyle_NoBorderEx | WType_TopLevel | WStyle_Dialog,
			      QPoint( 0, 0 ), FALSE );
    if ( swap )
	dockWidget->resize( dockWidget->height(), dockWidget->width() );
    updateLayout();
    if ( dockWidgets->isEmpty() )
	setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
}

int QDockArea::findDockWidget( QDockWidget *w )
{
    return dockWidgets->findRef( w );
}

void QDockArea::updateLayout()
{
    layout->invalidate();
    layout->activate();
}

bool QDockArea::eventFilter( QObject *o, QEvent *e )
{
    if ( o->inherits( "QDockWidget" ) ) {
	if ( e->type() == QEvent::Close ) {
	    o->removeEventFilter( this );
	    QApplication::sendEvent( o, e );
	    if ( ( (QCloseEvent*)e )->isAccepted() )
		removeDockWidget( (QDockWidget*)o, FALSE, FALSE );
	    return TRUE;
	}
    }
    return FALSE;
}

void QDockArea::invalidNextOffset( QDockWidget *dw )
{
    int i = dockWidgets->find( dw );
    if ( i == -1 || i >= (int)dockWidgets->count() - 1 )
	return;
    if ( ( dw = dockWidgets->at( ++i ) ) )
	dw->setOffset( 0 );
}

bool QDockArea::isEmpty() const
{
    return dockWidgets->isEmpty();
}


QList<QDockWidget> QDockArea::dockWidgetList() const
{
    return *dockWidgets;
}

void QDockArea::lineUp( bool keepNewLines )
{
    for ( QDockWidget *dw = dockWidgets->first(); dw; dw = dockWidgets->next() ) {
	dw->setOffset( 0 );
	if ( !keepNewLines )
	    dw->setNewLine( FALSE );
    }
    layout->activate();
}

void QDockArea::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton )
	emit rightButtonPressed( e->globalPos() );
}

QDockArea::DockWidgetData *QDockArea::dockWidgetData( QDockWidget *w )
{
    DockWidgetData *data = new DockWidgetData;
    data->index = findDockWidget( w );
    if ( data->index == -1 ) {
	delete data;
	return 0;
    }
    QList<QDockWidget> lineStarts = layout->lineStarts();
    int i = -1;
    for ( QDockWidget *dw = dockWidgets->first(); dw; dw = dockWidgets->next() ) {
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

void QDockArea::dockWidget( QDockWidget *dockWidget, DockWidgetData *data )
{
    if ( !data )
	return;

    dockWidget->reparent( this, QPoint( 0, 0 ), FALSE );
    dockWidget->installEventFilter( this );
    dockWidget->dockArea = this;
    dockWidget->updateGui();

    if ( dockWidgets->isEmpty() ) {
	dockWidgets->append( dockWidget );
    } else {
	QList<QDockWidget> lineStarts = layout->lineStarts();
	int index = 0;
	if ( (int)lineStarts.count() > data->line )
	    index = dockWidgets->find( lineStarts.at( data->line ) );
	if ( index == -1 ) {
	    index = 0;
	    (void)dockWidgets->at( index );
	}
	bool firstTime = TRUE;
	int offset = data->offset;
	for ( QDockWidget *dw = dockWidgets->current(); dw; dw = dockWidgets->next() ) {
	    if ( !firstTime && lineStarts.find( dw ) != -1 )
		break;
	    if ( offset <
		 point_pos( dw->pos(), orientation() ) + size_extent( dw->size(), orientation() ) / 2 )
		break;
	    index++;
	    firstTime = FALSE;
	}
	if ( index >= 0 && index < (int)dockWidgets->count() &&
	     dockWidgets->at( index )->newLine() && lineOf( index ) == data->line ) {
	    dockWidgets->at( index )->setNewLine( FALSE );
	    dockWidget->setNewLine( TRUE );
	} else {
	    dockWidget->setNewLine( FALSE );
	}

	dockWidgets->insert( index, dockWidget );
    }
    dockWidget->show();
    updateLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );

}

bool QDockArea::isDockWidgetAccepted( QDockWidget *dw )
{
    if ( !dw )
	return FALSE;
    if ( forbiddenWidgets.findRef( dw ) != -1 )
	return FALSE;
    if ( !parentWidget() || !parentWidget()->inherits( "QMainWindow" ) )	
	return TRUE;
    QMainWindow *mw = (QMainWindow*)parentWidget();
    if ( !mw->hasDockWidget( dw ) )
	return FALSE;
    if ( !mw->isDockEnabled( this ) )
	return FALSE;
    if ( !mw->isDockEnabled( dw, this ) )
	return FALSE;
    return TRUE;
}

void QDockArea::setAcceptDockWidget( QDockWidget *dw, bool accept )
{
    if ( accept )
	forbiddenWidgets.removeRef( dw );
    else if ( forbiddenWidgets.findRef( dw ) == -1 )
	forbiddenWidgets.append( dw );
}
