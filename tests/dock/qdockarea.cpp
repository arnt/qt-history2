#include "qdockarea.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qvector.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qwidgetlist.h>
#include <qmap.h>

extern bool toolbarHackFor30Development;

struct DockData 
{
    DockData() : w( 0 ), rect() {}
    DockData( QDockWidget *dw, const QRect &r ) : w( dw ), rect( r ) {}
    QDockWidget *w;
    QRect rect;
};

class QToolLayout : public QLayout
{
    Q_OBJECT

public:
    QToolLayout( QWidget* parent, Qt::Orientation o, QList<QDockWidget> *wl, int space = -1, int margin = -1, const char *name = 0 )
	: QLayout( parent, space, margin, name ), orient( o ), dockWidgets( wl ) { init(); }
    ~QToolLayout() {}

    void addItem( QLayoutItem * ) {}
    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    int widthForHeight( int ) const;
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const { return QSizePolicy::NoDirection; }
    void invalidate();
    Qt::Orientation orientation() const { return orient; }

protected:
    void setGeometry( const QRect& );

private:
    void init();
    int layoutItems( const QRect&, bool testonly = FALSE );
    Qt::Orientation orient;
    int cached_width, cached_height;
    int cached_hfw, cached_wfh;
    QList<QDockWidget> *dockWidgets;
    
};


QSize QToolLayout::sizeHint() const
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

bool QToolLayout::hasHeightForWidth() const
{
    return orient == Horizontal;
}

void QToolLayout::init()
{
    cached_width = 0;
    cached_height = 0;
    cached_hfw = -1;
    cached_wfh = -1;
}

QSize QToolLayout::minimumSize() const
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

void QToolLayout::invalidate()
{
    cached_width = 0;
    cached_height = 0;
}

static int start_pos( const QRect &r, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	if ( !qApp->reverseLayout() )
	    return r.x();
	else
	    return r.x() + r.width();
    } else {
	return r.y();
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

static int dock_extend( QDockWidget *w, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	int wid;
	if ( ( wid = w->fixedExtend().width() ) != -1 )
	    return wid;
	return w->sizeHint().width();
    } else {
	int hei;
	if ( ( hei = w->fixedExtend().height() ) != -1 )
	    return hei;
	return w->sizeHint().height();
    }
}

static int dock_strut( QDockWidget *w, Qt::Orientation o )
{
    if ( o != Qt::Horizontal ) {
	int wid;
	if ( ( wid = w->fixedExtend().width() ) != -1 )
	    return wid;
	return w->sizeHint().width();
    } else {
	int hei;
	if ( ( hei = w->fixedExtend().height() ) != -1 )
	    return hei;
	return w->sizeHint().height();
    }
}

static void set_geometry( QDockWidget *w, int pos, int sectionpos, int extend, int strut, Qt::Orientation o )
{
    if ( o == Qt::Horizontal )
	w->setGeometry( pos, sectionpos, extend, strut );
    else
	w->setGeometry( sectionpos, pos, strut, extend );
}

static int size_extend( const QSize &s, Qt::Orientation o )
{
    if ( o == Qt::Horizontal )
	return s.width();
    return s.height();
}

static void finish_line( const QValueList<DockData> &lastLine, Qt::Orientation o, int linestrut, int fullextend )
{
    QDockWidget *last = 0;
    QRect lastRect;
    for ( QValueList<DockData>::ConstIterator it = lastLine.begin(); it != lastLine.end(); ++it ) {
	if ( !last ) {
	    last = (*it).w;
	    lastRect = (*it).rect;
	    continue;
	}
	if ( !last->isStretchable() )
	    set_geometry( last, lastRect.x(), lastRect.y(), lastRect.width(), lastRect.height(), o );
	else
	    set_geometry( last, lastRect.x(), lastRect.y(), (*it).rect.x() - lastRect.x(), linestrut, o );

	last = (*it).w;
	lastRect = (*it).rect;
    }
    if ( !last )
	return;
    if ( !last->isStretchable() )
	set_geometry( last, lastRect.x(), lastRect.y(), lastRect.width(), lastRect.height(), o );
    else
	set_geometry( last, lastRect.x(), lastRect.y(), fullextend - lastRect.x(), linestrut, o );
}

int QToolLayout::layoutItems( const QRect &r, bool testonly )
{
    if ( !dockWidgets || !dockWidgets->first() )
	return 0;
    QListIterator<QDockWidget> it( *dockWidgets );
    QDockWidget *dw = 0;
    int start = start_pos( r, orientation() ) + 1;
    int pos = start;
    int sectionpos = 0;
    int linestrut = 0;
    QValueList<DockData> lastLine;
    while ( ( dw = it.current() ) != 0 ) {
 	++it;
	if ( lastLine.isEmpty() || 
	     space_left( r, pos, orientation() ) >= dock_extend( dw, orientation() ) ) {
	    lastLine.append( DockData( dw, QRect( QMAX( pos, dw->offset() ), sectionpos, 
						  dock_extend( dw, orientation() ), dock_strut( dw, orientation() ) ) ) );
	    linestrut = QMAX( dock_strut( dw, orientation() ), linestrut );
	    add_size( dock_extend( dw, orientation() ), pos, orientation() );
	} else {
	    finish_line( lastLine, orientation(), linestrut, size_extend( r.size(), orientation() ) );
	    lastLine.clear();
	    sectionpos += linestrut;
	    linestrut = 0;
	    pos = start;
	    --it;
	}
    }
    finish_line( lastLine, orientation(), linestrut, size_extend( r.size(), orientation() ) );
    return sectionpos + linestrut;
}

int QToolLayout::heightForWidth( int w ) const
{
    if ( cached_width != w ) {
	QToolLayout * mthis = (QToolLayout*)this;
	mthis->cached_width = w;
	int h = mthis->layoutItems( QRect( 0, 0, w, 0 ), TRUE );
	mthis->cached_hfw = h;
	return h;
    }
    return cached_hfw;
}

int QToolLayout::widthForHeight( int h ) const
{
    if ( cached_height != h ) {
	QToolLayout * mthis = (QToolLayout*)this;
	mthis->cached_height = h;
	int w = mthis->layoutItems( QRect( 0, 0, 0, h ), TRUE );
	mthis->cached_wfh = w;
	return w;
    }
    return cached_wfh;
}




QDockArea::QDockArea( Orientation o, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o ), layout( 0 )
{
    dockWidgets = new QList<QDockWidget>;
    dockWidgets->setAutoDelete( TRUE );
    setMinimumSize( 3, 3 );
    layout = new QToolLayout( this, o, dockWidgets, -1, -1, "toollayout" ); 
}

QDockArea::~QDockArea()
{
    delete dockWidgets;
}

void QDockArea::moveDockWidget( QDockWidget *w, const QPoint &, const QRect &, bool swap )
{
    QDockWidget *dockWidget = 0;
    int i = findDockWidget( w );
    if ( i != -1 ) {
	dockWidget = dockWidgets->at( i );
    } else {
	dockWidget = w;
	dockWidget->reparent( this, QPoint( 0, 0 ), TRUE );
	if ( swap )
	    dockWidget->resize( dockWidget->height(), dockWidget->width() );
	dockWidgets->append( dockWidget );
	w->installEventFilter( this );
    }
    updateLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Fixed,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Fixed ) );
}

void QDockArea::removeDockWidget( QDockWidget *w, bool makeFloating, bool swap )
{
    w->removeEventFilter( this );
    QDockWidget *dockWidget = 0;
    int i = findDockWidget( w );
    if ( i == -1 )
	return;
    dockWidget = dockWidgets->at( i );
    if ( makeFloating )
	dockWidget->reparent( 0, WStyle_Customize | WStyle_NoBorderEx, QPoint( 0, 0 ), FALSE );
    if ( swap )
	dockWidget->resize( dockWidget->height(), dockWidget->width() );
    dockWidgets->remove( i );
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

#include "qdockarea.moc"
