/****************************************************************************
** $Id: $
**
**  Splitter widget
**
**  Created:  980105
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "qsplitter.h"
#ifndef QT_NO_SPLITTER

#include "../kernel/qlayoutengine_p.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdrawutil.h"
#include "qmemarray.h"
#include "qobjectlist.h"
#include "qpainter.h"
#include "qptrlist.h"
#include "qstyle.h"

class QSplitterHandle : public QWidget
{
    Q_OBJECT
public:
    QSplitterHandle( Qt::Orientation o,
		       QSplitter *parent, const char* name=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    bool opaque() const { return s->opaqueResize(); }

    QSize sizeHint() const;

    int id() const { return myId; } // data->list.at(id())->wid == this
    void setId( int i ) { myId = i; }

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    Qt::Orientation orient;
    bool opaq;
    int myId;

    QSplitter *s;
};

#include "qsplitter.moc"

static int mouseOffset;
static int opaqueOldPos = -1; // this assumes that there's only one mouse

static QPoint toggle( QWidget *w, QPoint pos )
{
    return -pos - QPoint( w->minimumWidth(), w->minimumHeight() );
}

static bool inFirstQuadrant( QWidget *w )
{
    return w->x() >= 0 && w->y() >= 0;
}

static QPoint topLeft( QWidget *w )
{
    if ( inFirstQuadrant(w) ) {
	return w->pos();
    } else {
	return toggle( w, w->pos() );
    }
}

static QPoint bottomRight( QWidget *w )
{
    if ( inFirstQuadrant(w) ) {
	return w->geometry().bottomRight();
    } else {
	return toggle( w, w->pos() ) - QPoint( 1, 1 );
    }
}

QSplitterHandle::QSplitterHandle( Qt::Orientation o,
				  QSplitter *parent, const char * name )
    : QWidget( parent, name )
{
    s = parent;
    setOrientation( o );
}

QSize QSplitterHandle::sizeHint() const
{
    int sw = parentWidget()->style().pixelMetric( QStyle::PM_SplitterWidth,
						  this );
    return parentWidget()->style().sizeFromContents( QStyle::CT_Splitter, s,
						     QSize(sw, sw) )
				  .expandedTo( QApplication::globalStrut() );
}

void QSplitterHandle::setOrientation( Qt::Orientation o )
{
    orient = o;
#ifndef QT_NO_CURSOR
    setCursor( o == QSplitter::Horizontal ? splitHCursor : splitVCursor );
#endif
}

void QSplitterHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !(e->state()&LeftButton) )
	return;
    QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
		 - mouseOffset;
    if ( opaque() ) {
	s->moveSplitter( pos, id() );
    } else {
	s->setRubberband( s->adjustPos(pos, id()) );
    }
}

void QSplitterHandle::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton )
	mouseOffset = s->pick( e->pos() );
}

void QSplitterHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !opaque() && e->button() == LeftButton ) {
	QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
		     - mouseOffset;
	s->setRubberband( -1 );
	s->moveSplitter( pos, id() );
    }
}

void QSplitterHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    parentWidget()->style().drawPrimitive( QStyle::PE_Splitter, &p, rect(),
					   colorGroup(),
					   (orientation() == Qt::Horizontal ?
					    QStyle::Style_Horizontal : 0) );
}

class QSplitterLayoutStruct
{
public:
    QSplitter::ResizeMode mode;
    QCOORD sizer;
    bool isSplitter;
    QWidget *wid;

    QSplitterLayoutStruct() : sizer( -1 ) { }
    QCOORD getSizer( Qt::Orientation orient );
};

QCOORD QSplitterLayoutStruct::getSizer( Qt::Orientation orient )
{
    if ( sizer == -1 ) {
	QSize s = wid->sizeHint();
	if ( !s.isValid() || wid->testWState(Qt::WState_Resized) )
	    s = wid->size();
	sizer = ( orient == Qt::Horizontal ) ? s.width() : s.height();
    }
    return sizer;
}

class QSplitterData
{
public:
    QSplitterData() : opaque( FALSE ), firstShow( TRUE ) {}

    QPtrList<QSplitterLayoutStruct> list;
    bool opaque;
    bool firstShow;
};


/*!
    \class QSplitter qsplitter.h
    \brief The QSplitter class implements a splitter widget.

    \ingroup organizers
    \mainclass

    A splitter lets the user control the size of child widgets by
    dragging the boundary between the children. Any number of widgets
    may be controlled.

    To show a QListBox, a QListView and a QTextEdit side by side:
    \code
	QSplitter *split = new QSplitter( parent );
	QListBox *lb = new QListBox( split );
	QListView *lv = new QListView( split );
	QTextEdit *ed = new QTextEdit( split );
    \endcode

    In QSplitter the boundary can be either horizontal or vertical.
    The default is horizontal (the children are side by side) but you
    can use setOrientation( QSplitter::Vertical ) to set it to
    vertical.

    By default, all widgets can be as large or as small as the user
    wishes, down to \link QWidget::minimumSizeHint()
    minimumSizeHint()\endlink. You can also use setMinimumSize() and
    setMaximumSize() on the children. Use setResizeMode() to specify
    that a widget should keep its size when the splitter is resized.

    Although QSplitter normally resizes the children only at the end
    of a resize operation, if you call setOpaqueResize( TRUE ) the
    widgets are resized as often as possible.

    The initial distribution of size between the widgets is determined
    by the initial size of each widget. You can also use setSizes() to
    set the sizes of all the widgets. The function sizes() returns the
    sizes set by the user.

    If you hide() a child its space will be distributed among the
    other children. It will be reinstated when you show() it again. It
    is also possible to reorder the widgets within the splitter using
    moveToFirst() and moveToLast().

    <img src=qsplitter-m.png> <img src=qsplitter-w.png>

    \sa QTabBar
*/


/*!
    Constructs a horizontal splitter with the \a parent and \a name
    arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter( QWidget *parent, const char *name )
    : QFrame( parent, name, WPaintUnclipped )
{
    orient = Horizontal;
    init();
}


/*!
    Constructs a splitter with orientation \a o with the \a parent and
    \a name arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter( Orientation o, QWidget *parent, const char *name )
    : QFrame( parent, name, WPaintUnclipped )
{
    orient = o;
    init();
}


/*!
    Destroys the splitter and any children.
*/

QSplitter::~QSplitter()
{
    data->list.setAutoDelete( TRUE );
    delete data;
}


void QSplitter::init()
{
    data = new QSplitterData;
    if ( orient == Horizontal )
	setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred) );
    else
	setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding) );
}


/*!
    \fn void QSplitter::refresh()

    Updates the splitter's state. You should not need to call this
    function.
*/


/*!
    \property QSplitter::orientation
    \brief the orientation of the splitter

    By default the orientation is horizontal (the widgets are side by
    side). The possible orientations are \c Qt:Vertical and
    \c Qt::Horizontal (the default).
*/

void QSplitter::setOrientation( Orientation o )
{
    if ( orient == o )
	return;
    orient = o;

    if ( orient == Horizontal )
	setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred) );
    else
	setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding) );

    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->isSplitter )
	    ((QSplitterHandle*)s->wid)->setOrientation( o );
	s = data->list.next();
    }
    recalc( isVisible() );
}


/*!
    \reimp
*/
void QSplitter::resizeEvent( QResizeEvent * )
{
    doResize();
}


/*
  Inserts the widget \a w at the end (or at the beginning if \a first
  is TRUE) of the splitter's list of widgets.

  It is the responsibility of the caller of this function to make sure
  that \a w is not already in the splitter and to call recalcId if
  needed. (If \a first is TRUE, then recalcId is very probably
  needed.)
*/

QSplitterLayoutStruct *QSplitter::addWidget( QWidget *w, bool first )
{
    QSplitterLayoutStruct *s;
    QSplitterHandle *newHandle = 0;
    if ( data->list.count() > 0 ) {
	s = new QSplitterLayoutStruct;
	s->mode = KeepSize;
	QString tmp = "qt_splithandle_";
	tmp += w->name();
	newHandle = new QSplitterHandle( orientation(), this, tmp );
	s->wid = newHandle;
	newHandle->setId( data->list.count() );
	s->isSplitter = TRUE;
	s->sizer = pick( newHandle->sizeHint() );
	if ( first )
	    data->list.prepend( s );
	else
	    data->list.append( s );
    }
    s = new QSplitterLayoutStruct;
    s->mode = Auto;
    s->wid = w;
    s->isSplitter = FALSE;
    if ( first )
	data->list.prepend( s );
    else
	data->list.append( s );
    if ( newHandle && isVisible() )
	newHandle->show(); // will trigger sending of post events
    return s;
}


/*!
    Tells the splitter that a child widget has been inserted or
    removed. The event is passed in \a c.
*/

void QSplitter::childEvent( QChildEvent *c )
{
    if ( c->type() == QEvent::ChildInserted ) {
	if ( !c->child()->isWidgetType() )
	    return;

	if ( ((QWidget*)c->child())->testWFlags( WType_TopLevel ) )
	    return;

	QSplitterLayoutStruct *s = data->list.first();
	while ( s ) {
	    if ( s->wid == c->child() )
		return;
	    s = data->list.next();
	}
	addWidget( (QWidget*)c->child() );
	recalc( isVisible() );
    } else if ( c->type() == QEvent::ChildRemoved ) {
	QSplitterLayoutStruct *p = 0;
	if ( data->list.count() > 1 )
	    p = data->list.at( 1 ); // remove handle after first widget
	QSplitterLayoutStruct *s = data->list.first();
	while ( s ) {
	    if ( s->wid == c->child() ) {
		data->list.removeRef( s );
		delete s;
		if ( p && p->isSplitter ) {
		    data->list.removeRef( p );
		    delete p->wid; // will call childEvent()
		    delete p;
		}
		recalcId();
		doResize();
		return;
	    }
	    p = s;
	    s = data->list.next();
	}
    }
}


/*!
    Shows a rubber band at position \a p. If \a p is negative, the
    rubber band is removed.
*/

void QSplitter::setRubberband( int p )
{
    QPainter paint( this );
    paint.setPen( gray );
    paint.setBrush( gray );
    paint.setRasterOp( XorROP );
    QRect r = contentsRect();
    const int rBord = 3; // customizable?
    int sw = style().pixelMetric( QStyle::PM_SplitterWidth, this );
    if ( orient == Horizontal ) {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( opaqueOldPos + sw/2 - rBord , r.y(),
			    2*rBord, r.height() );
	if ( p >= 0 )
	    paint.drawRect( p + sw/2 - rBord, r.y(), 2*rBord, r.height() );
    } else {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( r.x(), opaqueOldPos + sw/2 - rBord,
			    r.width(), 2*rBord );
	if ( p >= 0 )
	    paint.drawRect( r.x(), p + sw/2 - rBord, r.width(), 2*rBord );
    }
    opaqueOldPos = p;
}


/*!
    \reimp
*/

bool QSplitter::event( QEvent *e )
{
    switch ( e->type() ) {
    case QEvent::Show:
	if ( !data->firstShow )
	    break;
	data->firstShow = FALSE;
	// fall through
    case QEvent::LayoutHint:
	recalc( isVisible() );
	break;
    default:
	;
    }
    return QWidget::event( e );
}


/*!
  \obsolete

  Draws the splitter handle in the rectangle described by \a x, \a y,
  \a w, \a h using painter \a p.
  \sa QStyle::drawPrimitive()
*/

// ### Remove this in 4.0

void QSplitter::drawSplitter( QPainter *p,
			      QCOORD x, QCOORD y, QCOORD w, QCOORD h )
{
    style().drawPrimitive(QStyle::PE_Splitter, p, QRect(x, y, w, h), colorGroup(),
			  (orientation() == Qt::Horizontal ?
			   QStyle::Style_Horizontal : 0));
}


/*!
    Returns the ID of the splitter to the right of or below the widget
    \a w, or 0 if there is no such splitter (i.e. it is either not in
    this QSplitter or it is at the end).
*/

int QSplitter::idAfter( QWidget* w ) const
{
    QSplitterLayoutStruct *s = data->list.first();
    bool seen_w = FALSE;
    while ( s ) {
	if ( s->isSplitter && seen_w )
	    return data->list.at();
	if ( !s->isSplitter && s->wid == w )
	    seen_w = TRUE;
	s = data->list.next();
    }
    return 0;
}


/*!
    Moves the left/top edge of the splitter handle with ID \a id as
    close as possible to position \a p, which is the distance from the
    left (or top) edge of the widget.

    For Arabic and Hebrew the layout is reversed, and using this
    function to set the position of the splitter might lead to
    unexpected results, since in Arabic and Hebrew the position of
    splitter one is to the left of the position of splitter zero.

    \sa idAfter()
*/
void QSplitter::moveSplitter( QCOORD p, int id )
{
    QSplitterLayoutStruct *s = data->list.at( id );
    int farMin;
    int min;
    int max;
    int farMax;

    p = adjustPos( p, id, &farMin, &min, &max, &farMax );
    int oldP = pick( s->wid->pos() );

    if ( QApplication::reverseLayout() && orient == Horizontal ) {
	int q = p + s->wid->width();
	doMove( FALSE, q, id - 1, -1, (q > oldP), (p > max) );
	doMove( TRUE, q, id, -1, (q > oldP), (p < min) );
    } else {
	doMove( FALSE, p, id, +1, (p < oldP), (p > max) );
	doMove( TRUE, p, id - 1, +1, (p < oldP), (p < min) );
    }
    storeSizes();
}


void QSplitter::setGeo( QWidget *w, int p, int s, bool splitterMoved )
{
    QRect r;
    if ( orient == Horizontal ) {
	if ( QApplication::reverseLayout() && orient == Horizontal
	     && !splitterMoved )
	    p = contentsRect().width() - p - s;
	r.setRect( p, contentsRect().y(), s, contentsRect().height() );
    } else {
	r.setRect( contentsRect().x(), p, contentsRect().width(), s );
    }

    /*
      Hide the child widget, but without calling hide() so that the
      splitter handle is still shown.
    */
    if ( s <= 0 && pick(w->minimumSize()) > 0 )
	r.moveTopLeft( toggle(w, r.topLeft()) );
    w->setGeometry( r );
}


/*
  Places the right/bottom edge of the widget \a id at position \a pos - 1.

  \sa idAfter()
*/

void QSplitter::doMove( bool backwards, int pos, int id, int delta, bool upLeft,
			bool maySquash )
{
    if ( id < 0 || id >= (int) data->list.count() )
	return;

    QSplitterLayoutStruct *s = data->list.at( id );
    QWidget *w = s->wid;

    int nextId = backwards ? id - delta : id + delta;

    if ( w->isHidden() ) {
	doMove( backwards, pos, nextId, delta, upLeft, TRUE );
    } else {
	if ( s->isSplitter ) {
	    int dd = s->getSizer( orient );
	    int nextPos = backwards ? pos - dd : pos + dd;
	    int left = backwards ? pos - dd : pos;

	    if ( upLeft ) {
		setGeo( w, left, dd, TRUE );
		doMove( backwards, nextPos, nextId, delta, upLeft, maySquash );
	    } else {
		doMove( backwards, nextPos, nextId, delta, upLeft, maySquash );
		setGeo( w, left, dd, TRUE );
	    }
	} else {
	    int dd = backwards ? pos - pick( topLeft(w) )
			       : pick( bottomRight(w) ) - pos + 1;
	    if ( dd > 0 || (inFirstQuadrant(w) && !maySquash) ) {
		dd = QMAX( pick(w->minimumSize()),
			   QMIN(dd, pick(w->maximumSize())) );
	    } else {
		dd = 0;
	    }
	    setGeo( w, backwards ? pos - dd : pos, dd, TRUE );
	    doMove( backwards, backwards ? pos - dd : pos + dd, nextId, delta,
		    upLeft, TRUE );
	}
    }
}


void QSplitter::getRange( int id, int *farMin, int *min, int *max, int *farMax )
{
    int n = data->list.count();
    if ( id <= 0 || id >= n - 1 )
	return;

    int minBefore = 0;
    int minAfter = 0;
    int maxBefore = 0;
    int maxAfter = 0;
    int i;

    for ( i = 0; i < id; i++ )
	addContribution( i, &minBefore, &maxBefore, i != id - 1 );
    for ( i = id; i < n; i++ )
	addContribution( i, &minAfter, &maxAfter, i != id + 1 );

    QRect r = contentsRect();
    int minVal;
    int maxVal;

    if ( orient == Horizontal && QApplication::reverseLayout() ) {
	int splitterWidth = style().pixelMetric( QStyle::PM_SplitterWidth,
						 this );
	minVal = r.width() - QMIN( maxBefore, pick(r.size()) - minAfter )
		 - splitterWidth;
	maxVal = r.width() - QMAX( minBefore, pick(r.size()) - maxAfter )
		 - splitterWidth;
    } else {
	minVal = pick( r.topLeft() )
		 + QMAX( minBefore, pick(r.size()) - maxAfter );
	maxVal = pick( r.topLeft() )
		 + QMIN( maxBefore, pick(r.size()) - minAfter );
    }

    if ( farMin )
	*farMin = minVal - pick( data->list.at(id - 1)->wid->minimumSize() );
    if ( min )
	*min = minVal;
    if ( max )
	*max = maxVal;
    if ( farMax )
	*farMax = maxVal + pick( data->list.at( id + 1 )->wid->minimumSize() );
}


/*!
    Returns the valid range of the splitter with ID \a id in \a *min
    and \a *max if \a min and \a max are not 0.

    \sa idAfter()
*/


void QSplitter::getRange( int id, int *min, int *max )
{
    getRange( id, min, 0, 0, max );
}


/*!
    Returns the closest legal position to \a pos of the splitter with
    ID \a id.

    \sa idAfter()
*/

int QSplitter::adjustPos( int pos, int id )
{
    int a, b, c, d;
    return adjustPos( pos, id, &a, &b, &c, &d );
}

int QSplitter::adjustPos( int pos, int id, int *farMin, int *min, int *max,
			  int *farMax )
{
    getRange( id, farMin, min, max, farMax );
    if ( pos >= *min ) {
	if ( pos <= *max ) {
	    return pos;
	} else {
	    if ( pos - *max > *farMax - pos ) {
		return *farMax;
	    } else {
		return *max;
	    }
	}
    } else {
	if ( *min - pos > pos - *farMin ) {
	    return *farMin;
	} else {
	    return *min;
	}
    }
}

void QSplitter::doResize()
{
    QRect r = contentsRect();
    int n = data->list.count();
    QMemArray<QLayoutStruct> a( n );

    for ( int pass = 0; pass < 2; pass++ ) {
	int numAutoWithStretch = 0;
	int numAutoWithoutStretch = 0;

	for ( int i = 0; i < n; i++ ) {
	    a[i].init();
	    QSplitterLayoutStruct *s = data->list.at( i );
	    if ( s->wid->isHidden() || !inFirstQuadrant(s->wid) ) {
		a[i].maximumSize = 0;
	    } else if ( s->isSplitter ) {
		a[i].sizeHint = a[i].minimumSize = a[i].maximumSize = s->sizer;
		a[i].empty = FALSE;
	    } else {
		ResizeMode mode = s->mode;
		int stretch = 1;

		if ( mode == Auto ) {
		    QSizePolicy p = s->wid->sizePolicy();
		    int sizePolicyStretch =
			    pick( QSize(p.horStretch(), p.verStretch()) );
		    if ( sizePolicyStretch > 0 ) {
			mode = Stretch;
			stretch = sizePolicyStretch;
			numAutoWithStretch++;
		    } else {
			/*
			  Do things differently on the second pass,
			  if there's one. A second pass is necessary
			  if it was found out during the first pass
			  that all Auto items are KeepSize items. In
			  that case, we make them all Stretch items
			  instead, for a more Qt 3.0-compatible
			  behavior.
			*/
			mode = ( pass == 0 ) ? KeepSize : Stretch;
			numAutoWithoutStretch++;
		    }
		}

		a[i].minimumSize = pick( s->wid->minimumSize() );
		a[i].maximumSize = pick( s->wid->maximumSize() );
		a[i].empty = FALSE;

		if ( mode == Stretch ) {
		    if ( s->getSizer(orient) > 1 )
			stretch *= s->getSizer( orient );
		    // QMIN(): ad hoc work-around for layout engine limitation
		    a[i].stretch = QMIN( stretch, 8192 );
		    a[i].sizeHint = a[i].minimumSize;
		} else if ( mode == KeepSize ) {
		    a[i].sizeHint = s->getSizer( orient );
		} else { // mode == FollowSizeHint
		    a[i].sizeHint = pick( s->wid->sizeHint() );
		}
	    }
	}

	// a second pass would yield the same results
	if ( numAutoWithStretch > 0 || numAutoWithoutStretch == 0 )
	    break;
    }

    qGeomCalc( a, 0, n, pick( r.topLeft() ), pick( r.size() ), 0 );

    for ( int i = 0; i < n; i++ ) {
	QSplitterLayoutStruct *s = data->list.at(i);
	setGeo( s->wid, a[i].pos, a[i].size, FALSE );
    }
}


void QSplitter::recalc( bool update )
{
    int fi = 2 * frameWidth();
    int maxl = fi;
    int minl = fi;
    int maxt = QWIDGETSIZE_MAX;
    int mint = fi;
    int n = data->list.count();
    bool first = TRUE;

    /*
      Splitter handles before the first visible widget or right
      before a hidden widget must be hidden.
    */
    for ( int i = 0; i < n; i++ ) {
	QSplitterLayoutStruct *s = data->list.at( i );
	if ( !s->isSplitter ) {
	    QSplitterLayoutStruct *p = ( i > 0 ) ? p = data->list.at( i - 1 )
					: 0;
	    if ( p && p->isSplitter ) {
		// may trigger new recalc
		if ( first || s->wid->isHidden() ) {
		    p->wid->hide();
		} else {
		    p->wid->show();
		}
	    }
	    if ( !s->wid->isHidden() )
		first = FALSE;
	}
    }

    bool empty = TRUE;
    for ( int j = 0; j < n; j++ ) {
	QSplitterLayoutStruct *s = data->list.at( j );
	if ( !s->wid->isHidden() ) {
	    empty = FALSE;
	    if ( s->isSplitter ) {
		minl += s->getSizer( orient );
		maxl += s->getSizer( orient );
	    } else {
		QSize minS = s->wid->minimumSize();
		minl += pick( minS );
		maxl += pick( s->wid->maximumSize() );
		mint = QMAX( mint, trans(minS) );
		int tm = trans( s->wid->maximumSize() );
		if ( tm > 0 )
		    maxt = QMIN( maxt, tm );
	    }
	}
    }
    if ( empty ) {
	if ( parentWidget() != 0 && parentWidget()->inherits("QSplitter") ) {
	    // nested splitters; be nice
	    maxl = maxt = 0;
	} else {
	    // QSplitter with no children yet
	    maxl = QWIDGETSIZE_MAX;
	}
    } else {
	maxl = QMIN( maxl, QWIDGETSIZE_MAX );
    }
    if ( maxt < mint )
	maxt = mint;

    if ( orient == Horizontal ) {
	setMaximumSize( maxl, maxt );
	setMinimumSize( minl, mint );
    } else {
	setMaximumSize( maxt, maxl );
	setMinimumSize( mint, minl );
    }
    if ( update )
	doResize();
}

/*!
    \enum QSplitter::ResizeMode

    This enum type describes how QSplitter will resize each of its
    child widgets.

    \value Auto  The widget will be resized according to the stretch
    factors set in its sizePolicy().

    \value Stretch  The widget will be resized when the splitter
    itself is resized.

    \value KeepSize  QSplitter will try to keep this widget's size
    unchanged.

    \value FollowSizeHint  QSplitter will resize the widget when the
    widget's size hint changes.
*/

/*!
    Sets resize mode of widget \a w to \a mode. (The default is \c
    Auto.)
*/

void QSplitter::setResizeMode( QWidget *w, ResizeMode mode )
{
    processChildEvents();
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->wid == w ) {
	    s->mode = mode;
	    return;
	}
	s = data->list.next();
    }
    s = addWidget( w, TRUE );
    s->mode = mode;
}


/*!
    Returns TRUE if opaque resize is on; otherwise returns FALSE.

    \sa setOpaqueResize()
*/

bool QSplitter::opaqueResize() const
{
    return data->opaque;
}


/*!
    If \a on is TRUE then opaque resizing is turned on; otherwise
    opaque resizing is turned off. Opaque resizing is initially turned
    off.

    \sa opaqueResize()
*/

void QSplitter::setOpaqueResize( bool on )
{
    data->opaque = on;
}


/*!
    Moves widget \a w to the leftmost/top position.
*/

void QSplitter::moveToFirst( QWidget *w )
{
    processChildEvents();
    bool found = FALSE;
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->wid == w ) {
	    found = TRUE;
	    QSplitterLayoutStruct *p = data->list.prev();
	    if ( p ) { // not already at first place
		data->list.take(); // take p
		data->list.take(); // take s
		data->list.prepend( p );
		data->list.prepend( s );
	    }
	    break;
	}
	s = data->list.next();
    }
    if ( !found )
	addWidget( w, TRUE );
    recalcId();
}


/*!
    Moves widget \a w to the rightmost/bottom position.
*/

void QSplitter::moveToLast( QWidget *w )
{
    processChildEvents();
    bool found = FALSE;
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->wid == w ) {
	    found = TRUE;
	    data->list.take(); // take s
	    QSplitterLayoutStruct *p = data->list.current();
	    if ( p ) { // the splitter handle after s
		data->list.take(); // take p
		data->list.append( p );
	    }
	    data->list.append( s );
	    break;
	}
	s = data->list.next();
    }
    if ( !found )
	addWidget( w);
    recalcId();
}


void QSplitter::recalcId()
{
    int n = data->list.count();
    for ( int i = 0; i < n; i++ ) {
	QSplitterLayoutStruct *s = data->list.at( i );
	if ( s->isSplitter )
	    ((QSplitterHandle*)s->wid)->setId( i );
    }
}


/*!
    \reimp
*/
QSize QSplitter::sizeHint() const
{
    constPolish();
    int l = 0;
    int t = 0;
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o = it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() && !((QWidget*)o)->isHidden() ) {
		QSize s = ((QWidget*)o)->sizeHint();
		if ( s.isValid() ) {
		    l += pick( s );
		    t = QMAX( t, trans( s ) );
		}
	    }
	}
    }
    return orientation() == Horizontal ? QSize( l, t ) : QSize( t, l );
}


/*!
    \reimp
*/

QSize QSplitter::minimumSizeHint() const
{
    constPolish();
    int l = 0;
    int t = 0;
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while ( (o = it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() && !((QWidget*)o)->isHidden() ) {
		QSize s = qSmartMinSize( (QWidget*)o );
		if ( s.isValid() ) {
		    l += pick( s );
		    t = QMAX( t, trans( s ) );
		}
	    }
	}
    }
    return orientation() == Horizontal ? QSize( l, t ) : QSize( t, l );
}


void QSplitter::storeSizes()
{
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( !s->isSplitter )
	    s->sizer = pick( s->wid->size() );
	s = data->list.next();
    }
}


void QSplitter::addContribution( int id, int *min, int *max,
				 bool onlyFirstQuadrant )
{
    QSplitterLayoutStruct *s = data->list.at( id );
    if ( !s->wid->isHidden() ) {
	if ( s->isSplitter ) {
	    *min += s->getSizer( orient );
	    *max += s->getSizer( orient );
	} else {
	    if ( !onlyFirstQuadrant || inFirstQuadrant(s->wid) )
		*min += pick( s->wid->minimumSize() );
	    *max += pick( s->wid->maximumSize() );
	}
    }
}


/*!
    Returns a list of the size parameters of all the widgets in this
    splitter.

    Giving the values to another splitter's setSizes() function will
    produce a splitter with the same layout as this one.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QValueList<int> list = mySplitter.sizes();
    QValueList<int>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa setSizes()
*/

QValueList<int> QSplitter::sizes() const
{
    if ( !testWState(WState_Polished) ) {
	QWidget* that = (QWidget*) this;
	that->polish();
    }
    QValueList<int> list;
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( !s->isSplitter )
	    list.append( s->getSizer(orient) );
	s = data->list.next();
    }
    return list;
}



/*!
    Sets the size parameters to the values given in \a list. If the
    splitter is horizontal, the values set the sizes from left to
    right. If it is vertical, the sizes are applied from top to
    bottom. Extra values in \a list are ignored.

    If \a list contains too few values, the result is undefined but
    the program will still be well-behaved.

    \sa sizes()
*/

void QSplitter::setSizes( QValueList<int> list )
{
    processChildEvents();
    QValueList<int>::Iterator it = list.begin();
    QSplitterLayoutStruct *s = data->list.first();
    while ( s && it != list.end() ) {
	if ( !s->isSplitter ) {
	    s->sizer = *it;
	    ++it;
	}
	s = data->list.next();
    }
    doResize();
}


/*!
    Gets all posted child events, ensuring that the internal state of
    the splitter is consistent.
*/

void QSplitter::processChildEvents()
{
    QApplication::sendPostedEvents( this, QEvent::ChildInserted );
}


/*!
    \reimp
*/

void QSplitter::styleChange( QStyle& old )
{
    int sw = style().pixelMetric(QStyle::PM_SplitterWidth, this);
    QSplitterLayoutStruct *s = data->list.first();
    while ( s ) {
	if ( s->isSplitter )
	    s->sizer = sw;
	s = data->list.next();
    }
    doResize();
    QFrame::styleChange( old );
}

#ifndef QT_NO_TEXTSTREAM
/*!
    \relates QSplitter

    Writes the sizes and the hidden state of the widgets in the
    splitter \a splitter to the text stream \a ts.

    \sa operator>>(), sizes(), QWidget::isHidden()
*/

QTextStream& operator<<( QTextStream& ts, const QSplitter& splitter )
{
    QSplitterLayoutStruct *s = splitter.data->list.first();
    bool first = TRUE;
    ts << "[";

    while ( s != 0 ) {
	if ( !s->isSplitter ) {
	    if ( !first )
		ts << ",";

	    if ( s->wid->isHidden() ) {
		ts << "H";
	    } else if ( inFirstQuadrant(s->wid) ) {
		ts << s->getSizer( splitter.orientation() );
	    } else {
		ts << 0;
	    }
	    first = FALSE;
	}
	s = splitter.data->list.next();
    }
    ts << "]" << endl;
    return ts;
}

/*!
    \relates QSplitter

    Reads the sizes and the hidden state of the widgets in the
    splitter \a splitter from the text stream \a ts. The sizes must
    have been previously written by the operator<<() function.

    \sa operator<<(), setSizes(), QWidget::hide()
*/

QTextStream& operator>>( QTextStream& ts, QSplitter& splitter )
{
#undef SKIP_SPACES
#define SKIP_SPACES() \
    while ( line[i].isSpace() ) \
	i++

    QSplitterLayoutStruct *s = splitter.data->list.first();
    QString line = ts.readLine();
    int i = 0;

    SKIP_SPACES();
    if ( line[i] == '[' ) {
	i++;
	SKIP_SPACES();
	while ( line[i] != ']' ) {
	    while ( s != 0 && s->isSplitter )
		s = splitter.data->list.next();
	    if ( s == 0 )
		break;

	    if ( line[i].upper() == 'H' ) {
		s->wid->hide();
		i++;
	    } else {
		s->wid->show();
		int dim = 0;
		while ( line[i].digitValue() >= 0 ) {
		    dim *= 10;
		    dim += line[i].digitValue();
		    i++;
		}
		s->sizer = dim;
		if ( dim == 0 )
		    splitter.setGeo( s->wid, 0, 0, FALSE );
	    }
	    SKIP_SPACES();
	    if ( line[i] == ',' ) {
		i++;
	    } else {
		break;
	    }
	    SKIP_SPACES();
	    s = splitter.data->list.next();
	}
    }
    splitter.doResize();
    return ts;
}
#endif

#endif
